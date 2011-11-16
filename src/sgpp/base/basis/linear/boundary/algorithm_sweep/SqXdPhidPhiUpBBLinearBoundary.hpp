/******************************************************************************
* Copyright (C) 2009 Technische Universitaet Muenchen                         *
* This file is part of the SG++ project. For conditions of distribution and   *
* use, please see the copyright notice at http://www5.in.tum.de/SGpp          *
******************************************************************************/
// @author Alexander Heinecke (Alexander.Heinecke@mytum.de)

#ifndef SQXDPHIDPHIUPBBLINEARBOUNDARY_HPP
#define SQXDPHIDPHIUPBBLINEARBOUNDARY_HPP

#include "base/grid/GridStorage.hpp"
#include "base/datatypes/DataVector.hpp"

#include "base/basis/linear/noboundary/algorithm_sweep/SqXdPhidPhiUpBBLinear.hpp"

namespace sg
{
namespace finance
{



/**
 * Implementation of sweep operator (): 1D Up for
 * Bilinearform \f$\int_{x} x^{2} \frac{\partial \phi(x)}{x} \frac{\partial \phi(x)}{x} dx\f$
 * on linear boundary grids
 */
class SqXdPhidPhiUpBBLinearBoundary : public SqXdPhidPhiUpBBLinear
{
public:
	/**
	 * Constructor
	 *
	 * @param storage the grid's sg::base::GridStorage object
	 */
	SqXdPhidPhiUpBBLinearBoundary(sg::base::GridStorage* storage);

	/**
	 * Destructor
	 */
	virtual ~SqXdPhidPhiUpBBLinearBoundary();

	/**
	 * This operations performs the calculation of up in the direction of dimension <i>dim</i>
	 *
	 * For level zero it's assumed, that both ansatz-functions do exist: 0,0 and 0,1
	 * If one is missing this code might produce some bad errors (segmentation fault, wrong calculation
	 * result)
	 * So please assure that both functions do exist!
	 *
	 * @param source sg::base::DataVector that contains the gridpoint's coefficients (values from the vector of the laplace operation)
	 * @param result sg::base::DataVector that contains the result of the up operation
	 * @param index a iterator object of the grid
	 * @param dim current fixed dimension of the 'execution direction'
	 */
	virtual void operator()(sg::base::DataVector& source, sg::base::DataVector& result, grid_iterator& index, size_t dim);
};

 // namespace detail

} // namespace sg
}

#endif /* SQXDPHIDPHIUPBBLINEARBOUNDARY_HPP */