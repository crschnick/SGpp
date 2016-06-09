// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <sgpp/base/grid/generation/BoundaryGridGenerator.hpp>
#include <sgpp/base/grid/generation/GeneralizedBoundaryGridGenerator.hpp>
#include <sgpp/base/grid/generation/GridGenerator.hpp>
#include <sgpp/base/grid/generation/L0BoundaryGridGenerator.hpp>
#include <sgpp/base/grid/generation/PeriodicGridGenerator.hpp>
#include <sgpp/base/grid/generation/PrewaveletGridGenerator.hpp>
#include <sgpp/base/grid/generation/SquareRootGridGenerator.hpp>
#include <sgpp/base/grid/generation/StandardGridGenerator.hpp>
#include <sgpp/base/grid/generation/StretchedBoundaryGridGenerator.hpp>

#include <sgpp/base/datatypes/DataVector.hpp>
#include <sgpp/base/grid/GridStorage.hpp>
#include <sgpp/base/grid/generation/functors/SurplusRefinementFunctor.hpp>
#include <sgpp/base/grid/generation/functors/SurplusCoarseningFunctor.hpp>

using sgpp::base::BoundaryGridGenerator;
using sgpp::base::DataVector;
using sgpp::base::GeneralizedBoundaryGridGenerator;
using sgpp::base::GridStorage;
using sgpp::base::L0BoundaryGridGenerator;
using sgpp::base::PeriodicGridGenerator;
using sgpp::base::SquareRootGridGenerator;
using sgpp::base::StandardGridGenerator;
using sgpp::base::StretchedBoundaryGridGenerator;
using sgpp::base::SurplusCoarseningFunctor;
using sgpp::base::SurplusRefinementFunctor;

BOOST_AUTO_TEST_CASE(testPeriodicGridGenerator) {
  GridStorage storage(2);
  PeriodicGridGenerator* gridgen = new PeriodicGridGenerator(storage);

  gridgen->regular(2);
  BOOST_CHECK_EQUAL(storage.getSize(), 12);

  storage.emptyStorage();

  gridgen->cliques(3, 1);
  BOOST_CHECK_EQUAL(storage.getSize(), 13);

  delete gridgen;
}

BOOST_AUTO_TEST_CASE(testSquareRootGridGenerator) {
  GridStorage storage(2);
  SquareRootGridGenerator* gridgen = new SquareRootGridGenerator(storage);

  gridgen->regular(2);
  BOOST_CHECK_EQUAL(storage.getSize(), 21);
  BOOST_CHECK_EQUAL(gridgen->getNumberOfRefinablePoints(), 0);
  BOOST_CHECK_EQUAL(gridgen->getNumberOfRemovablePoints(), 0);
  BOOST_CHECK_EQUAL(gridgen->getNumberOfRefinablePointsToMaxLevel(1), 0);

  delete gridgen;
}

BOOST_AUTO_TEST_CASE(testGeneralizedBoundaryGridGenerator) {
  GridStorage storage(2);
  GeneralizedBoundaryGridGenerator* gridgen = new GeneralizedBoundaryGridGenerator(storage);

  gridgen->regular(2);
  BOOST_CHECK_EQUAL(storage.getSize(), 21);
  BOOST_CHECK_EQUAL(gridgen->getNumberOfRefinablePoints(), 0);
  BOOST_CHECK_EQUAL(gridgen->getNumberOfRemovablePoints(), 0);
  // BOOST_CHECK_EQUAL(gridgen->getNumberOfRefinablePointsToMaxLevel(1), 8);

  storage.emptyStorage();

  gridgen->truncated(3, 2);
  BOOST_CHECK_EQUAL(storage.getSize(), 65);
  BOOST_CHECK_EQUAL(gridgen->getNumberOfRefinablePoints(), 0);
  BOOST_CHECK_EQUAL(gridgen->getNumberOfRemovablePoints(), 0);

  // gridgen->full(2);
  // BOOST_CHECK_EQUAL(storage.getSize(), 25);
  // BOOST_CHECK_EQUAL(gridgen->getNumberOfRefinablePoints(), 0);
  // BOOST_CHECK_EQUAL(gridgen->getNumberOfRemovablePoints(), 0);
  // BOOST_CHECK_EQUAL(gridgen->getNumberOfRefinablePointsToMaxLevel(1), 8);

  // Not implemented
  // storage.emptyStorage();

  // gridgen->cliques(3, 1);
  // BOOST_CHECK_EQUAL(storage.getSize(), 25);

  delete gridgen;
}

