/******************************************************************************
* Copyright (C) 2009 Technische Universitaet Muenchen                         *
* This file is part of the SG++ project. For conditions of distribution and   *
* use, please see the copyright notice at http://www5.in.tum.de/SGpp          *
******************************************************************************/
// @author Alexander Heinecke (Alexander.Heinecke@mytum.de)

#include "algorithm/pde/BlackScholesParabolicPDESolverSystemEuropean.hpp"
#include "exception/algorithm_exception.hpp"
#include "grid/generation/SurplusCoarseningFunctor.hpp"
#include "grid/generation/SurplusRefinementFunctor.hpp"
#include <cmath>
using namespace sg::base;

namespace sg
{
namespace finance
{

BlackScholesParabolicPDESolverSystemEuropean::BlackScholesParabolicPDESolverSystemEuropean(Grid& SparseGrid, DataVector& alpha, DataVector& mu,
			DataVector& sigma, DataMatrix& rho, double r, double TimestepSize, std::string OperationMode,
			bool bLogTransform, bool useCoarsen, double coarsenThreshold, std::string adaptSolveMode,
			int numCoarsenPoints, double refineThreshold, std::string refineMode, size_t refineMaxLevel)
{
	this->BoundGrid = &SparseGrid;
	this->alpha_complete = &alpha;

	this->alpha_complete_old = new DataVector(*this->alpha_complete);
	this->alpha_complete_tmp = new DataVector(*this->alpha_complete);

	this->InnerGrid = NULL;
	this->alpha_inner = NULL;
	this->tOperationMode = OperationMode;
	this->TimestepSize = TimestepSize;
	this->TimestepSize_old = TimestepSize;
	this->BoundaryUpdate = new DirichletUpdateVector(SparseGrid.getStorage());
	this->GridConverter = new DirichletGridConverter();
	this->r = r;
	this->mus = &mu;
	this->sigmas = &sigma;
	this->rhos = &rho;
	this->BSalgoDims = this->BoundGrid->getAlgorithmicDimensions();

	// throw exception if grid dimensions not equal algorithmic dimensions
	if (this->BSalgoDims.size() != this->BoundGrid->getStorage()->dim())
	{
		throw algorithm_exception("BlackScholesParabolicPDESolverSystemEuropean::BlackScholesParabolicPDESolverSystemEuropean : Number of algorithmic dimensions is not equal to the number of grid's dimensions.");
	}

	// test if number of dimensions in the coefficients match the numbers of grid dimensions (mu and sigma)
	if (this->BoundGrid->getStorage()->dim() != this->mus->getSize() || this->BoundGrid->getStorage()->dim() != this->sigmas->getSize())
	{
		throw algorithm_exception("BlackScholesParabolicPDESolverSystemEuropean::BlackScholesParabolicPDESolverSystemEuropean : Dimension of mu and sigma parameters don't match the grid's dimensions!");
	}

	// test if number of dimensions in the coefficients match the numbers of grid dimensions (rho)
	if (this->BoundGrid->getStorage()->dim() != this->rhos->getNrows() || this->BoundGrid->getStorage()->dim() != this->rhos->getNcols())
	{
		throw algorithm_exception("BlackScholesParabolicPDESolverSystemEuropean::BlackScholesParabolicPDESolverSystemEuropean : Row or col of rho parameter don't match the grid's dimensions!");
	}

	// test if all algorithmic dimensions are inside the grid's dimensions
	for (size_t i = 0; i < this->BSalgoDims.size(); i++)
	{
		if (this->BSalgoDims[i] >= this->BoundGrid->getStorage()->dim())
		{
			throw algorithm_exception("BlackScholesParabolicPDESolverSystemEuropean::BlackScholesParabolicPDESolverSystemEuropean : Minimum one algorithmic dimension is not inside the grid's dimensions!");
		}
	}

	// test if there are double algorithmic dimensions
	std::vector<size_t> tempAlgoDims(this->BSalgoDims);
	for (size_t i = 0; i < this->BSalgoDims.size(); i++)
	{
		size_t dimCount = 0;
		for (size_t j = 0; j < tempAlgoDims.size(); j++)
		{
			if (this->BSalgoDims[i] == tempAlgoDims[j])
			{
				dimCount++;
			}
		}

		if (dimCount > 1)
		{
			throw algorithm_exception("BlackScholesParabolicPDESolverSystemEuropean::BlackScholesParabolicPDESolverSystemEuropean : There is minimum one doubled algorithmic dimension!");
		}
	}

	// build the coefficient vectors for the operations
	this->gammaCoef = new DataMatrix(this->BSalgoDims.size(), this->BSalgoDims.size());
	this->deltaCoef = new DataVector(this->BSalgoDims.size());

	// create the inner grid
	this->GridConverter->buildInnerGridWithCoefs(*this->BoundGrid, *this->alpha_complete, &this->InnerGrid, &this->alpha_inner);

	if (bLogTransform == false)
	{
		buildDeltaCoefficients();
		buildGammaCoefficients();

		//Create needed operations, on inner grid
		this->OpDeltaInner = this->InnerGrid->createOperationDelta(*this->deltaCoef);
		this->OpGammaInner = this->InnerGrid->createOperationGamma(*this->gammaCoef);
		// Create needed operations, on boundary grid
		this->OpDeltaBound = this->BoundGrid->createOperationDelta(*this->deltaCoef);
		this->OpGammaBound = this->BoundGrid->createOperationGamma(*this->gammaCoef);
	}
	// create needed operations that are different in case of a log-transformed Black-Scholoes equation
	else
	{
		buildDeltaCoefficientsLogTransform();
		buildGammaCoefficientsLogTransform();

		// operations on boundary grid
		this->OpDeltaBound = this->BoundGrid->createOperationDeltaLog(*this->deltaCoef);
		this->OpGammaBound = this->BoundGrid->createOperationGammaLog(*this->gammaCoef);
		//operations on inner grid
		this->OpDeltaInner = this->InnerGrid->createOperationDeltaLog(*this->deltaCoef);
		this->OpGammaInner = this->InnerGrid->createOperationGammaLog(*this->gammaCoef);
	}

	// Create operations, independent bLogTransform
	this->OpLTwoInner = this->InnerGrid->createOperationLTwoDotProduct();
	this->OpLTwoBound = this->BoundGrid->createOperationLTwoDotProduct();

	// right hand side if System
	this->rhs = NULL;

	// set coarsen settings
	this->useCoarsen = useCoarsen;
	this->coarsenThreshold = coarsenThreshold;
	this->refineThreshold = refineThreshold;
	this->adaptSolveMode = adaptSolveMode;
	this->numCoarsenPoints = numCoarsenPoints;
	this->refineMode = refineMode;
	this->refineMaxLevel = refineMaxLevel;

	// init Number of AverageGridPoins
	this->numSumGridpointsInner = 0;
	this->numSumGridpointsComplete = 0;
}

BlackScholesParabolicPDESolverSystemEuropean::~BlackScholesParabolicPDESolverSystemEuropean()
{
	delete this->OpDeltaBound;
	delete this->OpGammaBound;
	delete this->OpLTwoBound;
	delete this->OpDeltaInner;
	delete this->OpGammaInner;
	delete this->OpLTwoInner;
	delete this->gammaCoef;
	delete this->deltaCoef;
	delete this->BoundaryUpdate;
	delete this->GridConverter;
	if (this->InnerGrid != NULL)
	{
		delete this->InnerGrid;
	}
	if (this->alpha_inner != NULL)
	{
		delete this->alpha_inner;
	}
	if (this->rhs != NULL)
	{
		delete this->rhs;
	}
	delete this->alpha_complete_old;
	delete this->alpha_complete_tmp;
}

void BlackScholesParabolicPDESolverSystemEuropean::applyLOperatorComplete(DataVector& alpha, DataVector& result)
{
	DataVector temp(alpha.getSize());

	result.setAll(0.0);

	// Apply the riskfree rate
	if (this->r != 0.0)
	{
		this->OpLTwoBound->mult(alpha, temp);
		result.axpy((-1.0)*this->r, temp);
	}

	// Apply the delta method
	this->OpDeltaBound->mult(alpha, temp);
	result.add(temp);

	// Apply the gamma method
	this->OpGammaBound->mult(alpha, temp);
	result.sub(temp);
}

void BlackScholesParabolicPDESolverSystemEuropean::applyLOperatorInner(DataVector& alpha, DataVector& result)
{
	DataVector temp(alpha.getSize());

	result.setAll(0.0);

	// Apply the riskfree rate
	if (this->r != 0.0)
	{
		this->OpLTwoInner->mult(alpha, temp);
		result.axpy((-1.0)*this->r, temp);
	}

	// Apply the delta method
	this->OpDeltaInner->mult(alpha, temp);
	result.add(temp);

	// Apply the gamma method
	this->OpGammaInner->mult(alpha, temp);
	result.sub(temp);
}

void BlackScholesParabolicPDESolverSystemEuropean::applyMassMatrixComplete(DataVector& alpha, DataVector& result)
{
	DataVector temp(alpha.getSize());

	result.setAll(0.0);

	// Apply the mass matrix
	this->OpLTwoBound->mult(alpha, temp);

	result.add(temp);
}

void BlackScholesParabolicPDESolverSystemEuropean::applyMassMatrixInner(DataVector& alpha, DataVector& result)
{
	DataVector temp(alpha.getSize());

	result.setAll(0.0);

	// Apply the mass matrix
	this->OpLTwoInner->mult(alpha, temp);

	result.add(temp);
}

void BlackScholesParabolicPDESolverSystemEuropean::finishTimestep(bool isLastTimestep)
{
	// Replace the inner coefficients on the boundary grid
	this->GridConverter->updateBoundaryCoefs(*this->alpha_complete, *this->alpha_inner);

#ifndef NOBOUNDARYDISCOUNT
	// Adjust the boundaries with the riskfree rate
	if (this->r != 0.0)
	{
		if (this->tOperationMode == "ExEul" || this->tOperationMode == "AdBas")
		{
			this->BoundaryUpdate->multiplyBoundary(*this->alpha_complete, exp(((-1.0)*(this->r*this->TimestepSize))));
		}
	}
#endif

	// add number of Gridpoints
	this->numSumGridpointsInner += this->InnerGrid->getSize();
	this->numSumGridpointsComplete += this->BoundGrid->getSize();

	if (this->useCoarsen == true && isLastTimestep == false)
	{
		///////////////////////////////////////////////////
		// Start integrated refinement & coarsening
		///////////////////////////////////////////////////

		size_t originalGridSize = this->BoundGrid->getStorage()->size();

		// Coarsen the grid
		GridGenerator* myGenerator = this->BoundGrid->createGridGenerator();

		//std::cout << "Coarsen Threshold: " << this->coarsenThreshold << std::endl;
		//std::cout << "Grid Size: " << originalGridSize << std::endl;

		if (this->adaptSolveMode == "refine" || this->adaptSolveMode == "coarsenNrefine")
		{
			size_t numRefines = myGenerator->getNumberOfRefinablePoints();
			SurplusRefinementFunctor* myRefineFunc = new SurplusRefinementFunctor(this->alpha_complete, numRefines, this->refineThreshold);
			if (this->refineMode == "maxLevel")
			{
				myGenerator->refineMaxLevel(myRefineFunc, this->refineMaxLevel);
				this->alpha_complete->resizeZero(this->BoundGrid->getStorage()->size());
			}
			if (this->refineMode == "classic")
			{
				myGenerator->refine(myRefineFunc);
				this->alpha_complete->resizeZero(this->BoundGrid->getStorage()->size());
			}
			delete myRefineFunc;
		}

		if (this->adaptSolveMode == "coarsen" || this->adaptSolveMode == "coarsenNrefine")
		{
			size_t numCoarsen = myGenerator->getNumberOfRemoveablePoints();
			SurplusCoarseningFunctor* myCoarsenFunctor = new SurplusCoarseningFunctor(this->alpha_complete, numCoarsen, this->coarsenThreshold);
			myGenerator->coarsenNFirstOnly(myCoarsenFunctor, this->alpha_complete, originalGridSize);
			delete myCoarsenFunctor;
		}

		delete myGenerator;

		///////////////////////////////////////////////////
		// End integrated refinement & coarsening
		///////////////////////////////////////////////////

		// rebuild the inner grid + coefficients
		this->GridConverter->rebuildInnerGridWithCoefs(*this->BoundGrid, *this->alpha_complete, &this->InnerGrid, &this->alpha_inner);
	}
}

void BlackScholesParabolicPDESolverSystemEuropean::startTimestep()
{
#ifndef NOBOUNDARYDISCOUNT
	// Adjust the boundaries with the riskfree rate
	if (this->r != 0.0)
	{
		if (this->tOperationMode == "CrNic" || this->tOperationMode == "ImEul")
		{
			this->BoundaryUpdate->multiplyBoundary(*this->alpha_complete, exp(((-1.0)*(this->r*this->TimestepSize))));
		}
	}
#endif
}

void BlackScholesParabolicPDESolverSystemEuropean::buildGammaCoefficients()
{
	size_t dim = this->BSalgoDims.size();

	for (size_t i = 0; i < dim; i++)
	{
		for (size_t j = 0; j < dim; j++)
		{
			// handle diagonal
			if (i == j)
			{
			  this->gammaCoef->set(i, j, 0.5*((this->sigmas->get(this->BSalgoDims[i])*this->sigmas->get(this->BSalgoDims[j]))*this->rhos->get(this->BSalgoDims[i],this->BSalgoDims[j])));
			}
			else
			{
			  this->gammaCoef->set(i, j, ((this->sigmas->get(this->BSalgoDims[i])*this->sigmas->get(this->BSalgoDims[j]))*this->rhos->get(this->BSalgoDims[i],this->BSalgoDims[j])));
			}
		}
	}
}

void BlackScholesParabolicPDESolverSystemEuropean::buildDeltaCoefficients()
{
	size_t dim = this->BSalgoDims.size();
	double covar_sum = 0.0;

	for (size_t i = 0; i < dim; i++)
	{
		covar_sum = 0.0;
		for (size_t j = 0; j < dim; j++)
		{
			// handle diagonal
			if (i == j)
			{
				covar_sum += ((this->sigmas->get(this->BSalgoDims[i])*this->sigmas->get(this->BSalgoDims[j]))*this->rhos->get(this->BSalgoDims[i],this->BSalgoDims[j]));
			}
			else
			{
				covar_sum += (0.5*((this->sigmas->get(this->BSalgoDims[i])*this->sigmas->get(this->BSalgoDims[j]))*this->rhos->get(this->BSalgoDims[i],this->BSalgoDims[j])));
			}
		}
		this->deltaCoef->set(i, this->mus->get(this->BSalgoDims[i])-covar_sum);
	}
}

void BlackScholesParabolicPDESolverSystemEuropean::buildGammaCoefficientsLogTransform()
{
	size_t dim = this->BSalgoDims.size();

	for (size_t i = 0; i < dim; i++)
	{
		for (size_t j = 0; j < dim; j++)
		{
			// handle diagonal
			if (i == j)
			{
			  this->gammaCoef->set(i, j, 0.5*((this->sigmas->get(this->BSalgoDims[i])*this->sigmas->get(this->BSalgoDims[j]))*this->rhos->get(this->BSalgoDims[i],this->BSalgoDims[j])));
			}
			else
			{
			  this->gammaCoef->set(i, j, ((this->sigmas->get(this->BSalgoDims[i])*this->sigmas->get(this->BSalgoDims[j]))*this->rhos->get(this->BSalgoDims[i],this->BSalgoDims[j])));
			}
		}
	}
}

void BlackScholesParabolicPDESolverSystemEuropean::buildDeltaCoefficientsLogTransform()
{
	size_t dim = this->BSalgoDims.size();

	for (size_t i = 0; i < dim; i++)
	{
		this->deltaCoef->set(i, this->mus->get(this->BSalgoDims[i])-(0.5*(this->sigmas->get(this->BSalgoDims[i])*this->sigmas->get(this->BSalgoDims[i]))));
	}
}

}
}
