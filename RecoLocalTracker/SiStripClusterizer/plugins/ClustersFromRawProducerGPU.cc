/*
 */
#include "RecoLocalTracker/SiStripClusterizer/interface/StripClusterizerAlgorithmFactory.h"
#include "RecoLocalTracker/SiStripZeroSuppression/interface/SiStripRawProcessingFactory.h"

#include "RecoLocalTracker/SiStripClusterizer/plugins/SiStripRawToClusterGPUKernel.h"
#include "RecoLocalTracker/Records/interface/SiStripClusterizerGPUConditionsRcd.h"
#include "RecoLocalTracker/SiStripZeroSuppression/interface/SiStripRawProcessingAlgorithms.h"

#include "DataFormats/SiStripCluster/interface/SiStripCluster.h"
#include "DataFormats/Common/interface/DetSetVectorNew.h"

#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"
#include "EventFilter/SiStripRawToDigi/interface/SiStripFEDBuffer.h"
#include "DataFormats/SiStripCommon/interface/SiStripConstants.h"

#include "CalibFormats/SiStripObjects/interface/SiStripDetCabling.h"

#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Utilities/interface/Likely.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "HeterogeneousCore/CUDACore/interface/ScopedContext.h"
#include "HeterogeneousCore/CUDAServices/interface/CUDAService.h"

#include "SiStripConditionsGPUWrapper.h"
#include "ChanLocsGPU.h"

//#include <sstream>
#include <memory>
#include <mutex>

namespace {
  std::unique_ptr<sistrip::FEDBuffer> fillBuffer(int fedId, const FEDRawData& rawData) {
    std::unique_ptr<sistrip::FEDBuffer> buffer;

    // Check on FEDRawData pointer
    const auto st_buffer = sistrip::preconstructCheckFEDBuffer(rawData);
    if UNLIKELY (sistrip::FEDBufferStatusCode::SUCCESS != st_buffer) {
      if (edm::isDebugEnabled()) {
        edm::LogWarning(sistrip::mlRawToCluster_)
            << "[ClustersFromRawProducer::" << __func__ << "]" << st_buffer << " for FED ID " << fedId;
      }
      return buffer;
    }
    buffer = std::make_unique<sistrip::FEDBuffer>(rawData);
    const auto st_chan = buffer->findChannels();
    if UNLIKELY (sistrip::FEDBufferStatusCode::SUCCESS != st_chan) {
      if (edm::isDebugEnabled()) {
        edm::LogWarning(sistrip::mlRawToCluster_)
            << "Exception caught when creating FEDBuffer object for FED " << fedId << ": " << st_chan;
      }
      buffer.reset();
      return buffer;
    }
    if UNLIKELY (!buffer->doChecks(false)) {
      if (edm::isDebugEnabled()) {
        edm::LogWarning(sistrip::mlRawToCluster_)
            << "Exception caught when creating FEDBuffer object for FED " << fedId << ": FED Buffer check fails";
      }
      buffer.reset();
      return buffer;
    }

    return buffer;
  }
}  // namespace

class SiStripClusterizerFromRawGPU final : public edm::stream::EDProducer<edm::ExternalWork> {
public:
  explicit SiStripClusterizerFromRawGPU(const edm::ParameterSet& conf)
      : buffers(1024), raw(1024), gpuAlgo_(conf.getParameter<edm::ParameterSet>("Clusterizer")) {
    inputToken_ = consumes<FEDRawDataCollection>(conf.getParameter<edm::InputTag>("ProductLabel"));
    outputToken_ = produces<cms::cuda::Product<SiStripClustersCUDA>>();

    conditionsToken_ = esConsumes<SiStripConditionsGPUWrapper, SiStripClusterizerGPUConditionsRcd>(
        edm::ESInputTag{"", conf.getParameter<std::string>("ConditionsLabel")});
    CPUconditionsToken_ = esConsumes<SiStripClusterizerConditions, SiStripClusterizerConditionsRcd>(
        edm::ESInputTag{"", conf.getParameter<std::string>("ConditionsLabel")});
  }

  void beginRun(const edm::Run&, const edm::EventSetup& es) override {}

