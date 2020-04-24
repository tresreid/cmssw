#ifndef _CLUSTER_GPU_KERNEL_
#define _CLUSTER_GPU_KERNEL_

#include "RecoLocalTracker/SiStripClusterizer/interface/SiStripConditionsGPU.h"
#include "unpackGPU.cuh"

#include <cstdint>

static constexpr auto MAX_SEEDSTRIPS = 200000;
static constexpr auto kClusterMaxStrips = 16;

struct sst_data_t {
  stripgpu::detId_t *detId;
  stripgpu::stripId_t *stripId;
  uint8_t *adc;
  stripgpu::fedId_t *fedId;
  stripgpu::fedCh_t *fedCh;
  int *seedStripsNCIndex, *seedStripsMask, *seedStripsNCMask, *prefixSeedStripsNCMask;
  void *d_temp_storage;
  size_t temp_storage_bytes;
  int nSeedStripsNC;
  int nStrips;
};

struct clust_data_t {
  int *clusterLastIndexLeft, *clusterLastIndexRight;
  uint8_t *clusterADCs;
  stripgpu::detId_t *clusterDetId;
  stripgpu::stripId_t *firstStrip;
  bool *trueCluster;
  float *barycenter;
  int nSeedStripsNC;
};

void allocateSSTDataGPU(int max_strips, StripDataGPU* stripdata, sst_data_t *sst_data_d, sst_data_t **pt_sst_data_d, cudaStream_t stream);
void allocateClustDataGPU(int max_strips, clust_data_t *clust_data_d, clust_data_t **pt_clust_data_t, cudaStream_t stream);
void allocateClustData(int max_seedstrips, clust_data_t *clust_data, cudaStream_t stream);

void freeSSTDataGPU(sst_data_t *sst_data_d, sst_data_t *pt_sst_data_d, cudaStream_t stream);
void freeClustDataGPU(clust_data_t *clust_data_d, clust_data_t *pt_clust_data_d, cudaStream_t stream);
void freeClustData(clust_data_t *clust_data_t);

void setSeedStripsNCIndexGPU(sst_data_t *sst_data_d, sst_data_t *pt_sst_data_d, const SiStripConditionsGPU *conditions, cudaStream_t stream);
void findClusterGPU(sst_data_t *sst_data_d, sst_data_t *pt_sst_data_d, const SiStripConditionsGPU *conditions, clust_data_t *clust_data, clust_data_t *clust_data_d, clust_data_t *pt_clust_data_d, cudaStream_t stream);

void cpyGPUToCPU(clust_data_t *clust_data, clust_data_t *clust_data_d, cudaStream_t stream);
#endif
