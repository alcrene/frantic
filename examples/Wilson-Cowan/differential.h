#ifndef DIFFERENTIAL_H
#define DIFFERENTIAL_H

#include <map>
#include <string>
#include <vector>
#include <array>

#include <functional>
#include <eigen3/Eigen/Dense>

#include <cmath>
#include <algorithm>
#include <random>


#include "o2scl/hist.h"
#include "o2scl/uniform_grid.h"

#include "solvers/solver.h"
#include "solvers/histcollection.h"
#include "solvers/euler.h"
#include "solvers/rkf45_gsl.h"
#include "solvers/euler_sttic.h"


/* Definition class of the system of differential equations
 * Here the actual equations are defined, as well as the vector and series data types.
 * The Series-derived class used to store the result can also be overloaded, to provide further control.
 */
struct Differential
{

  using XVector =  Eigen::Vector2d;                                 // Basic dependant variable type. Can be scalar (Matrix<1,1>), vector or matrix
  using XMatrix = Eigen::Matrix2d;                                  // If XVector is an nx1 vector, this corresponds to an nxn matrix. Can be used for linear mixing operators; does not work as is if XVector is a matrix
  using SeriesParent = solvers::InterpolatedSeries<XVector, 1, 3>;  // The series class. We derive it to allow doing extra things (like adding to a histogram everytime we compute a new point).
                                                                    // Template parameters are:  XVector, the minimum interpolation order and the number of points used for interpolation
  class XSeries;                                                    // Derives from SeriesParent

  struct ParamType {
    // Container class for all parameters in this differential
    XVector alpha;          // linear relaxation strength
    XVector beta;           // nonlinear component (w/ coupling) strength
    XMatrix w;              // Weight matrix between populations
    double tau;             // length of delay
    double D;               // Fokker-Planck diffusion constant ( ξ = √(2D) dW/dt)

    ParamType();                      // Empty constructor must be implementated because copy constructor below removes defalut (true ?)
    ParamType(const ParamType& source);   // Copy constructor isn't implemented automatically
    virtual ~ParamType() {}
  };
  ODETypes::NOISE_SHAPE g_shape = ODETypes::NOISE_VECTOR;           // Define shape of stochastic part of problem. Used for homegrown type checking

private:
  const XSeries* txSeries = NULL;     // Pointer to the time series data (useful for look backs)
  ParamType params;

public:
  Differential() {}
  Differential(const XSeries& series) : Differential(series, ParamType()) {}
  Differential(const XSeries& series, const ParamType parameters) : txSeries(&series), params(parameters) {}

  void setSeries(const XSeries& series);
  void setParameters(const ParamType& parameters);

  /* Initial values */
  XVector x0;                              // Initial value of the IVP
  double t0, tn;                           // The initial and final times  \todo: These are currently unused (setRange takes own values). Make IV definition consistent (same place for x0, t0, tn)
  XVector initPhi(const double t) const;   // Initial condition function for delay equations; define in .cpp file
  XVector h(const double t) const;         // Input function; define in .cpp file

  /* Drift portion of the differential equation.*/
  XVector f(const double t, const XVector& X) const;
  XVector f(const double t, const XVector& X, const XVector& Xtau) const;
  XVector F(XVector x) const;

  /* Diffusion portion of the differential equation */
  using engine_t = std::mt19937;
  using dist_t = std::normal_distribution<>;
private:
  mutable engine_t engine{};
  mutable dist_t dist;

public:
  XVector g(const double t, const XVector& x) const;
  double dS(const double dt) const;

};


class Differential::XSeries : public Differential::SeriesParent //Vector type, interpolation order, # nodes
    //Order should match that of integrator
{
public:
  XSeries(std::string varname="x", size_t cmaxlines=0) :
    SeriesParent(varname, cmaxlines) {}

  void line_of_data(double t, typename Differential::XVector x);
};


/* The total differential of the ODE that we want to solve, i.e. dX = f(t, x)dt + g(t, x)dS
 * Should ensure that the functions in func_dX can be inlined, as they are likely to be called MANY times
 */


inline Differential::ParamType::ParamType() {                // Empty constructor must be implementated because copy constructor below removes defalut
  alpha=XVector(); beta=XVector(); w=XMatrix(); tau = 0;
}
inline Differential::ParamType::ParamType(const ParamType& source) {  // Copy constructor isn't implemented automatically
  alpha = source.alpha;
  beta = source.beta;
  w = source.w;
  tau = source.tau;
  D = source.D;
}


inline void Differential::setSeries(const XSeries& series) {
  txSeries = &series;
}
inline void Differential::setParameters(const ParamType& parameters) {
  params = parameters;
}


/* Drift portion of the differential equation.
   */
// const is required to accept temporaries
inline typename Differential::XVector Differential::f(const double t, const XVector& X) const {
  static XVector Xtau;
  if (t < params.tau) {
    Xtau = initPhi(t - params.tau);
  } else {
    assert(txSeries != NULL);
    Xtau << txSeries->interpolate(t - params.tau);
  }
  return f(t, X, Xtau);
}

inline typename Differential::XVector Differential::f(const double t, const XVector& X, const XVector& Xtau) const {
  return -params.alpha.cwiseProduct(X) + params.beta.cwiseProduct(F(h(t) + params.w*Xtau));
}

/* Sigmoidal non-linear component of Wilson-Cowan model */
inline typename Differential::XVector Differential::F(XVector x) const {
  return 1 / (1 + x.array().exp());  // Convert to array for coefficient-wise operations
}

/* =============================================================
   * Stochastic components
   * (comment out / delete for deterministic problems)
   * ============================================================= */

/* The return type of g might be XVector or a higher order tensor if there are multiple random variables.
   * The return type of dS should correspond to that of g.
   * (This is yet untested with return types other than XVector.)
   */
inline typename Differential::XVector Differential::g(const double t, const XVector& x) const {
  static XVector retval;
  retval << sqrt(2*params.D), sqrt(2*params.D);
  return retval;
}

/* Noise increment */
inline double Differential::dS (const double dt) const {
  static double lastdt = 0;
  if (lastdt != dt) {
    dist_t::param_type p(0, sqrt(dt));
    dist.param(p);
  }

  double a = dist(engine);
  return a;
}


#endif // DIFFERENTIAL_H
