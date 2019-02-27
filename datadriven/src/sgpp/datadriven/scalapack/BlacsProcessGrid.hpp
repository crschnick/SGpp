/*
 * Copyright (C) 2008-today The SG++ project
 * This file is part of the SG++ project. For conditions of distribution and
 * use, please see the copyright notice provided with SG++ or at
 * sgpp.sparsegrids.org
 *
 * BlacsProcessGrid.hpp
 *
 * Created on: Jan 14, 2019
 *     Author: Jan Schopohl
 */
#ifdef USE_SCALAPACK

#pragma once

#include <sgpp/datadriven/scalapack/blacs.hpp>

#include <cstddef>

namespace sgpp {
namespace datadriven {

class BlacsProcessGrid {
 public:
  /**
   * Creates a BLACS process grid with a certain number of rows and columns.
   * There must be at least rows * columns processes available.
   *
   * @param rows
   * @param columns
   */
  BlacsProcessGrid(int rows, int columns);

  /**
   * Creates a square process grid of maximum size.
   * Given p processes, creates a sqrt(p) * sqrt(p) process grid.
   */
  BlacsProcessGrid();

  ~BlacsProcessGrid();

  /**
   * @returns the context handle of the BLACS context
   */
  int getContextHandle() const;

  /**
   * @returns Total number of rows of the grid
   */
  int getTotalRows() const;

  /**
   * @returns Total number of columns of the grid
   */
  int getTotalColumns() const;

  /**
   * @returns Row of the current process
   */
  int getCurrentRow() const;

  /**
   * @returns Column of the current process
   */
  int getCurrentColumn() const;

  /**
   * @returns Number of the current process
   */
  int getCurrentProcess() const;

  static void initializeBlacs();

  static void exitBlacs();

 private:
  // system context for use in gridinit
  static int systemContext;

  // total number of available processes
  static int numberOfProcesses;

  // BLACS context handle
  int ictxt;

  // process grid rows
  int rows;

  // process grid columns
  int columns;

  // current process number
  int mypnum;

  // row of the current process
  int myrow;

  // column of the current process
  int mycolumn;
};
}  // namespace datadriven
}  // namespace sgpp

#endif  // USE_SCALAPACK