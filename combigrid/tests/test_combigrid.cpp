// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE SGppCombigridModule

#include <sgpp/base/grid/GridStorage.hpp>
#include <sgpp/base/operation/hash/common/basis/BsplineBasis.hpp>
#include <sgpp/base/operation/hash/common/basis/LinearBasis.hpp>

#include <sgpp/combigrid/LevelIndexTypes.hpp>
#include <sgpp/combigrid/basis/HeterogeneousBasis.hpp>
#include <sgpp/combigrid/grid/CombinationGrid.hpp>
#include <sgpp/combigrid/grid/FullGrid.hpp>
#include <sgpp/combigrid/grid/IndexVectorRange.hpp>
#include <sgpp/combigrid/operation/OperationEvalCombinationGrid.hpp>
#include <sgpp/combigrid/operation/OperationPole.hpp>
#include <sgpp/combigrid/operation/OperationPoleHierarchisationGeneral.hpp>
#include <sgpp/combigrid/operation/OperationPoleHierarchisationLinear.hpp>
#include <sgpp/combigrid/operation/OperationPoleNodalisationBspline.hpp>
#include <sgpp/combigrid/operation/OperationUPCombinationGrid.hpp>
#include <sgpp/combigrid/operation/OperationUPFullGrid.hpp>

#include <boost/test/unit_test.hpp>

#include <memory>
#include <vector>

using sgpp::base::DataVector;
using sgpp::combigrid::CombinationGrid;
using sgpp::combigrid::FullGrid;
using sgpp::combigrid::HeterogeneousBasis;
using sgpp::combigrid::IndexVector;
using sgpp::combigrid::IndexVectorRange;
using sgpp::combigrid::LevelVector;
using sgpp::combigrid::OperationEvalCombinationGrid;
using sgpp::combigrid::OperationPole;
using sgpp::combigrid::OperationPoleHierarchisationGeneral;
using sgpp::combigrid::OperationPoleHierarchisationLinear;
using sgpp::combigrid::OperationPoleNodalisationBspline;
using sgpp::combigrid::OperationUPCombinationGrid;
using sgpp::combigrid::OperationUPFullGrid;

// fix for clang (from https://stackoverflow.com/a/33755176)
#ifdef __clang__
#include <string>

namespace boost {
namespace unit_test {
namespace ut_detail {

std::string normalize_test_case_name(const_string name) {
  return ((name[0] == '&') ? std::string(name.begin() + 1, name.size() - 1) :
                             std::string(name.begin(), name.size()));
}

}  // namespace ut_detail
}  // namespace unit_test
}  // namespace boost
#endif

BOOST_AUTO_TEST_CASE(testFullGrid) {
  sgpp::base::SBsplineBase basis1d;
  HeterogeneousBasis basis(3, basis1d);
  FullGrid fullGrid({2, 3, 2}, basis);
  IndexVectorRange range(fullGrid);
  BOOST_CHECK_EQUAL(std::vector<IndexVector>(range.begin(), range.end()).size(), 225);

  /*for (const IndexVector& index : IndexVectorRange(fullGrid)) {
    std::cout << index.size() << ": " << index[0] << ", " << index[1] << ", " << index[2] << "\n";
  }*/
}