  void acquire(edm::Event const& ev,
               edm::EventSetup const& es,
               edm::WaitingTaskWithArenaHolder waitingTaskHolder) override {
    const auto& conditions = es.getData(conditionsToken_);
    const auto& CPUconditions = es.getData(CPUconditionsToken_);

    // Sets the current device and creates a CUDA stream
    cms::cuda::ScopedContextAcquire ctx{ev.streamID(), std::move(waitingTaskHolder), ctxState_};

    // get raw data
    edm::Handle<FEDRawDataCollection> rawData;
    ev.getByToken(inputToken_, rawData);

    run(*rawData, CPUconditions);

    // Queues asynchronous data transfers and kernels to the CUDA stream
    // returned by cms::cuda::ScopedContextAcquire::stream()
    gpuAlgo_.makeAsync(raw, buffers, conditions, ctx.stream());

    // Destructor of ctx queues a callback to the CUDA stream notifying
    // waitingTaskHolder when the queued asynchronous work has finished
  }

  void produce(edm::Event& ev, const edm::EventSetup& es) override {
    cms::cuda::ScopedContextProduce ctx{ctxState_};

    // Now getResult() returns data in GPU memory that is passed to the
    // constructor of OutputData. cms::cuda::ScopedContextProduce::emplace() wraps the
    // OutputData to cms::cuda::Product<OutputData>. cms::cuda::Product<T> stores also
    // the current device and the CUDA stream since those will be needed
    // in the consumer side.
    ctx.emplace(ev, outputToken_, gpuAlgo_.getResults(ctx.stream()));

    for (auto& buf : buffers)
      buf.reset(nullptr);
  }

private:
  void run(const FEDRawDataCollection& rawColl, const SiStripClusterizerConditions& conditions);
  void fill(uint32_t idet, const FEDRawDataCollection& rawColl, const SiStripClusterizerConditions& conditions);

private:
  std::vector<std::unique_ptr<sistrip::FEDBuffer>> buffers;
  std::vector<const FEDRawData*> raw;
  cms::cuda::ContextState ctxState_;

  stripgpu::SiStripRawToClusterGPUKernel gpuAlgo_;

  edm::EDGetTokenT<FEDRawDataCollection> inputToken_;
  edm::EDPutTokenT<cms::cuda::Product<SiStripClustersCUDA>> outputToken_;

  edm::ESGetToken<SiStripConditionsGPUWrapper, SiStripClusterizerGPUConditionsRcd> conditionsToken_;
  edm::ESGetToken<SiStripClusterizerConditions, SiStripClusterizerConditionsRcd> CPUconditionsToken_;
};

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(SiStripClusterizerFromRawGPU);

void SiStripClusterizerFromRawGPU::run(const FEDRawDataCollection& rawColl,
                                       const SiStripClusterizerConditions& conditions) {
  // loop over good det in cabling
  for (auto idet : conditions.allDetIds()) {
    fill(idet, rawColl, conditions);
  }  // end loop over dets
}

void SiStripClusterizerFromRawGPU::fill(uint32_t idet,
                                        const FEDRawDataCollection& rawColl,
                                        const SiStripClusterizerConditions& conditions) {
  auto const& det = conditions.findDetId(idet);
  if (!det.valid())
    return;

  // Loop over apv-pairs of det
  for (auto const conn : conditions.currentConnection(det)) {
    if UNLIKELY (!conn)
      continue;

    const uint16_t fedId = conn->fedId();

    // If fed id is null or connection is invalid continue
    if UNLIKELY (!fedId || !conn->isConnected()) {
      continue;
    }

    // If Fed hasnt already been initialised, extract data and initialise
    sistrip::FEDBuffer* buffer = buffers[fedId].get();
    if (!buffer) {
      const FEDRawData& rawData = rawColl.FEDData(fedId);
      raw[fedId] = &rawData;
      buffer = fillBuffer(fedId, rawData).release();
      if (!buffer) {
        continue;
      }
      buffers[fedId].reset(buffer);
    }
    assert(buffer);
  }  // end loop over conn
}
