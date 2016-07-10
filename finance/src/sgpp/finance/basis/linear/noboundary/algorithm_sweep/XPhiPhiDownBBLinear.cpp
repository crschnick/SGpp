// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#include <sgpp/finance/basis/linear/noboundary/algorithm_sweep/XPhiPhiDownBBLinear.hpp>

#include <sgpp/globaldef.hpp>

namespace sgpp {
namespace finance {

XPhiPhiDownBBLinear::XPhiPhiDownBBLinear(sgpp::base::GridStorage* storage)
    : storage(storage), boundingBox(storage->getBoundingBox()) {}

XPhiPhiDownBBLinear::~XPhiPhiDownBBLinear() {}

void XPhiPhiDownBBLinear::operator()(sgpp::base::DataVector& source, sgpp::base::DataVector& result,
                                     grid_iterator& index, size_t dim) {
  double q = this->boundingBox->getIntervalWidth(dim);
  double t = this->boundingBox->getIntervalOffset(dim);

  bool useBB = false;

  if (q != 1.0 || t != 0.0) {
    useBB = true;
  }

  if (useBB) {
    recBB(source, result, index, dim, 0.0, 0.0, q, t);
  } else {
    rec(source, result, index, dim, 0.0, 0.0);
  }
}

void XPhiPhiDownBBLinear::rec(sgpp::base::DataVector& source, sgpp::base::DataVector& result,
                              grid_iterator& index, size_t dim, double fl, double fr) {
  size_t seq = index.seq();

  double alpha_value = source[seq];

  sgpp::base::level_t l;
  sgpp::base::index_t i;

  index.get(dim, l, i);

  double i_dbl = static_cast<double>(i);
  int l_int = static_cast<int>(l);

  double hsquare = (1.0 / static_cast<double>(1 << (2 * l_int)));

  // integration
  result[seq] = (hsquare * ((fl + fr) / 2.0)) * i_dbl + hsquare * (fr - fl) / 12.0 +
                (((2.0 / 3.0) * hsquare * i_dbl) * alpha_value);

  // dehierarchisation
  double fm = ((fl + fr) / 2.0) + alpha_value;

  if (!index.hint()) {
    index.leftChild(dim);

    if (!storage->isValidSequenceNumber(index.seq())) {
      rec(source, result, index, dim, fl, fm);
    }

    index.stepRight(dim);

    if (!storage->isValidSequenceNumber(index.seq())) {
      rec(source, result, index, dim, fm, fr);
    }

    index.up(dim);
  }
}

void XPhiPhiDownBBLinear::recBB(sgpp::base::DataVector& source, sgpp::base::DataVector& result,
                                grid_iterator& index, size_t dim, double fl, double fr, double q,
                                double t) {
  size_t seq = index.seq();

  double alpha_value = source[seq];

  sgpp::base::level_t l;
  sgpp::base::index_t i;

  index.get(dim, l, i);

  double i_dbl = static_cast<double>(i);
  int l_int = static_cast<int>(l);

  double h = (1.0 / (static_cast<double>(1 << (l_int))));

  // integration
  result[seq] = (h * h * i_dbl * q * q + h * q * t) * ((fl + fr) / 2.0) +
                h * h * q * q * (fr - fl) / 12.0 +
                (((2.0 / 3.0) * h * q * t + (2.0 / 3.0) * i_dbl * h * h * q * q) *
                 alpha_value);  // diagonal entry

  // dehierarchisation
  double fm = ((fl + fr) / 2.0) + alpha_value;

  if (!index.hint()) {
    index.leftChild(dim);

    if (!storage->isValidSequenceNumber(index.seq())) {
      recBB(source, result, index, dim, fl, fm, q, t);
    }

    index.stepRight(dim);

    if (!storage->isValidSequenceNumber(index.seq())) {
      recBB(source, result, index, dim, fm, fr, q, t);
    }

    index.up(dim);
  }
}

}  // namespace finance
}  // namespace sgpp
