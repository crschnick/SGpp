/**
 * Copyright (C) 2008-today The SG++ project
 * This file is part of the SG++ project. For conditions of distribution and
 * use, please see the copyright notice provided with SG++ or at
 * sgpp.sparsegrids.org
 *
 * GridFactory.cpp
 *
 *  Created on: May 22, 2018
 *      Author: dominik
 */

#include <sgpp/base/exception/algorithm_exception.hpp>
#include <sgpp/base/grid/Grid.hpp>
#include <sgpp/base/grid/GridDataBase.hpp>
#include <sgpp/base/operation/hash/OperationMatrix.hpp>
#include <sgpp/datadriven/algorithm/GridFactory.hpp>

#include <set>
#include <string>
#include <vector>

using sgpp::base::algorithm_exception;
using sgpp::base::Grid;
using sgpp::base::GridGenerator;
using sgpp::base::GridType;

namespace sgpp {
namespace datadriven {

sgpp::base::Grid* GridFactory::createGrid(const sgpp::base::GeneralGridConfiguration& gridConfig,
                                          const std::set<std::set<size_t>> interactions) const {
  Grid* tmpGrid;
  if (gridConfig.type_ == GridType::Linear) {
    tmpGrid = Grid::createLinearGrid(gridConfig.dim_);
  } else if (gridConfig.type_ == GridType::LinearL0Boundary) {
    tmpGrid = Grid::createLinearBoundaryGrid(
        gridConfig.dim_, static_cast<base::GridPoint::level_type>(gridConfig.boundaryLevel_));
  } else if (gridConfig.type_ == GridType::LinearBoundary) {
    tmpGrid = Grid::createLinearBoundaryGrid(gridConfig.dim_);
  } else if (gridConfig.type_ == GridType::ModLinear) {
    tmpGrid = Grid::createModLinearGrid(gridConfig.dim_);
  } else {
    throw algorithm_exception("LearnerBase::InitializeGrid: An unsupported grid type was chosen!");
  }

  // Generate component grid
  if (gridConfig.generalType_ == sgpp::base::GeneralGridType::ComponentGrid) {
    tmpGrid->getGenerator().anisotropicFull(gridConfig.levelVector_);
    return tmpGrid;
  }

  // Generate regular Grid with LEVELS Levels
  if (interactions.size() == 0) {
    tmpGrid->getGenerator().regular(gridConfig.level_);
  } else {
    std::cout << "Creating geometry aware sparse grid..." << std::endl;
    tmpGrid->getGenerator().regularInter(gridConfig.level_, interactions, 0.0);
    std::cout << "Interactions set!" << std::endl;
  }
  std::cout << "Grid Size: " << tmpGrid->getSize() << std::endl;
  return tmpGrid;
}

std::set<std::set<size_t>> sgpp::datadriven::GridFactory::getInteractions(
    sgpp::datadriven::GeometryConfiguration config) const {
  std::set<std::set<size_t>> interactions;
  std::vector<std::vector<int64_t>> res = config.dim;

  interactions = std::set<std::set<size_t>>();
  for (StencilConfiguration stencil : config.stencils) {
    switch (stencil.stencilType) {
      case sgpp::datadriven::StencilType::DirectNeighbour:
        getDirectNeighbours(interactions, config, stencil);
        break;
      case sgpp::datadriven::StencilType::NextHierarchicalParent:
      case sgpp::datadriven::StencilType::AllHierarchicalParent:
        getHierarchicalParents(interactions, config, stencil);
        break;
      case sgpp::datadriven::StencilType::None:
        break;
      default:
        std::cout << "Stencil not found" << std::endl;
        break;
    }
  }
  return interactions;
}

std::vector<std::vector<size_t>> sgpp::datadriven::GridFactory::getMultiplicatorsPerLevel(
    std::vector<std::vector<int64_t>>& imageDimensions) const {
  auto multiplicatorsPerLevel = std::vector<std::vector<size_t>>();
  for (auto dimension : imageDimensions) {
    auto multiplicators = std::vector<size_t>();
    multiplicators.push_back(1);
    for (size_t i = 1; i < dimension.size(); i++) {
      multiplicators.push_back(multiplicators.at(i - 1) * dimension.at(i - 1));
    }
    multiplicatorsPerLevel.push_back(multiplicators);
  }
  return multiplicatorsPerLevel;
}

std::vector<size_t> sgpp::datadriven::GridFactory::getOffsetPerLevel(
    std::vector<std::vector<int64_t>>& imageDimensions,
    std::vector<std::vector<size_t>>& multiplicatorsPerLevel) const {
  auto offsetsPerLevel = std::vector<size_t>();
  offsetsPerLevel.push_back(0);
  for (size_t i = 1; i < imageDimensions.size(); i++) {
    auto childDimensions = imageDimensions.at(i - 1);
    auto childMultiplicator = multiplicatorsPerLevel.at(i - 1);
    auto numberOfDataColumnsInChild = childMultiplicator.at(childMultiplicator.size() - 1) *
                                      childDimensions.at(childDimensions.size() - 1);
    auto childOffset = offsetsPerLevel.at(i - 1);
    offsetsPerLevel.push_back(childOffset + numberOfDataColumnsInChild);
  }
  return offsetsPerLevel;
}

void sgpp::datadriven::GridFactory::getHierarchicalParents(
    std::set<std::set<size_t>>& interactions, sgpp::datadriven::GeometryConfiguration& geometryConf,
    sgpp::datadriven::StencilConfiguration& stencilConf) const {
  auto multiplicatorsPerLevel = getMultiplicatorsPerLevel(geometryConf.dim);
  auto offsetsPerLevel = getOffsetPerLevel(geometryConf.dim, multiplicatorsPerLevel);

  for (size_t i : stencilConf.applyOnLayers) {
    for (size_t j = i + 1; j < geometryConf.dim.size(); j++) {
      auto scale = std::vector<double>();
      for (size_t k = 0; k < geometryConf.dim.at(i).size(); k++) {
        scale.push_back(static_cast<double>(geometryConf.dim.at(j).at(k)) /
                        static_cast<double>(geometryConf.dim.at(i).at(k)));
      }

      auto position = std::vector<int64_t>(geometryConf.dim.at(j).size(), 0);

      // Iterate over all cells and add its possible parents
      do {
        std::vector<int64_t> parentPosition = std::vector<int64_t>(position.size(), 0);
        addChildParentInteractionRecursive(scale, geometryConf.dim.at(i), 0, position,
                                           parentPosition, multiplicatorsPerLevel.at(j),
                                           multiplicatorsPerLevel.at(i), offsetsPerLevel.at(j),
                                           offsetsPerLevel.at(i), interactions);

        getNextPosition(geometryConf.dim.at(i), position, -1);
      } while (position.size() != 0);
      if (stencilConf.stencilType == StencilType::NextHierarchicalParent) {
        break;
      }
    }
    if (stencilConf.colorIndex != -1) {
      addColorInteractions(geometryConf, stencilConf, i, offsetsPerLevel.at(i),
                           multiplicatorsPerLevel.at(i), interactions);
    }
    addOneDimensionalInteractions(geometryConf.dim.at(i), offsetsPerLevel.at(i), interactions);
  }

  interactions.insert(std::set<size_t>());
}

void sgpp::datadriven::GridFactory::addOneDimensionalInteractions(
    std::vector<int64_t>& imageDimensions, size_t offset, std::set<std::set<size_t>>& vec) const {
  size_t factor = 1;
  for (auto dimension : imageDimensions) {
    factor *= dimension;
  }

  for (size_t i = offset; i < factor + offset; i++) {
    vec.insert({i});
  }
}

void sgpp::datadriven::GridFactory::addChildParentInteractionRecursive(
    std::vector<double>& rescale, std::vector<int64_t>& childDim, size_t currentDimension,
    std::vector<int64_t>& childPosition, std::vector<int64_t>& parentPosition,
    std::vector<size_t>& parentMultiplicators, std::vector<size_t>& childMultiplicators,
    size_t parentOffset, size_t childOffset, std::set<std::set<size_t>>& res) const {
  if (currentDimension < childDim.size()) {
    size_t position = static_cast<size_t>(rescale.at(currentDimension) *
                                          static_cast<double>(childPosition.at(currentDimension)));
    double nextPosition =
        rescale.at(currentDimension) * static_cast<double>(childPosition.at(currentDimension) + 1);
    // iterate over all positions inbetween (neccesarry for increasing dimensions)
    for (size_t i = position; static_cast<double>(i) < nextPosition; i++) {
      parentPosition.at(currentDimension) = i;
      addChildParentInteractionRecursive(rescale, childDim, currentDimension + 1, childPosition,
                                         parentPosition, parentMultiplicators, childMultiplicators,
                                         parentOffset, childOffset, res);
    }
  } else {
    auto tmp = std::set<size_t>();
    tmp.insert(getDataIndex(childDim.size(), childMultiplicators, childPosition) + childOffset);
    tmp.insert(getDataIndex(childDim.size(), parentMultiplicators, parentPosition) + parentOffset);
    res.insert(tmp);
  }
}

size_t sgpp::datadriven::GridFactory::getDataIndex(size_t numberOfDimensions,
                                                   std::vector<size_t>& multiplicators,
                                                   std::vector<int64_t>& position) const {
  size_t index = 0;
  for (size_t i = 0; i < numberOfDimensions; i++) {
    index += multiplicators.at(i) * position.at(i);
  }
  return index;
}

void sgpp::datadriven::GridFactory::getNextPosition(std::vector<int64_t>& dimension,
                                                    std::vector<int64_t>& position,
                                                    size_t colorIndex) const {
  for (size_t i = 0; i < position.size(); i++) {
    if (i == colorIndex) {
      continue;
    }
    if (position.at(i) + 1 < dimension.at(i)) {
      position.at(i)++;
      break;
    } else {
      if (i == position.size() - 1)  // last element reached
      {
        position.clear();
      } else {
        position.at(i) = 0;
      }
    }
  }
}

void sgpp::datadriven::GridFactory::addColorInteractions(
    GeometryConfiguration& geometryConf, StencilConfiguration& stencilConf, size_t layer,
    size_t offset, std::vector<size_t>& multiplicators,
    std::set<std::set<size_t>>& interactions) const {
  std::vector<int64_t> position = std::vector<int64_t>(geometryConf.dim.at(layer).size(), 0);
  while (position.size() != 0) {
    size_t colorChannels = geometryConf.dim.at(layer).at(stencilConf.colorIndex);
    for (size_t j = 1; j < pow(2, static_cast<double>(colorChannels)); j++) {
      std::set<size_t> tmp = std::set<size_t>();
      for (size_t k = 0; k < colorChannels; k++) {
        if (j & (1 << k)) {
          position.at(stencilConf.colorIndex) = k;
          tmp.insert(offset +
                     getDataIndex(geometryConf.dim.at(layer).size(), multiplicators, position));
        }
      }
      interactions.insert(tmp);
    }
    getNextPosition(geometryConf.dim.at(layer), position, stencilConf.colorIndex);
  }
}

void sgpp::datadriven::GridFactory::getDirectNeighbours(
    std::set<std::set<size_t>>& interactions, sgpp::datadriven::GeometryConfiguration& geometryConf,
    sgpp::datadriven::StencilConfiguration& stencilConf) const {
  auto multiplicatorsPerLevel = getMultiplicatorsPerLevel(geometryConf.dim);
  auto offsetsPerLevel = getOffsetPerLevel(geometryConf.dim, multiplicatorsPerLevel);

  for (size_t i : stencilConf.applyOnLayers) {
    std::vector<int64_t> position = std::vector<int64_t>(geometryConf.dim.at(i).size(), 0);

    do {
      for (size_t j = 0; j < geometryConf.dim.at(i).size(); j++) {
        if (position.at(j) + 1 < geometryConf.dim.at(i).at(j)) {
          std::set<size_t> tmp = std::set<size_t>();
          tmp.insert(offsetsPerLevel.at(i) + getDataIndex(geometryConf.dim.at(i).size(),
                                                          multiplicatorsPerLevel.at(i), position));
          position.at(j)++;
          tmp.insert(offsetsPerLevel.at(i) + getDataIndex(geometryConf.dim.at(i).size(),
                                                          multiplicatorsPerLevel.at(i), position));
          position.at(j)--;
          interactions.insert(tmp);
        }
      }

      // Color index set to -1 to also include neighbours with same color
      getNextPosition(geometryConf.dim.at(i), position, -1);
    } while (position.size() != 0);

    if (stencilConf.colorIndex != -1) {
      addColorInteractions(geometryConf, stencilConf, i, offsetsPerLevel.at(i),
                           multiplicatorsPerLevel.at(i), interactions);
    }
    addOneDimensionalInteractions(geometryConf.dim.at(i), offsetsPerLevel.at(i), interactions);
  }

  // add empty vector
  interactions.insert(std::set<size_t>());
}
}  // namespace datadriven
}  // namespace sgpp