BOOST_AUTO_TEST_CASE(testCombinationGrid) {
  BOOST_CHECK_EQUAL(CombinationGrid::enumerateLevelsWithSumWithBoundary(3, 5).size(), 21);
  BOOST_CHECK_EQUAL(CombinationGrid::enumerateLevelsWithSumWithoutBoundary(3, 7).size(), 15);

  sgpp::base::SBsplineBase basis1d;
  HeterogeneousBasis basis(3, basis1d);

  for (bool hasBoundary : {true, false}) {
    std::vector<LevelVector> subspaceLevels;

    if (hasBoundary) {
      subspaceLevels = {
          {0, 0, 5}, {0, 1, 4}, {0, 2, 3}, {0, 3, 2}, {0, 4, 1}, {0, 5, 0},
          {1, 0, 4}, {1, 1, 3}, {1, 2, 2}, {1, 3, 1}, {1, 4, 0},
          {2, 0, 3}, {2, 1, 2}, {2, 2, 1}, {2, 3, 0},
          {3, 0, 2}, {3, 1, 1}, {3, 2, 0},
          {4, 0, 1}, {4, 1, 0},
          {5, 0, 0},
          {0, 0, 4}, {0, 1, 3}, {0, 2, 2}, {0, 3, 1}, {0, 4, 0},
          {1, 0, 3}, {1, 1, 2}, {1, 2, 1}, {1, 3, 0},
          {2, 0, 2}, {2, 1, 1}, {2, 2, 0},
          {3, 0, 1}, {3, 1, 0},
          {4, 0, 0},
          {0, 0, 3}, {0, 1, 2}, {0, 2, 1}, {0, 3, 0},
          {1, 0, 2}, {1, 1, 1}, {1, 2, 0},
          {2, 0, 1}, {2, 1, 0},
          {3, 0, 0},
          {0, 0, 2}, {0, 1, 1}, {0, 2, 0},
          {1, 0, 1}, {1, 1, 0},
          {2, 0, 0},
          {0, 0, 1}, {0, 1, 0},
          {1, 0, 0},
          {0, 0, 0}};
    } else {
      subspaceLevels = {
          {1, 1, 5}, {1, 2, 4}, {1, 3, 3}, {1, 4, 2}, {1, 5, 1},
          {2, 1, 4}, {2, 2, 3}, {2, 3, 2}, {2, 4, 1},
          {3, 1, 3}, {3, 2, 2}, {3, 3, 1},
          {4, 1, 2}, {4, 2, 1},
          {5, 1, 1},
          {1, 1, 4}, {1, 2, 3}, {1, 3, 2}, {1, 4, 1},
          {2, 1, 3}, {2, 2, 2}, {2, 3, 1},
          {3, 1, 2}, {3, 2, 1},
          {4, 1, 1},
          {1, 1, 3}, {1, 2, 2}, {1, 3, 1},
          {2, 1, 2}, {2, 2, 1},
          {3, 1, 1},
          {1, 1, 2}, {1, 2, 1},
          {2, 1, 1},
          {1, 1, 1}};
    }

    std::vector<CombinationGrid> combinationGrids = {
        CombinationGrid::fromRegular(3, 5, basis, hasBoundary),
        CombinationGrid::fromSubspaces(subspaceLevels, basis, hasBoundary)};

    for (const CombinationGrid& combinationGrid : combinationGrids) {
      BOOST_CHECK_EQUAL(combinationGrid.getFullGrids().size(),
          (hasBoundary ? (21 + 15 + 10) : (15 + 10 + 6)));
      sgpp::base::GridStorage gridStorage(3);
      combinationGrid.combinePoints(gridStorage);
      BOOST_CHECK_EQUAL(gridStorage.getSize(), (hasBoundary ? 705 : 351));
    }

    const std::vector<FullGrid>& fullGrids0 = combinationGrids[0].getFullGrids();
    const std::vector<FullGrid>& fullGrids1 = combinationGrids[1].getFullGrids();
    const DataVector& coefficients0 = combinationGrids[0].getCoefficients();
    const DataVector& coefficients1 = combinationGrids[1].getCoefficients();

    for (size_t i = 0; i < fullGrids1.size(); i++) {
      const FullGrid& fullGrid = fullGrids1[i];
      auto it = std::find(fullGrids0.begin(), fullGrids0.end(), fullGrid);
      BOOST_CHECK(it != fullGrids0.end());
      BOOST_CHECK_EQUAL(coefficients0[it - fullGrids0.begin()], coefficients1[i]);
    }
  }
}

BOOST_AUTO_TEST_CASE(testOperationEvalCombinationGrid) {
  sgpp::base::SLinearBase basis1d;
  HeterogeneousBasis basis(2, basis1d, false);
  CombinationGrid combinationGrid = CombinationGrid::fromRegular(2, 2, basis, false);
  OperationEvalCombinationGrid op(combinationGrid);
  std::vector<DataVector> surpluses = {
      DataVector{1.0, -2.0, -1.0}, DataVector{-0.5, 0.5, 1.0}, DataVector{5.0}};
  DataVector point{0.625, 0.375};
  // 1 * (1.0*(0*0.75) + -2.0*(0.5*0.75) + -1.0*(0.5*0.75))
  // + 1 * (-0.5*(0.75*0.5) + 0.5*(0.75*0.5) + 1.0*(0.75*0))
  // + (-1) * 5.0*0.75*0.75
  // = (0 - 0.75 - 0.375) + (-0.1875 + 0.1875 + 0) - 2.8125 = -1.125 + 0 - 2.8125 = -3.9375
  BOOST_CHECK_EQUAL(op.eval(surpluses, point), -3.9375);
}

