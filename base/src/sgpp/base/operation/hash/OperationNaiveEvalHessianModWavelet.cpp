// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#include <sgpp/globaldef.hpp>
#include <sgpp/base/operation/hash/OperationNaiveEvalHessianModWavelet.hpp>

namespace SGPP {
  namespace base {

    float_t OperationNaiveEvalHessianModWavelet::evalHessian(
      DataVector& alpha, const std::vector<float_t>& point,
      DataVector& gradient, DataMatrix& hessian) {
      const size_t n = storage->size();
      const size_t d = storage->dim();
      float_t result = 0.0;

      gradient.resize(storage->dim());
      gradient.setAll(0.0);

      hessian = DataMatrix(d, d);
      hessian.setAll(0.0);

      DataVector cur_gradient(d);
      DataMatrix cur_hessian(d, d);

      for (size_t i = 0; i < n; i++) {
        const GridIndex* gp = storage->get(i);
        float_t cur_val = 1.0;
        cur_gradient.setAll(alpha[i]);
        cur_hessian.setAll(alpha[i]);

        for (size_t t = 0; t < d; t++) {
          float_t val1d = base.eval(gp->getLevel(t), gp->getIndex(t), point[t]);
          float_t dx1d = base.evalDx(gp->getLevel(t), gp->getIndex(t), point[t]);
          float_t dxdx1d = base.evalDxDx(gp->getLevel(t), gp->getIndex(t), point[t]);

          cur_val *= val1d;

          for (size_t t2 = 0; t2 < d; t2++) {
            if (t2 == t) {
              cur_gradient[t2] *= dx1d;

              for (size_t t3 = 0; t3 < d; t3++) {
                if (t3 == t) {
                  cur_hessian[t2 * d + t3] *= dxdx1d;
                } else {
                  cur_hessian[t2 * d + t3] *= dx1d;
                }
              }
            } else {
              cur_gradient[t2] *= val1d;

              for (size_t t3 = 0; t3 < d; t3++) {
                if (t3 == t) {
                  cur_hessian[t2 * d + t3] *= dx1d;
                } else {
                  cur_hessian[t2 * d + t3] *= val1d;
                }
              }
            }
          }
        }

        result += alpha[i] * cur_val;
        gradient.add(cur_gradient);
        hessian.add(cur_hessian);
      }

      return result;
    }

  }
}