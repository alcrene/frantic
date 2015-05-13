#ifndef OU_PROCESS_H
#define OU_PROCESS_H

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

#include "integrators/integrator.h"
#include "integrators/io.h"
#include "integrators/stochastic.h"
#include "ui/uiparameter.h"

/* Definition class of the system of differential equations
 * Here the actual equations are defined, as well as the vector and series data types.
 */

class OU_Process
{
public:
  using XVector =  Eigen::Matrix<double, 1, 1>;                     // Basic dependent variable type. Can be scalar (Matrix<1,1>), vector or matrix
  // 2* because we need amplitude, phase
  // Combining multiple forms of history is as simple as creating a child
  // class that inherits both and overloading the update function
  using XSeries = frantic::InterpolatedSeries<XVector, 1, 3>;  // using Series = […] causes conflicts with the parent class
  using XProbabilityDensity = frantic::ProbabilityDensity<XVector>;
  class XHistory : public XSeries, public XProbabilityDensity
  {
  public:
    XHistory (const std::string& varname, size_t estimated_snapshots=0):
      XSeries(varname), XProbabilityDensity(estimated_snapshots) {}
    void update(double t, const XVector& x) {
      XSeries::update(t, x);
      XProbabilityDensity::update(t,x);
    }
    void reset() {
      XSeries::reset();
      XProbabilityDensity::reset();
    }

    XSeries::dump_to_text_t save_0 = XSeries::dump_to_text();
    XProbabilityDensity::dump_to_text_t save_1 = XProbabilityDensity::dump_to_text();

    // If a second Series is included for errors, overload the
    // set, set_initial_state and update methods
  };
  

  /********************************************************
   * Process parameters                                   *
   ********************************************************/
  using Parameter = frantic::InputUIParameter<double>;
  using Parameters = frantic::UIParameterTuple<false, Parameter, Parameter, Parameter>;
  Parameter alpha;
  Parameter tau;
  Parameter D;
  Parameters parameters;

  /********************************************************
   * Other values we want to cache                        *
   * (are typically computed from the parameters)         *
   ********************************************************/
  Eigen::VectorXcd lambda;
  Eigen::VectorXcd K;

  /********************************************************
   * Constructor
   ********************************************************/
  OU_Process()
    : alpha("alpha","α", -1), tau("tau", "τ", 1), D("D", "D", 1),
      parameters(alpha, tau, D) {}

  /********************************************************
   * Run initializer                                      *
   * This function is executing right before integrating; *
   * use it to cache quantities that are used in the      *
   * drift and diffusion functions                        *
   ********************************************************/
  void initialize() { }


  /********************************************************
   * Drift portion of the differential equation.          *
   ********************************************************/
  XVector drift(double t, const XVector& x, const XHistory& history) const {
    // const is required to accept temporaries
    static XVector x_out;
    x_out(0) = alpha.value * history(t - tau.value)(0);
    for (int n = 1; n <= n_modes; ++n) {
      //x_out(2*n - 1) = lambda(n).real() * x(2*n - 1);
      //x_out(2*n) = lambda(n).imag();
      x_out(2*n - 1) = lambda(n).real() * x(2*n - 1) - lambda(n).imag()*x(2*n);
      x_out(2*n) = lambda(n).imag() * x(2*n - 1) + lambda(n).real() * x(2*n);
    }
    return x_out;
  }


  /********************************************************
   * Diffusion portion of the differential equation.      *
   ********************************************************/
  using DiffusionCoeff = frantic::Tuple<XVector>;
  using DiffusionDifferential = frantic::Tuple<double>;
  typename frantic::GaussianWhiteNoise<double> generator1;

  DiffusionCoeff diffusion_coeffs(double t, const XVector& x, const XHistory& history) const {
    static XVector x_out;
    x_out(0) = sqrt(2*D.value);
    for (int n=1; n <= n_modes; ++n) {
      //x_out(2*n - 1) = sqrt(2*D.value) * (K(n).real() * cos(x(2*n))
      //                                    + K(n).imag() * sin(x(2*n)));
      //x_out(2*n) = sqrt(2*D.value) / x(2*n - 1) * (K(n).imag() * cos(x(2*n))
      //                                             - K(n).real() * sin(x(2*n)));
      x_out(2*n - 1) = sqrt(2*D.value) * K(n).real();
      x_out(2*n) = sqrt(2*D.value) * K(n).imag();
    }
    return DiffusionCoeff(x_out);
  }

  DiffusionDifferential diffusion_differentials(double dt) const {
    return DiffusionDifferential(generator1(dt));
  }


};

#endif // OU_PROCESS_H