BOOST_AUTO_TEST_CASE(testL0BoundaryGridGenerator) {
  GridStorage storage(2);
  L0BoundaryGridGenerator* gridgen = new L0BoundaryGridGenerator(storage);

  gridgen->regular(2);
  BOOST_CHECK_EQUAL(storage.getSize(), 17);
  BOOST_CHECK_EQUAL(gridgen->getNumberOfRefinablePoints(), 9);
  BOOST_CHECK_EQUAL(gridgen->getNumberOfRemovablePoints(), 9);
  BOOST_CHECK_EQUAL(gridgen->getNumberOfRefinablePointsToMaxLevel(1), 8);

  storage.emptyStorage();

  gridgen->full(2);
  BOOST_CHECK_EQUAL(storage.getSize(), 25);
  BOOST_CHECK_EQUAL(gridgen->getNumberOfRefinablePoints(), 16);
  BOOST_CHECK_EQUAL(gridgen->getNumberOfRemovablePoints(), 4);
  BOOST_CHECK_EQUAL(gridgen->getNumberOfRefinablePointsToMaxLevel(1), 8);

  // Not implemented
  // storage.emptyStorage();

  // gridgen->cliques(3, 1);
  // BOOST_CHECK_EQUAL(storage.getSize(), 25);

  storage.emptyStorage();
  gridgen->regular(2);
  DataVector* alpha = new DataVector(storage.getSize(), 1);
  {
    SurplusRefinementFunctor rfunc(*alpha, 12);
    gridgen->refine(rfunc);
  }
  BOOST_CHECK_EQUAL(storage.getSize(), 37);

  storage.emptyStorage();
  delete alpha;
  gridgen->full(2);
  alpha = new DataVector(storage.getSize(), 1);
  {
    SurplusRefinementFunctor rfunc(*alpha, 25);
    gridgen->refineMaxLevel(rfunc, 3);
  }
  BOOST_CHECK_EQUAL(storage.getSize(), 65);

  /* Coarsening does not remove boundary nodes
  alpha->resizeZero(49);
  SurplusCoarseningFunctor* cfunc = new SurplusCoarseningFunctor(alpha, 37, 0.5);
  std::cout << gridgen->getNumberOfRemovablePoints() << std::endl;
  gridgen->coarsen(cfunc, alpha);
  std::cout << gridgen->getNumberOfRemovablePoints() << std::endl;
  gridgen->coarsen(cfunc, alpha);
  BOOST_CHECK_EQUAL(storage.getSize(), 12);
  std::cout << storage.toString() << std::endl << alpha->toString() << std::endl;
  */

  delete gridgen;
  // delete cfunc;
  delete alpha;
}

BOOST_AUTO_TEST_CASE(testBoundaryGridGenerator) {
  GridStorage storage(2);
  BoundaryGridGenerator* gridgen = new BoundaryGridGenerator(storage);

  gridgen->regular(2);
  BOOST_CHECK_EQUAL(storage.getSize(), 21);
  BOOST_CHECK_EQUAL(gridgen->getNumberOfRefinablePoints(), 12);
  BOOST_CHECK_EQUAL(gridgen->getNumberOfRemovablePoints(), 4);
  BOOST_CHECK_EQUAL(gridgen->getNumberOfRefinablePointsToMaxLevel(1), 8);

  storage.emptyStorage();

  gridgen->full(2);
  BOOST_CHECK_EQUAL(storage.getSize(), 25);
  BOOST_CHECK_EQUAL(gridgen->getNumberOfRefinablePoints(), 16);
  BOOST_CHECK_EQUAL(gridgen->getNumberOfRemovablePoints(), 4);
  BOOST_CHECK_EQUAL(gridgen->getNumberOfRefinablePointsToMaxLevel(1), 8);

  // Not implemented
  // storage.emptyStorage();

  // gridgen->cliques(3, 1);
  // BOOST_CHECK_EQUAL(storage.getSize(), 25);

  storage.emptyStorage();
  gridgen->regular(2);
  DataVector* alpha = new DataVector(storage.getSize(), 1);
  {
    SurplusRefinementFunctor rfunc(*alpha, 12);
    gridgen->refine(rfunc);
  }
  BOOST_CHECK_EQUAL(storage.getSize(), 49);

  storage.emptyStorage();
  delete alpha;
  gridgen->full(2);
  alpha = new DataVector(storage.getSize(), 1);
  {
    SurplusRefinementFunctor rfunc(*alpha, 25);
    gridgen->refineMaxLevel(rfunc, 3);
  }
  BOOST_CHECK_EQUAL(storage.getSize(), 65);

  /* Coarsening does not remove boundary nodes
  alpha->resizeZero(49);
  SurplusCoarseningFunctor* cfunc = new SurplusCoarseningFunctor(alpha, 37, 0.5);
  std::cout << gridgen->getNumberOfRemovablePoints() << std::endl;
  gridgen->coarsen(cfunc, alpha);
  std::cout << gridgen->getNumberOfRemovablePoints() << std::endl;
  gridgen->coarsen(cfunc, alpha);
  BOOST_CHECK_EQUAL(storage.getSize(), 12);
  std::cout << storage.toString() << std::endl << alpha->toString() << std::endl;
  */

  delete gridgen;
  // delete cfunc;
  delete alpha;
}

