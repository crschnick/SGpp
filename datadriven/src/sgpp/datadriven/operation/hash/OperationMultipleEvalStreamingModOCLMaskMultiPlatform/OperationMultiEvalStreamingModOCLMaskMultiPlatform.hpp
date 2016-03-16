// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#pragma once

#include <omp.h>

#include <chrono>
#include <algorithm>
#include <vector>

#include "sgpp/base/operation/hash/OperationMultipleEval.hpp"
#include "sgpp/base/tools/SGppStopwatch.hpp"
#include "sgpp/base/exception/operation_exception.hpp"
#include "sgpp/globaldef.hpp"
#include "sgpp/base/opencl/OCLOperationConfiguration.hpp"
#include "sgpp/base/opencl/OCLManagerMultiPlatform.hpp"
#include "sgpp/base/opencl/QueueLoadBalancer.hpp"
#include "Configuration.hpp"
#include "KernelMult.hpp"
#include "KernelMultTranspose.hpp"

namespace sgpp {
namespace datadriven {

template <typename T>
class OperationMultiEvalStreamingModOCLMaskMultiPlatform : public base::OperationMultipleEval {
 protected:
  bool verbose;
  size_t dims;
  sgpp::base::DataMatrix preparedDataset;
  std::shared_ptr<base::OCLOperationConfiguration> parameters;
  std::vector<T> kernelDataset;
  size_t datasetSizeUnpadded;
  size_t datasetSizePadded;
  size_t datasetSizeBuffers;
  /// Member to store the sparse grid's levels for better vectorization
  std::vector<T> level;
  /// Member to store the sparse grid's indices for better vectorization
  std::vector<T> index;
  /// Member to store the sparse grid's mask for better vectorization
  std::vector<T> mask;
  /// Member to store the sparse grid's offset for better vectorization
  std::vector<T> offset;
  size_t gridSizeUnpadded;
  size_t gridSizePadded;
  size_t gridSizeBuffers;

  /// Timer object to handle time measurements
  sgpp::base::SGppStopwatch myTimer;

  base::GridStorage &storage;

  double duration;

  std::shared_ptr<base::OCLManagerMultiPlatform> manager;
  std::vector<std::shared_ptr<base::OCLDevice>> devices;

  std::vector<StreamingModOCLMaskMultiPlatform::KernelMult<T>> multKernels;
  std::vector<StreamingModOCLMaskMultiPlatform::KernelMultTranspose<T>> multTransposeKernels;

  std::shared_ptr<sgpp::base::QueueLoadBalancer> queueLoadBalancerMult;
  std::shared_ptr<sgpp::base::QueueLoadBalancer> queueLoadBalancerMultTrans;

  size_t overallGridBlockingSize;
  size_t overallDataBlockingSize;

 public:
  OperationMultiEvalStreamingModOCLMaskMultiPlatform(
      base::Grid &grid, base::DataMatrix &dataset,
      std::shared_ptr<base::OCLManagerMultiPlatform> manager,
      std::shared_ptr<base::OCLOperationConfiguration> parameters)
      : OperationMultipleEval(grid, dataset),
        preparedDataset(dataset),
        parameters(parameters),
        myTimer(sgpp::base::SGppStopwatch()),
        storage(grid.getStorage()),
        duration(-1.0),
        manager(manager),
        devices(manager->getDevices()) {
    this->verbose = (*parameters)["VERBOSE"].getBool();

    this->dims = dataset.getNcols();  // be aware of transpose!

    // padded grid size is set by prepare
    this->gridSizeUnpadded = storage.getSize();
    // initialized in prepare
    this->gridSizePadded = 0;
    this->gridSizeBuffers = 0;
    this->datasetSizeUnpadded = this->dataset.getNrows();
    // initialize in pad
    this->datasetSizePadded = 0;
    this->datasetSizeBuffers = 0;
    this->padDataset(this->preparedDataset);
    this->preparedDataset.transpose();

    overallGridBlockingSize = calculateCommonGridPadding();
    overallDataBlockingSize = calculateCommonDatasetPadding();

    std::cout << "overallDataBlockingSize: " << overallDataBlockingSize << std::endl;
    std::cout << "overallGridBlockingSize: " << overallGridBlockingSize << std::endl;

    queueLoadBalancerMult = std::make_shared<sgpp::base::QueueLoadBalancer>();
    queueLoadBalancerMultTrans = std::make_shared<sgpp::base::QueueLoadBalancer>();

    //    std::cout << "dims: " << this->dims << std::endl;
    //    std::cout << "padded instances: " << this->datasetSize << std::endl;

    // corresponds to size of dim * datasetSizeBuffers
    this->kernelDataset =
        std::vector<T>(this->preparedDataset.getNrows() * this->preparedDataset.getNcols());

    for (size_t i = 0; i < this->preparedDataset.getSize(); i++) {
      this->kernelDataset[i] = (T) this->preparedDataset[i];
    }

    for (size_t deviceIndex = 0; deviceIndex < devices.size(); deviceIndex++) {
      json::Node &platformConfiguration =
          (*parameters)["PLATFORMS"][devices[deviceIndex]->platformName];
      json::Node &deviceConfiguration =
          platformConfiguration["DEVICES"][devices[deviceIndex]->deviceName];
      json::Node &kernelConfiguration = deviceConfiguration
          ["KERNELS"][StreamingModOCLMaskMultiPlatform::Configuration::getKernelName()];

      multKernels.emplace_back(devices[deviceIndex], dims, this->manager, kernelConfiguration,
                               queueLoadBalancerMult);

      multTransposeKernels.emplace_back(devices[deviceIndex], dims, this->manager,
                                        kernelConfiguration, queueLoadBalancerMultTrans);
    }

    // create the kernel specific data structures
    // also sets the correct padded grid size
    this->prepare();
  }

