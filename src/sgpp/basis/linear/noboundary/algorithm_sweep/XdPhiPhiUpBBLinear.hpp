/******************************************************************************
* Copyright (C) 2009 Technische Universitaet Muenchen                         *
* This file is part of the SG++ project. For conditions of distribution and   *
* use, please see the copyright notice at http://www5.in.tum.de/SGpp          *
******************************************************************************/
// @author Alexander Heinecke (Alexander.Heinecke@mytum.de)

#ifndef XDPHIPHIUPBBLINEAR_HPP
#define XDPHIPHIUPBBLINEAR_HPP

#include "grid/GridStorage.hpp"
#include "data/DataVector.hpp"
using namespace sg::base;

namespace sg
{
namespace finance
{



/**
 * Implementation of sweep operator (): 1D Up for
 * Bilinearform \f$\int_{x} x \frac{\partial \phi(x)}{x} \phi(x) dx\f$
 */
class XdPhiPhiUpBBLinear
{
protected:
	typedef GridStorage::grid_iterator grid_iterator;

	/// Pointer to GridStorage object
	GridStorage* storage;
	/// Pointer to the bounding box Obejct
	BoundingBox* boundingBox;

public:
	/**
	 * Constructor
	 *
	 * @param storage the grid's GridStorage object
	 */
	XdPhiPhiUpBBLinear(GridStorage* storage);

	/**
	 * Destructor
	 */
	~XdPhiPhiUpBBLinear();

	/**
	 * This operations performs the calculation of up in the direction of dimension <i>dim</i>
	 * on a grid with fix Dirichlet 0 boundary conditions
	 *
	 * @param source DataVector that contains the gridpoint's coefficients (values from the vector of the laplace operation)
	 * @param result DataVector that contains the result of the up operation
	 * @param index a iterator object of the grid
	 * @param dim current fixed dimension of the 'execution direction'
	 */
	void operator()(DataVector& source, DataVector& result, grid_iterator& index, size_t dim);

protected:

	/**
	 * recursive function for the calculation of Up
	 *
	 * @param source DataVector that contains the coefficients of the ansatzfunction
	 * @param result DataVector in which the result of the operation is stored
	 * @param index reference to a griditerator object that is used navigate through the grid
	 * @param dim the dimension in which the operation is executed
	 * @param fl function value on the left boundary, reference parameter
	 * @param fr function value on the right boundary, reference parameter
	 */
	void rec(DataVector& source, DataVector& result, grid_iterator& index, size_t dim, double& fl, double& fr);

	/**
	 * recursive function for the calculation of Up with Bounding Box support
	 *
	 * @param source DataVector that contains the coefficients of the ansatzfunction
	 * @param result DataVector in which the result of the operation is stored
	 * @param index reference to a griditerator object that is used navigate through the grid
	 * @param dim the dimension in which the operation is executed
	 * @param fl function value on the left boundary, reference parameter
	 * @param fr function value on the right boundary, reference parameter
	 */
	void recBB(DataVector& source, DataVector& result, grid_iterator& index, size_t dim, double& fl, double& fr, double q, double t);
};

 // namespace detail

} // namespace sg
}

#endif /* XDPHIPHIUPBBLINEARBOUNDARY_HPP */
