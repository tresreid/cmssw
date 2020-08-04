#ifndef _CLUSTER_GPU_KERNEL_
#define _CLUSTER_GPU_KERNEL_

#include "CUDADataFormats/SiStripCluster/interface/SiStripClustersCUDA.h"
#include "RecoLocalTracker/SiStripClusterizer/interface/SiStripConditionsGPU.h"

#include <cstdint>

namespace stripgpu {
  class StripDataGPU;
}
struct ChanLocStruct;

static constexpr auto MAX_SEEDSTRIPS = 200000;
static constexpr uint32_t kClusterMaxStrips = 16;

struct sst_data_t {
  const ChanLocStruct* chanlocs;
  uint16_t* channel;
  stripgpu::stripId_t *stripId;
  uint8_t *adc;
  int *seedStripsNCIndex, *seedStripsMask, *seedStripsNCMask, *prefixSeedStripsNCMask;
  void *d_temp_storage;
  size_t temp_storage_bytes;
  int nSeedStripsNC;
  int nStrips;
};

struct clust_data_t {
  int *clusterIndexLeft, *clusterSize;
  uint8_t *clusterADCs;
  stripgpu::detId_t *clusterDetId;
  stripgpu::stripId_t *firstStrip;
  bool *trueCluster;
  float *barycenter;
  int nSeedStripsNC;
};
#endif
