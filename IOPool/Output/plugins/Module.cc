#include "FWCore/Framework/interface/MakerMacros.h"
#include "IOPool/Output/interface/PoolOutputModule.h"
#include "IOPool/Output/src/ParallelPoolOutputModule.h"
#include "IOPool/Output/interface/TimeoutPoolOutputModule.h"

using edm::ParallelPoolOutputModule;
using edm::PoolOutputModule;
using edm::TimeoutPoolOutputModule;
DEFINE_FWK_MODULE(PoolOutputModule);
DEFINE_FWK_MODULE(ParallelPoolOutputModule);
DEFINE_FWK_MODULE(TimeoutPoolOutputModule);
