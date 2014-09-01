/* ****************************************************************************
* Copyright (C) 2014 Technische Universitaet Muenchen                         *
* This file is part of the SG++ project. For conditions of distribution and   *
* use, please see the copyright notice at http://www5.in.tum.de/SGpp          *
**************************************************************************** */
// @author Julian Valentin (julian.valentin@stud.mathematik.uni-stuttgart.de)

#ifndef SGPP_BASE_BASIS_BSPLINE_MODIFIED_OPERATION_OPERATIONEVALHESSIANMODBSPLINE_HPP
#define SGPP_BASE_BASIS_BSPLINE_MODIFIED_OPERATION_OPERATIONEVALHESSIANMODBSPLINE_HPP

#include "base/operation/OperationNaiveEvalHessian.hpp"
#include "base/grid/GridStorage.hpp"
#include "base/basis/bspline/modified/ModBsplineBasis.hpp"
#include "base/datatypes/DataVector.hpp"
#include "base/datatypes/DataMatrix.hpp"

namespace sg {
  namespace base {

    /**
     * Operation for evaluating modified B-spline linear combinations on Noboundary grids,
     * their gradients and their Hessians.
     */
    class OperationNaiveEvalHessianModBspline : public OperationNaiveEvalHessian {
      public:
        /**
         * Constructor.
         *
         * @param storage   storage of the sparse grid
         * @param degree    B-spline degree
         */
        OperationNaiveEvalHessianModBspline(GridStorage* storage, size_t degree) :
          storage(storage), base(degree) {
        }

        /**
         * Virtual destructor.
         */
        virtual ~OperationNaiveEvalHessianModBspline() {
        }

        /**
         * @param       alpha       coefficient vector
         * @param       point       evaluation point
         * @param[out]  gradient    gradient vector of linear combination
         * @param[out]  hessian     Hessian matrix of linear combination
         * @return                  value of linear combination
         */
        virtual double evalHessian(DataVector& alpha, const std::vector<double>& point,
                                   DataVector& gradient, DataMatrix& hessian);

      protected:
        /// storage of the sparse grid
        GridStorage* storage;
        /// 1D B-spline basis
        SModBsplineBase base;
    };

  }
}

#endif
