import pysgpp
import numpy as np
import matplotlib.pyplot as plt
import math

pi = math.pi

class interpolation_function():
  def __init__(self, d, f):
    self.f = f
    self.d = d
    self.grid = pysgpp.Grid.createPolyGrid(d, 3)
    self.gridStorage = self.grid.getStorage()
    self.hierarch = pysgpp.createOperationHierarchisation(self.grid)
    self.opeval = pysgpp.createOperationEval(self.grid)
    self.alpha = pysgpp.DataVector(self.gridStorage.getSize())

  def create_interpolation(self, grid_lvl):
    self.gridStorage.clear()
    self.grid.getGenerator().regular(grid_lvl)
    self.alpha = pysgpp.DataVector(self.gridStorage.getSize())
    self.min_f = float('inf')
    for i in range(self.gridStorage.getSize()):
      gp = self.gridStorage.getPoint(i)
      x = [self.gridStorage.getCoordinate(gp, j) for j in range(self.d)]
      self.alpha[i] = self.f(x)
      if self.alpha[i] < self.min_f:
          self.min_f = self.alpha[i]
          self.min_x = x
    self.hierarch.doHierarchisation(self.alpha)

  def __call__(self, x):
    if (self.d == 1 and not isinstance(x, list)):
        x = [x]
    return self.opeval.eval(self.alpha, pysgpp.DataVector(x))

# ---------------------------------------------

def distrib(x):
    if isinstance(x, list):
        x = x[0]
    return (np.sin(2.5*pi*x - (pi - pi/4.)) + 1./2**0.5) / 0.527044

def eval_rosenblatt(sg_pdf, xs):
    op = pysgpp.createOperationRosenblattTransformation1D(sg_pdf.grid)
    ys = []
    for x in xs:
        ys.append(op.doTransformation1D(sg_pdf.alpha, x))
        print("------------------------------")
    return ys
# ---------------------------------------------

interpolation = interpolation_function(1, distrib)
interpolation.create_interpolation(6)

xs = np.arange(0, 1, 1e-2)
# ys = [interpolation(x) for x in xs]
ys = eval_rosenblatt(interpolation, xs)
plt.plot(xs, ys)
plt.legend()
plt.show()