  ~OperationMultiEvalStreamingModOCLMaskMultiPlatform() {}

  void mult(sgpp::base::DataVector &alpha, sgpp::base::DataVector &result) override {
    this->prepare();

    this->myTimer.start();

    size_t gridFrom = 0;
    size_t gridTo = this->gridSizePadded;
    size_t datasetFrom = 0;
    size_t datasetTo = this->datasetSizePadded;

    queueLoadBalancerMult->initialize(datasetFrom, datasetTo);

    std::vector<T> alphaArray(this->gridSizePadded);

    for (size_t i = 0; i < alpha.getSize(); i++) {
      alphaArray[i] = (T)alpha[i];
    }

    for (size_t i = alpha.getSize(); i < this->gridSizePadded; i++) {
      alphaArray[i] = 0.0;
    }

    //    std::cout << "--- printing non-zero alpha: ---" << std::endl;
    //
    //    for (size_t i = 0; i < 100; i++) {
    //      if (alpha[i] != 0.0) {
    //        std::cout << "alpha[" << i << "] = " << alpha[i] << std::endl;
    //      }
    //    }
    //    std::cout << "--- finished printing non-zero alpha: ---" << std::endl;

    std::vector<T> resultArray(this->datasetSizeBuffers);
    std::fill(resultArray.begin(), resultArray.end(), 0.0);

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    omp_set_num_threads(static_cast<int>(devices.size()));

#pragma omp parallel
    {
      size_t threadId = omp_get_thread_num();
      //            std::cout << "threadId: " << threadId << std::endl;
      this->multKernels[threadId].mult(this->level, this->index, this->mask, this->offset,
                                       this->kernelDataset, alphaArray, resultArray, gridFrom,
                                       gridTo, datasetFrom, datasetTo);
    }
    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;

    if (verbose) {
      std::cout << "duration mult ocl mod: " << elapsed_seconds.count() << std::endl;
    }

    for (size_t i = 0; i < result.getSize(); i++) {
      result[i] = resultArray[i];
    }

    //    std::cout << "--- printing non-zero result: ---" << std::endl;
    //
    //    for (size_t i = 0; i < 100; i++) {
    //      if (resultArray[i] != 0.0) {
    //        std::cout << "resultArray[" << i << "] = " << resultArray[i] << std::endl;
    //      }
    //    }
    //    std::cout << "--- finished printing non-zero result: ---" << std::endl;

    this->duration = this->myTimer.stop();
  }

  void multTranspose(sgpp::base::DataVector &source, sgpp::base::DataVector &result) override {
    this->prepare();

    this->myTimer.start();

    size_t gridFrom = 0;
    size_t gridTo = this->gridSizePadded;
    size_t datasetFrom = 0;
    size_t datasetTo = this->datasetSizePadded;

    //        if (omp_get_thread_num() == 0) {
    //        queueLoadBalancerMultTrans->initialize(scheduleSizeTranspose,
    //        gridFrom, gridTo, overallGridBlockingSize);
    //        }

    // #pragma omp barrier

    queueLoadBalancerMultTrans->initialize(gridFrom, gridTo);

    std::vector<T> sourceArray(this->datasetSizePadded);

    for (size_t i = 0; i < source.getSize(); i++) {
      sourceArray[i] = (T)source[i];
    }

    for (size_t i = source.getSize(); i < this->datasetSizePadded; i++) {
      sourceArray[i] = 0.0;
    }

    std::vector<T> resultArray(this->gridSizeBuffers);

    std::fill(resultArray.begin(), resultArray.end(), 0.0);

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    omp_set_num_threads(static_cast<int>(devices.size()));

#pragma omp parallel
    {
      size_t threadId = omp_get_thread_num();

      this->multTransposeKernels[threadId].multTranspose(
          this->level, this->index, this->mask, this->offset, this->kernelDataset, sourceArray,
          resultArray, gridFrom, gridTo, datasetFrom, datasetTo);
    }
    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    if (verbose) {
      std::cout << "duration multTranspose ocl mod: " << elapsed_seconds.count() << std::endl;
    }

    for (size_t i = 0; i < result.getSize(); i++) {
      result[i] = resultArray[i];
    }

    this->duration = this->myTimer.stop();
  }

