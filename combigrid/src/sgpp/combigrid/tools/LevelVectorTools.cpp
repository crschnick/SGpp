// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#include <sgpp/combigrid/tools/LevelVectorTools.hpp>

#include <algorithm>
#include <cassert>
#include <numeric>
#include <vector>

namespace sgpp {
namespace combigrid {

std::vector<LevelVector> LevelVectorTools::generateHyperCube(
    const LevelVector& minLevel, const LevelVector& maxLevel) {
  return generateHyperCubeRecursive(minLevel, maxLevel, LevelVector{});
}

std::vector<LevelVector> LevelVectorTools::generateHyperCubeRecursiveLastDim(
    const LevelVector& minLevel, const LevelVector& maxLevel, const LevelVector& prefix) {
  assert(prefix.size() == minLevel.size() - 1);
  assert(minLevel.back() <= maxLevel.back());
  std::vector<LevelVector> oneDRange;
  LevelVector current = prefix;
  current.push_back(0);

  for (level_t l = minLevel.back(); l <= maxLevel.back(); ++l) {
    current[prefix.size()] = l;
    oneDRange.push_back(current);
  }

  return oneDRange;
}

std::vector<LevelVector> LevelVectorTools::generateHyperCubeRecursive(
    const LevelVector& minLevel, const LevelVector& maxLevel, const LevelVector& prefix) {
  std::vector<LevelVector> currentVector;
  const size_t currentDim = prefix.size();

  // if we are at the last dimension, add one one-dimensional row of the hypercube
  if (currentDim == minLevel.size() - 1) {
    const std::vector<LevelVector> newPole =
        generateHyperCubeRecursiveLastDim(minLevel, maxLevel, prefix);
    currentVector.insert(currentVector.end(), newPole.begin(), newPole.end());
  } else {
    LevelVector newPrefix = prefix;
    newPrefix.push_back(0);

    // else, recurse to the next dimension
    for (level_t l = minLevel[currentDim]; l <= maxLevel[currentDim]; ++l) {
      newPrefix[prefix.size()] = l;
      const std::vector<LevelVector> newHypercube =
          generateHyperCubeRecursive(minLevel, maxLevel, newPrefix);
      currentVector.insert(currentVector.end(), newHypercube.begin(), newHypercube.end());
    }
  }

  return currentVector;
}

std::vector<LevelVector> LevelVectorTools::generateDiagonal(
    const LevelVector& minLevel, level_t levelSum) {
  if (minLevel.empty()) {
    if (levelSum == 0) {
      return std::vector<LevelVector>{LevelVector{}};
    } else {
      return std::vector<LevelVector>{};
    }
  } else {
    const level_t minLevelSum = std::accumulate(minLevel.begin(), minLevel.end(), 0);
    std::vector<LevelVector> result;
    generateDiagonalRecursive(minLevel, minLevelSum, levelSum, LevelVector{}, result);
    return result;
  }
}

void LevelVectorTools::generateDiagonalRecursive(
    const LevelVector& minLevel, level_t minLevelSum, level_t levelSum, const LevelVector& suffix,
    std::vector<LevelVector>& result) {
  const size_t dim = minLevel.size();
  const size_t currentDim = dim - suffix.size();

  if (levelSum < minLevelSum) {
    return;
  } else if (currentDim == 1) {
    LevelVector l{levelSum};
    l.insert(l.end(), suffix.begin(), suffix.end());
    result.push_back(l);
  } else {
    const level_t newMinLevelSum = minLevelSum - minLevel[currentDim - 1];
    LevelVector newSuffix{0};
    newSuffix.insert(newSuffix.end(), suffix.begin(), suffix.end());

    for (level_t l = minLevel[currentDim - 1]; l <= levelSum - newMinLevelSum; l++) {
      newSuffix[0] = l;
      generateDiagonalRecursive(
          minLevel, newMinLevelSum, levelSum - l, newSuffix, result);
    }
  }
}

std::vector<LevelVector> LevelVectorTools::generateDiagonalWithBoundary(
    size_t dim, level_t levelSum) {
  return generateDiagonal(LevelVector(dim, 0), levelSum);
}

std::vector<LevelVector> LevelVectorTools::generateDiagonalWithoutBoundary(
    size_t dim, level_t levelSum) {
  return generateDiagonal(LevelVector(dim, 1), levelSum);
}

std::vector<LevelVector> LevelVectorTools::makeDownwardClosed(
    const std::vector<LevelVector>& subspaceLevels, LevelVector lowestLevelVector) {
  assert(lowestLevelVector.size() == subspaceLevels[0].size());
  std::vector<LevelVector> downwardClosedSet = subspaceLevels;

  // for each subspace level, ...
  for (const LevelVector& subspaceLevel : subspaceLevels) {
    // add the full hypercube of lower levels, if not already present
    for (const LevelVector& level : generateHyperCube(lowestLevelVector, subspaceLevel)) {
      if (std::find(downwardClosedSet.begin(), downwardClosedSet.end(), level) ==
          downwardClosedSet.end()) {
        downwardClosedSet.push_back(level);
      }
    }
  }

  std::sort(downwardClosedSet.begin(), downwardClosedSet.end());
  return downwardClosedSet;
}

}  // namespace combigrid
}  // namespace sgpp