BOOST_AUTO_TEST_CASE(testOperationUPFullGridLinear) {
  sgpp::base::SLinearBase basis1d;
  HeterogeneousBasis basis(2, basis1d);
  FullGrid fullGrid({2, 1}, basis);
  OperationPoleHierarchisationLinear operationPole;
  OperationUPFullGrid operation(fullGrid, operationPole);
  DataVector values{-0.5, 3.0, 0.25, 0.5, -1.0, 1.0, 5.0, 2.5,
                    -1.5, 0.0, 2.0, -1.0, 1.0, -2.0, -1.0};
  operation.apply(values);
  DataVector correctSurpluses{-0.5, 3.125, 1.0, 0.875, -1.0, 0.25, 2.9375, 1.25,
                              -2.1875, 1.0, 2.0, -2.5, 0.5, -2.0, -1.0};

  for (size_t i = 0; i < values.size(); i++) {
    BOOST_CHECK_CLOSE(values[i], correctSurpluses[i], 1e-8);
  }
}

BOOST_AUTO_TEST_CASE(testOperationUPFullGridGeneral) {
  for (bool isBasisHierarchical : {true, false}) {
    sgpp::base::SBsplineBase basis1d(3);
    HeterogeneousBasis basis(2, basis1d, isBasisHierarchical);
    FullGrid fullGrid({2, 1}, basis);
    std::vector<std::unique_ptr<OperationPole>> operationPole;
    OperationPoleHierarchisationGeneral::fromHeterogenerousBasis(basis, operationPole);
    OperationUPFullGrid operation(fullGrid, operationPole);
    DataVector values{-0.5, 3.0, 0.25, 0.5, -1.0, 1.0, 5.0, 2.5,
                      -1.5, 0.0, 2.0, -1.0, 1.0, -2.0, -1.0};
    operation.apply(values);
    DataVector correctSurpluses;

    if (isBasisHierarchical) {
      correctSurpluses = DataVector{-4.34548191029053, 11.29388598889826, -0.45638469432632633,
          8.30837174457185, -3.402942074462426, -4.7579856595053664, 10.752772401474628,
          4.220642134584388, -8.117821514507945, 5.795370207225638, 8.991769884961288,
          -15.38082204309674, 3.036651517372788, -7.852574819533291, -3.5702774351738715};
    } else {
      correctSurpluses = DataVector{-2.860096153846154, 5.333241758241758, -3.044299450549451,
          3.4689560439560445, -3.4386675824175827, -1.101923076923078, 10.83626373626374,
          4.042582417582418, -4.506593406593407, 2.4123626373626372, 5.998557692307693,
          -7.601373626373627, 3.8355082417582422, -4.365659340659341, -1.4800137362637362};
    }

    for (size_t i = 0; i < values.size(); i++) {
      BOOST_CHECK_CLOSE(values[i], correctSurpluses[i], 1e-8);
    }
  }
}

BOOST_AUTO_TEST_CASE(testOperationUPCombinationGrid) {
  sgpp::base::SBsplineBase basis1d;
  HeterogeneousBasis basis(2, basis1d);
  CombinationGrid combinationGrid = CombinationGrid::fromRegular(2, 1, basis);
  OperationPoleNodalisationBspline operationPole(3);
  OperationUPCombinationGrid operation(combinationGrid, operationPole);
  std::vector<DataVector> values = {
      DataVector{-0.5, 3.0, 0.25, 0.5, -1.0, 1.0}, DataVector{5.0, 2.5, -1.5, 0.0, 2.0, -1.0},
      DataVector{1.0, -2.0, -1.0, 0.5}};
  operation.apply(values);
  BOOST_CHECK_CLOSE(values[0][0], -3.83571429, 1e-6);
  BOOST_CHECK_CLOSE(values[0][1], 9.34285714, 1e-6);
  BOOST_CHECK_CLOSE(values[0][2], -2.33571429, 1e-6);
  BOOST_CHECK_CLOSE(values[0][3], 2.96785714, 1e-6);
  BOOST_CHECK_CLOSE(values[0][4], -5.87142857, 1e-6);
  BOOST_CHECK_CLOSE(values[0][5], 3.71785714, 1e-6);
  BOOST_CHECK_CLOSE(values[1][0], 12.66428571, 1e-6);
  BOOST_CHECK_CLOSE(values[1][1], 2.7, 1e-6);
  BOOST_CHECK_CLOSE(values[1][2], -8.65714286, 1e-6);
  BOOST_CHECK_CLOSE(values[1][3], 1.2, 1e-6);
  BOOST_CHECK_CLOSE(values[1][4], 7.56428571, 1e-6);
  BOOST_CHECK_CLOSE(values[1][5], -3.9, 1e-6);
  BOOST_CHECK_CLOSE(values[2][0], 4.56, 1e-6);
  BOOST_CHECK_CLOSE(values[2][1], -6.24, 1e-6);
  BOOST_CHECK_CLOSE(values[2][2], -3.84, 1e-6);
  BOOST_CHECK_CLOSE(values[2][3], 3.36, 1e-6);
}