  double getDuration() { return this->duration; }

  void prepare() override {
    this->recalculateLevelIndexMask();

    for (size_t deviceIndex = 0; deviceIndex < devices.size(); deviceIndex++) {
      this->multKernels[deviceIndex].resetKernel();
    }

    for (size_t deviceIndex = 0; deviceIndex < devices.size(); deviceIndex++) {
      this->multTransposeKernels[deviceIndex].resetKernel();
    }
  }

 private:
  void padDataset(sgpp::base::DataMatrix &dataset) {
    size_t oldSize = dataset.getNrows();

    size_t commonDatasetPadding = calculateCommonDatasetPadding();

    // Assure that data has a even number of instances -> padding might be
    // needed
    size_t remainder = oldSize % commonDatasetPadding;
    // round up to next number divisible by padding (distributable to threads)
    // then add another padding (for irregular schedules)
    size_t padding = commonDatasetPadding - remainder + commonDatasetPadding;

    sgpp::base::DataVector lastRow(dataset.getNcols());
    dataset.getRow(oldSize - 1, lastRow);
    dataset.resize(oldSize + padding);

    for (size_t i = 0; i < padding; i++) {
      dataset.setRow(oldSize + i, lastRow);
    }

    // excluding the additional padding for irregular schedules
    this->datasetSizePadded = oldSize + commonDatasetPadding - remainder;
    // totol size for buffer allocation
    this->datasetSizeBuffers = oldSize + commonDatasetPadding - remainder + commonDatasetPadding;
    std::cout << "datasetSizeUnpadded: " << datasetSizeUnpadded << std::endl;
    std::cout << "datasetSizePadded: " << datasetSizePadded << std::endl;
    std::cout << "datasetSizeBuffers: " << datasetSizeBuffers << std::endl;
  }

