// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#pragma once

#include <sgpp/base/datatypes/DataVector.hpp>

#include <chrono>
#include <iostream>
#include <random>
#include <string>

namespace sgpp {
namespace base {

/**
 * enum to address different pdfs in a standardized way
 *
 */
// enum class DistributionType {
//  Uniform,    //  0
//  Normal,     //  1
//  Lognormal,  //  2
//};

/**
 * stores a sparse grid not a knot B-spline interpolant in the framework of a respsonse surface
 */
class Distribution {
 public:
  /**
   * Constructor
   */
  Distribution() {
    // set seed
    gen.seed(std::chrono::system_clock::now().time_since_epoch().count());
  }

  /**
   * Destructor
   */
  virtual ~Distribution() {}

  /**
   *
   */
  virtual double sample() = 0;

  /**
   *
   */
  virtual double eval(double x) = 0;

  sgpp::base::DataVector samples(size_t num);

  virtual sgpp::base::DataVector getBounds() = 0;

  virtual std::string getType() = 0;

 protected:
  std::default_random_engine gen;
};

}  // namespace base
}  // namespace sgpp
