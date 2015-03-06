// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#ifndef POLY_BASE_HPP
#define POLY_BASE_HPP

#include <cmath>
#include <vector>
#include <sgpp/base/exception/factory_exception.hpp>
#include <sgpp/base/datatypes/DataVector.hpp>
#include <sgpp/base/operation/hash/common/basis/Basis.hpp>

#include <sgpp/globaldef.hpp>


namespace SGPP {

  namespace base {

    /**
     * Polynomial basis functions.
     *
     * @version $HEAD$
     */
    template<class LT, class IT>
    class PolyBasis: public Basis<LT, IT> {
      public:
        /**
         * Constructor
         *
         * @param degree the polynom's max. degree
         */
        PolyBasis(size_t degree) : polynoms(NULL), degree(degree) {
          if (degree < 2) {
            throw factory_exception("PolyBasis: degree < 2");
          }

          int polycount = (1 << (degree - 1)) - 1;
          std::vector<float_t> x;
          x.push_back(0.0);
          x.push_back(1.0);

          polynoms = new float_t[(degree + 3) * polycount];
          initPolynoms(x, 1, 1);
        }

        /**
         * Destructor
         */
        ~PolyBasis() {
          if (polynoms) {
            delete [] polynoms;
          }
        }

        /**
         * Evaluate a basis function.
         * Has a dependence on the absolute position of grid point and support.
         */
        float_t eval(LT level, IT index, float_t p) {
          size_t deg = degree - 1 < level ? degree - 1 : level;

          size_t idMask = (1 << deg) - 1;
          size_t id = (((index & idMask) >> 1) | (1 << (deg - 1))) - 1;

          // scale p to a value in [-1.0,1.0]
          float_t val = (1 << level) * p - index;
          return evalPolynom(id, deg, val);
        }

        float_t evalHierToTop(LT level, IT index, DataVector& koeffs, float_t pos) {
          float_t result = 0.0;

          for (; level >= 1; level--) {
            result += koeffs[level] * eval(level, index, pos);
            index = ((index - 1) / 2);
            index = (index % 2 == 0) ? (index + 1) : index;
          }

          return result;
        }

        size_t getDegree() {
          return degree;
        }


      protected:
        /// Pointer to polynoms
        float_t* polynoms;
        /// the polynom's max degree
        size_t degree;

      private:
        /**
         * Evaluate a basis function.
         * Has a dependence on the absolute position of grid point and support.
         */
        float_t evalPolynom(size_t id, size_t deg, float_t val) {
          float_t* x_store = this->polynoms + (degree + 3) * id;
          float_t* y_store = x_store + 2;

          float_t y_val = y_store[deg + 1];
          // scale val back into the right range
          float_t x_val = x_store[0] + val * pow(2.0, -(1.0) * (static_cast<float_t>(deg)));

          //Horner
          for (size_t i = deg; i > 0; i--) {
            y_val = y_val * x_val + y_store[i];
          }

          return y_val * x_val + y_store[0];
        }

        /**
         * recursively creates polynomial values
         */
        void initPolynoms(std::vector<float_t>& x, LT level, IT index) {
          // Add new point
          x.push_back(index * pow(2.0, -(1.0) * (static_cast<float_t>(level))));

          std::vector<float_t> y;
          std::vector<float_t> intpoly;

          for (size_t i = 0; i < level + 1; i++) {
            y.push_back(0.0);
            intpoly.push_back(0.0);
          }

          y.push_back(1.0);
          intpoly.push_back(0.0);

          // Every poly has a unique id similiar to sparse grid level/index pairs
          size_t id = ((index >> 1) | (1 << (level - 1))) - 1;

          int n = level + 2;
          std::vector<std::vector<float_t> > lagpoly;

          /**
           * Fill lagpoly with multiplied lagrange polynomials
           * Add lagrange polynomials together into intpoly
           */
          for (int i = 0; i < n; i++) {
            lagpoly.push_back(std::vector<float_t>());
            lagpoly[i].push_back(1.0);
            float_t fac = y[i];

            int j = 0;

            for (int k = 0; k < n; k++) {
              if (k == i) {
                continue;
              }

              lagpoly[i].push_back(lagpoly[i][j]);

              for (int jj = j; jj > 0; jj--) {
                lagpoly[i][jj] = lagpoly[i][jj - 1] - lagpoly[i][jj] * x[k];
              }

              lagpoly[i][0] *= -x[k];
              j += 1;
              fac /= (x[i] - x[k]);
            }

            for (int l = 0; l < n; l++) {
              lagpoly[i][l] *= fac;
              intpoly[l] += lagpoly[i][l];
            }
          }

          //determine position in storage. (degree + 1) polynomial factors and 2 values for support and x-value
          float_t* x_store = this->polynoms + (degree + 3) * id;
          float_t* y_store = x_store + 2;

          // Copy values into storage
          for (size_t i = 0; i < level + 2; i++) {
            y_store[i] = intpoly[i];
          }

          x_store[0] = x[level + 1];

          // Integrate polynom
          /*
          {
            //get the antiderivative of the polynomial
            for(int i = 0; i < level + 2; i++)
            {
              intpoly[i] /= i + 1;
            }

            //evaluate bounds
            float_t support = pow(2.0, -level);
            float_t lb = x[level+1] - support;
            float_t rb = x[level+1] + support;

            float_t val = intpoly[level + 1];
            for(int i = level; i >= 0; i--)
            {
              val = val * rb + intpoly[i];
            }
            val *= rb;

            float_t val2 = intpoly[level + 1];
            for(int i = level; i >= 0; i--)
            {
              val2 = val2 * lb + intpoly[i];
            }
            val2 *= lb;

            val -= val2;
            val /= 2*support;

            x_store[1] = val;
          }
          */

          if ((level + 1) < degree) {
            initPolynoms(x, level + 1, index * 2 - 1);
            initPolynoms(x, level + 1, index * 2 + 1);
          }

          x.pop_back();
        }

    };

    // default type-def (unsigned int for level and index)
    typedef PolyBasis<unsigned int, unsigned int> SPolyBase;

  }
}

#endif /* POLY_BASE_HPP */