  /**
   * Converts this storage from AOS (array of structures) to SOA (structure of
   * array)
   * with modification to speed up iterative function evaluation. The Level
   * array won't contain the levels, it contains the level to the power of two.
   *
   * The returned format is only useful for a multi-evaluation of modlinear
   * grids
   */
  void recalculateLevelIndexMask() {
    //    size_t commonGridPadding = calculateCommonGridPadding();

    size_t remainder = storage.getSize() % overallGridBlockingSize;
    size_t padding = 0;

    if (remainder != 0) {
      padding = overallGridBlockingSize - remainder;
    }

    // size to distribute, not actual padded grid size
    this->gridSizePadded = storage.getSize() + padding;

    // size for distributing schedules of different size
    this->gridSizeBuffers = storage.getSize() + padding + overallGridBlockingSize;

    std::cout << "gridSizeUnpadded: " << storage.getSize() << std::endl;
    std::cout << "gridSizePadded: " << gridSizePadded << std::endl;
    std::cout << "gridSizeBuffers: " << gridSizeBuffers << std::endl;

    sgpp::base::HashGridIndex::level_type curLevel;
    sgpp::base::HashGridIndex::index_type curIndex;

    this->level = std::vector<T>(gridSizeBuffers * this->dims);
    this->index = std::vector<T>(gridSizeBuffers * this->dims);
    this->mask = std::vector<T>(gridSizeBuffers * this->dims);
    this->offset = std::vector<T>(gridSizeBuffers * this->dims);

    for (size_t i = 0; i < storage.getSize(); i++) {
      for (size_t dim = 0; dim < this->dims; dim++) {
        storage.get(i)->get(dim, curLevel, curIndex);

        if (curLevel == 1) {
          this->level[i * this->dims + dim] = 0.0;
          this->index[i * this->dims + dim] = 0.0;
          if (std::is_same<T, double>::value) {
            uint64_t intmask = 0x0000000000000000;
            this->mask[i * this->dims + dim] = *reinterpret_cast<T *>(&intmask);
          } else {
            uint32_t intmask = 0x00000000;
            this->mask[i * this->dims + dim] = *reinterpret_cast<T *>(&intmask);
          }
          this->offset[i * this->dims + dim] = 1.0;
        } else if (curIndex == 1) {
          this->level[i * this->dims + dim] = static_cast<T>(-1.0) * static_cast<T>(1 << curLevel);
          this->index[i * this->dims + dim] = 0.0;
          if (std::is_same<T, double>::value) {
            uint64_t intmask = 0x0000000000000000;
            this->mask[i * this->dims + dim] = *reinterpret_cast<T *>(&intmask);
          } else {
            uint32_t intmask = 0x00000000;
            this->mask[i * this->dims + dim] = *reinterpret_cast<T *>(&intmask);
          }
          this->offset[i * this->dims + dim] = 2.0;
        } else if (curIndex ==
                   static_cast<sgpp::base::HashGridIndex::level_type>(((1 << curLevel) - 1))) {
          this->level[i * this->dims + dim] = static_cast<T>(1 << curLevel);
          this->index[i * this->dims + dim] = static_cast<T>(curIndex);
          if (std::is_same<T, double>::value) {
            uint64_t intmask = 0x0000000000000000;
            this->mask[i * this->dims + dim] = *reinterpret_cast<T *>(&intmask);
          } else {
            uint32_t intmask = 0x00000000;
            this->mask[i * this->dims + dim] = *reinterpret_cast<T *>(&intmask);
          }
          this->offset[i * this->dims + dim] = 1.0;
        } else {
          this->level[i * this->dims + dim] = static_cast<T>(1 << curLevel);
          this->index[i * this->dims + dim] = static_cast<T>(curIndex);
          if (std::is_same<T, double>::value) {
            uint64_t intmask = 0x8000000000000000;
            this->mask[i * this->dims + dim] = *reinterpret_cast<T *>(&intmask);
          } else {
            uint32_t intmask = 0x80000000;
            this->mask[i * this->dims + dim] = *reinterpret_cast<T *>(&intmask);
          }
          this->offset[i * this->dims + dim] = 1.0;
        }
      }
    }

    for (size_t i = storage.getSize(); i < gridSizeBuffers; i++) {
      for (size_t dim = 0; dim < this->dims; dim++) {
        this->level[i * this->dims + dim] = 0;
        this->index[i * this->dims + dim] = 0;
        if (std::is_same<T, double>::value) {
          uint64_t intmask = 0x0000000000000000;
          this->mask[i * this->dims + dim] = *reinterpret_cast<T *>(&intmask);
        } else {
          uint32_t intmask = 0x00000000;
          this->mask[i * this->dims + dim] = *reinterpret_cast<T *>(&intmask);
        }
        this->offset[i * this->dims + dim] = 1.0;
      }
    }
  }

  size_t calculateCommonDatasetPadding() {
    size_t commonPaddingRequiredment = 1;
    for (size_t deviceIndex = 0; deviceIndex < devices.size(); deviceIndex++) {
      json::Node &platformConfiguration =
          (*parameters)["PLATFORMS"][devices[deviceIndex]->platformName];
      json::Node &deviceConfiguration =
          platformConfiguration["DEVICES"][devices[deviceIndex]->deviceName];
      json::Node &kernelConfiguration = deviceConfiguration
          ["KERNELS"][StreamingModOCLMaskMultiPlatform::Configuration::getKernelName()];

      commonPaddingRequiredment = std::max(commonPaddingRequiredment,
                                           kernelConfiguration["KERNEL_DATA_BLOCK_SIZE"].getUInt() *
                                               kernelConfiguration["LOCAL_SIZE"].getUInt());
    }
    return commonPaddingRequiredment;
  }

  size_t calculateCommonGridPadding() {
    size_t commonPaddingRequiredment = 1;
    for (size_t deviceIndex = 0; deviceIndex < devices.size(); deviceIndex++) {
      json::Node &platformConfiguration =
          (*parameters)["PLATFORMS"][devices[deviceIndex]->platformName];
      json::Node &deviceConfiguration =
          platformConfiguration["DEVICES"][devices[deviceIndex]->deviceName];
      json::Node &kernelConfiguration = deviceConfiguration
          ["KERNELS"][StreamingModOCLMaskMultiPlatform::Configuration::getKernelName()];

      commonPaddingRequiredment = std::max(
          commonPaddingRequiredment, kernelConfiguration["KERNEL_TRANS_GRID_BLOCK_SIZE"].getUInt() *
                                         kernelConfiguration["LOCAL_SIZE"].getUInt());
    }
    return commonPaddingRequiredment;
  }
};

}  // namespace datadriven
}  // namespace sgpp