BOOST_AUTO_TEST_CASE(testStretchedBoundaryGridGenerator) {
  GridStorage storage(2);
  StretchedBoundaryGridGenerator* gridgen = new StretchedBoundaryGridGenerator(storage);

  gridgen->regular(2);
  BOOST_CHECK_EQUAL(storage.getSize(), 21);
  BOOST_CHECK_EQUAL(gridgen->getNumberOfRefinablePoints(), 12);
  BOOST_CHECK_EQUAL(gridgen->getNumberOfRemovablePoints(), 4);
  BOOST_CHECK_EQUAL(gridgen->getNumberOfRefinablePointsToMaxLevel(1), 8);

  storage.emptyStorage();

  gridgen->full(2);
  BOOST_CHECK_EQUAL(storage.getSize(), 25);
  BOOST_CHECK_EQUAL(gridgen->getNumberOfRefinablePoints(), 16);
  BOOST_CHECK_EQUAL(gridgen->getNumberOfRemovablePoints(), 4);
  BOOST_CHECK_EQUAL(gridgen->getNumberOfRefinablePointsToMaxLevel(1), 8);

  // Not implemented
  // storage.emptyStorage();

  // gridgen->cliques(3, 1);
  // BOOST_CHECK_EQUAL(storage.getSize(), 25);

  storage.emptyStorage();
  gridgen->regular(2);
  DataVector* alpha = new DataVector(storage.getSize(), 1);
  SurplusRefinementFunctor rfunc(*alpha, 12);
  gridgen->refine(rfunc);
  BOOST_CHECK_EQUAL(storage.getSize(), 49);

  /*storage.emptyStorage();
  delete rfunc;
  delete alpha;
  gridgen->full(2);
  alpha = new DataVector(storage.getSize(), 1);
  rfunc = new SurplusRefinementFunctor(alpha, 25);
  gridgen->refineMaxLevel(rfunc, 3);
  BOOST_CHECK_EQUAL(storage.getSize(), 65);
  */
  /* Coarsening does not remove boundary nodes
  alpha->resizeZero(49);
  SurplusCoarseningFunctor* cfunc = new SurplusCoarseningFunctor(alpha, 37, 0.5);
  std::cout << gridgen->getNumberOfRemovablePoints() << std::endl;
  gridgen->coarsen(cfunc, alpha);
  std::cout << gridgen->getNumberOfRemovablePoints() << std::endl;
  gridgen->coarsen(cfunc, alpha);
  BOOST_CHECK_EQUAL(storage.getSize(), 12);
  std::cout << storage.toString() << std::endl << alpha->toString() << std::endl;
  */

  delete gridgen;
  // delete cfunc;
  delete alpha;
}

BOOST_AUTO_TEST_CASE(testStandardGridGenerator) {
  GridStorage storage(2);
  StandardGridGenerator* gridgen = new StandardGridGenerator(storage);

  gridgen->regular(2);
  BOOST_CHECK_EQUAL(storage.getSize(), 5);
  BOOST_CHECK_EQUAL(gridgen->getNumberOfRefinablePoints(), 4);
  BOOST_CHECK_EQUAL(gridgen->getNumberOfRemovablePoints(), 4);

  storage.emptyStorage();

  gridgen->full(2);
  BOOST_CHECK_EQUAL(storage.getSize(), 9);
  BOOST_CHECK_EQUAL(gridgen->getNumberOfRefinablePoints(), 8);
  BOOST_CHECK_EQUAL(gridgen->getNumberOfRemovablePoints(), 4);

  storage.emptyStorage();

  gridgen->cliques(3, 1);
  BOOST_CHECK_EQUAL(storage.getSize(), 13);

  storage.emptyStorage();
  gridgen->regular(2);
  DataVector* alpha = new DataVector(storage.getSize(), 1);
  SurplusRefinementFunctor rfunc(*alpha, 4);
  gridgen->refine(rfunc);
  BOOST_CHECK_EQUAL(storage.getSize(), 17);

  alpha->resizeZero(17);
  SurplusCoarseningFunctor cfunc(*alpha, 12, 0.5);
  gridgen->coarsen(cfunc, *alpha);
  BOOST_CHECK_EQUAL(storage.getSize(), 5);

  delete gridgen;
  delete alpha;
}