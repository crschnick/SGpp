/* ****************************************************************************
* Copyright (C) 2014 Technische Universitaet Muenchen                         *
* This file is part of the SG++ project. For conditions of distribution and   *
* use, please see the copyright notice at http://www5.in.tum.de/SGpp          *
**************************************************************************** */
// @author Julian Valentin (julian.valentin@stud.mathematik.uni-stuttgart.de)

#ifndef SGPP_BASE_BASIS_NOBOUNDARY_OPERATION_OPERATIONNAIVEEVALBSPLINE_HPP
#define SGPP_BASE_BASIS_NOBOUNDARY_OPERATION_OPERATIONNAIVEEVALBSPLINE_HPP

#include "base/operation/OperationNaiveEval.hpp"
#include "base/grid/GridStorage.hpp"
#include "base/basis/bspline/noboundary/BsplineBasis.hpp"
#include "base/datatypes/DataVector.hpp"

namespace sg {
  namespace base {

    /**
     * Operation for evaluating B-spline linear combinations on Noboundary grids.
     */
    class OperationNaiveEvalBspline : public OperationNaiveEval {
      public:
        /**
         * Constructor.
         *
         * @param storage   storage of the sparse grid
         * @param degree    B-spline degree
         */
        OperationNaiveEvalBspline(GridStorage* storage, size_t degree) :
          storage(storage), base(degree) {
        }

        /**
         * Virtual destructor.
         */
        virtual ~OperationNaiveEvalBspline() {
        }

        /**
         * @param alpha     coefficient vector
         * @param point     evaluation point
         * @return          value of linear combination
         */
        virtual double eval(DataVector& alpha, std::vector<double>& point);

      protected:
        /// storage of the sparse grid
        GridStorage* storage;
        /// 1D B-spline basis
        SBsplineBase base;
    };

  }
}

#endif
