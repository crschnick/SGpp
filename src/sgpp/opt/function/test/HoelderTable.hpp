/* ****************************************************************************
* Copyright (C) 2014 Technische Universitaet Muenchen                         *
* This file is part of the SG++ project. For conditions of distribution and   *
* use, please see the copyright notice at http://www5.in.tum.de/SGpp          *
**************************************************************************** */
// @author Julian Valentin (julian.valentin@stud.mathematik.uni-stuttgart.de)

#ifndef SGPP_OPT_FUNCTION_TEST_HOELDERTABLE_HPP
#define SGPP_OPT_FUNCTION_TEST_HOELDERTABLE_HPP

#include "opt/function/test/Test.hpp"

#include <cmath>

namespace sg {
  namespace opt {
    namespace function {
      namespace test {

        /**
         * Hoelder table test function.
         *
         * Definition:
         * \f$f(\vec{x}) := -\left|\sin x_1 \cos x_2
         *         \exp\!\left(\left|1 - \frac{\lVert \vec{x} \rVert_2}{\pi}\right|\right)\right|\f$,
         * \f$\vec{x} \in [-10, 10]^2\f$,
         * \f$\vec{x}_{\text{opt}} \in \{(8.0550, \pm 9.6646)^{\mathrm{T}},
         *                               (-8.0550, \pm 9.6646)^{\mathrm{T}}\}\f$,
         * \f$f_{\text{opt}} = -19.2085\f$
         * (domain scaled to \f$[0, 1]^2\f$)
         *
         * The displacement is restricted because the minimal points lie near the corners
         * of \f$[0, 1]^2\f$.
         */
        class HoelderTable : public Test {
          public:
            /**
             * Constructor.
             */
            HoelderTable() : Test(2) {
            }

            /**
             * Generate normally distributed pseudorandom displacement
             * with the restriction of \f$\vec{d} \in [-0.005, 0.005] \times [-0.01, 0.01]\f$.
             *
             * @param std_dev   standard deviation of the displacement coordinates
             */
            void generateDisplacement(double std_dev) {
              do {
                Test::generateDisplacement(std_dev);
              } while ((displacement[0] > 0.005) || (displacement[0] < -0.005) ||
                       (displacement[1] > 0.01) || (displacement[1] < -0.01));
            }

            /**
             * Evaluates the test function.
             *
             * @param x     point \f$\vec{x} \in [0, 1]^2\f$
             * @return      \f$f(\vec{x})\f$
             */
            double evalUndisplaced(const std::vector<double>& x) {
              const double x1 = 20.0 * x[0] - 10.0;
              const double x2 = 20.0 * x[1] - 10.0;

              return -std::abs(std::sin(x1) * std::cos(x2) *
                               std::exp(std::abs(1.0 - std::sqrt(x1*x1 + x2*x2) / M_PI)));
            }

            /**
             * Returns minimal point and function value of the test function.
             *
             * @param[out] x    minimal point \f$\vec{x}_{\text{opt}} \in [0, 1]^2\f$
             * @return          minimal function value \f$f_{\text{opt}} = f(\vec{x}_{\text{opt}})\f$
             */
            double getOptimalPointUndisplaced(std::vector<double>& x) {
              x.clear();
              x.push_back(0.902751);
              x.push_back(0.9832295);
              return evalUndisplaced(x);
            }

            /**
             * @return clone of the object
             */
            virtual tools::SmartPointer<Objective> clone() {
              return tools::SmartPointer<Objective>(new HoelderTable(*this));
            }
        };

      }
    }
  }
}

#endif
