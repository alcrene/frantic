/* Example run file for CENT
 * Computes the time series of the delayed Langevin equation for a set of parameters
 * and plots the result.
 */

#ifndef Langevin_H
#define Langevin_H

#include <iostream>
#include <assert.h>
#include <cmath>
#include <map>
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <functional>
#include <random>
#include <eigen3/Eigen/Dense>
#include <QMainWindow>
#include <QGridLayout>


#include "o2scl/hist.h"
#include "o2scl/uniform_grid.h"

#include "solvers/solver.h"
#include "solvers/histcollection.h"
#include "solvers/euler.h"
#include "solvers/rkf45_gsl.h"
#include "solvers/euler_sttic.h"
#include "ui/qtcent.h"
#include "ui/tab.h"
#include "ui/curve.h"
#include "ui/plot.h"
#include "ui/histogram.h"



/* Definition class of the system of differential equations
 * Here the actual equations are defined, as well as the vector and series data types.
 * The Series-derived class used to store the result can also be overloaded, to provide further control.
 */
struct odeDef
{
  typedef Matrix<double, 1, 1> XVector;
  //  typedef Vector2d XVector;
  //    typedef solvers::InterpolatedSeries<XVector, 5, 6> XSeries;
  typedef solvers::InterpolatedSeries<XVector, 5, 6> SeriesParent;  // The series class. We derive it to allow doing extra things
  // (like adding to a histogram everytime we compute a new point)
  class XSeries;           // Derives from SeriesParent

  XVector x0;             // Initial value of the IVP
  double t0, tn;      // The initial and final times


  /* The total differential of the ODE that we want to solve, i.e. dX = f(t, x)dt + g(t, x)dS
   * Should ensure that the functions in func_dX can be inlined, as they are likely to be called MANY times
   */
  struct func_dX;  // Defined below

  /* Elements defining the shape of the stochastic part of the problem,
   * and the engine we use to generate the random numbers
   * (the engine has to be used in conjunction with a distribution, defined in the func_dX)
   */
  ODETypes::NOISE_SHAPE g_shape = ODETypes::NOISE_VECTOR;
  using engine_t = std::mt19937;
  engine_t engine{};

};


/* UI driver class
 * Highest level code execution. Also provides UI.
 * Code in accompanying .cpp file
 */

class StticLangevin : public QMainWindow
{
  Q_OBJECT

public:
  typedef solvers::Euler_sttic<odeDef, odeDef::XVector, odeDef::XSeries> TSolver;

  explicit StticLangevin(QMainWindow *parent = 0);
  ~StticLangevin() {}
  void run();

private:
  void setupUI();

  odeDef ode;

  cent::tab* tabOutput;
  cent::InfoBox* statusBox;

  //signals:

  //public slots:

};





class odeDef::XSeries : public odeDef::SeriesParent //Vector type, interpolation order, # nodes
    //Order should match that of integrator
{
public:
  XSeries(std::string varname="x", size_t cmaxlines=0) :
    SeriesParent(varname, cmaxlines) {}

  void line_of_data(double t, odeDef::XVector x);

  cent::HistCollection<odeDef::XVector> probDens; // The probablity density as a series of histogram "snapshots"
};

struct odeDef::func_dX{
  double alpha;
  double tau;
  double D;   // Fokker-Planck diffusion constant (dx = sqrt(2D) dW)

  XSeries& txSeries;
  odeDef& ode;


  func_dX(XSeries& series, odeDef& ode) : txSeries(series), ode(ode) {}


  void setParameters(solvers::Param parameters) {
    auto itr = parameters.find("alpha"); assert(itr != parameters.end());
    alpha = itr->second;

    itr = parameters.find("tau"); assert(itr != parameters.end());
    tau = itr->second;

    itr = parameters.find("D"); assert(itr != parameters.end());
    D = itr->second;
  }

  /* Initial condition function for delay equations
   */
  XVector initPhi(double t); // Define in .cpp file

  /* Drift portion of the differential equation.
   */
  // const is required to accept temporaries
  XVector f(const double t, const XVector& X) {
    static XVector Xtau;
    if (t < tau) {
      Xtau = initPhi(t - tau);
    } else {
      Xtau << txSeries.interpolate(t - tau);
    }
    return f(t, X, Xtau);
  }

  XVector f(const double t, const XVector& X, const XVector& Xtau) {
    //return alpha * Xtau;
    return 0 * X;
  }

  /* =============================================================
   * Stochastic components
   * (comment out / delete for deterministic problems)
   * ============================================================= */

  using dist_t = std::normal_distribution<>;
  dist_t dist;

  /* The return type of g might be XVector or a higher order tensor if there are multiple random variables.
   * The return type of dS should correspond to that of g.
   * (This is yet untested with return types other than XVector.)
   */
  XVector g(const double t, const XVector& x) {
    static XVector retval;
    retval << sqrt(2*D);
    return retval;
  }

  /* Noise increment */
  double dS(const double dt) {
    static double lastdt = 0;
    if (lastdt != dt) {
      dist_t::param_type p(0, sqrt(dt));
      dist.param(p);
    }

    double a = dist(ode.engine);
    return a;
  }

};




#endif // Langevin_H
