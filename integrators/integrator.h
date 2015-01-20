/* Generic integrator class.
 *
 * All integrators should inherit from this one; it provides a generic interface,
 * as well as general functions that may be overloaded if needed.
 * 
 * Implements the few elements of o2scl/ode/ode_step.h to allow near copy-paste
 * implementations of algorithms from o2scl.
 */

#ifndef INTEGRATOR_H
#define INTEGRATOR_H

//#include "DEET_global.h"

#include <cmath>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>
#include <functional>
#include <algorithm>
#include <string>
#include <map>
#include <assert.h>

#include <o2scl/table.h>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/StdVector>

#include "series.h"
#include "io.h"

/* \todo: Mark appropriate functions as const (get__, for example)
 * \todo: Use child class for InterpolatedIntegrator, similar as InterpolatedSeries ?
 */



using std::vector;
using namespace::Eigen;

namespace ODETypes {
// The shape of g(x,t) in the ODE definition
enum NOISE_SHAPE {
    NOISE_NONE,       // No noise
    NOISE_VECTOR,     // Only one random variable. i.e. noise weighting is deterministic, only magitude is random
    NOISE_MATRIX      // Multiple random variables.
};
}

namespace integrators {

  //typedef std::map<std::string, double> Param;  // \todo: Delete. Parameter struct is now defined in problem's Differential class


  /* XVector should be a class derived from Eigen/Matrix
   * XSeries is the type of the series for the result variable (mostly, whether interpolated (and with how many points) or not)
   * \todo: Implement move semantics
   *        Careful not to break references (e.g. txSeries in dX).
   */
  template <class Differential>
  class Integrator
  {
  protected:
    using XVector = typename Differential::XVector;
    using XSeries = typename Differential::XSeries;

  public:
    //typedef vector<XVector, aligned_allocator<XVector> > XSeries;

    XSeries odeSeries;
    XSeries& odeSeriesRef = odeSeries;  // I can't figure out why I need this, but without it, the runnning .cpp
                                        // complains that "odeSeries isn't accessible within this context".
    Series<XVector> odeSeriesError;
    Series<XVector>& odeSeriesErrorRef = odeSeriesError;

    Integrator() : Integrator("x") {}
    Integrator(std::string varname) : Integrator(varname, varname + "err") {}
    Integrator(std::string varname, std::string varerrname)
      : odeSeries(varname), odeSeriesError(varerrname) {
      order = 0;     //Provided for O2scl compatibility
    }
    // \todo This will require move semantics of XSeries
    // Integrator(ODEdef& ode, XSeries odeSeries): odeSeries(odeSeries),
    Integrator(const Integrator& source) = delete;
    // We disable the copy constructor because it can lead to problems, notably:
    //      - Copying of large result data sets
    //      - Non-synchronious copies, if it was made by accident and one expects a reference
    //      - e.g. dX objects expect a reference to odeSeries, which would be broken on the copy

    virtual ~Integrator() {}

	/* Provided for O2scl compatibility
	   Since O2scl expects int, we round up in case we have fractional order (which happens in stochastics) */
	virtual int get_order() {
	  return int(ceil(order));
	}

    void reset();
    /* setRange functions set tBegin, tEnd, dt and tNumsteps to
     * compatible values for use by discretize() */
    void setRange(double begin, double end, double stepSize, double growFactor=1.0);
    void setRange(double begin, double end, int numSteps, double growFactor=1.0);
    vector<double> getTSeriesVector(int stepMultiplier=0);
    vector<double> getXSeriesVector(ptrdiff_t component);
    //vector<double> getXSeriesVector(ptrdiff_t component, XSeries xseries);
    Series<XVector> evalFunction(std::function<XVector(const double&)> f);
    vector<double> evalFunctionComponent(ptrdiff_t component, std::function<XVector(const double&)> f);


    // Debugging helpers
    void dump(std::string cmpntName);

    // \todo: When Differential class is moved to FRANTIC, use Differential call signature
    void integrate(frantic::ParameterMap& parameters);                /* This function should always be overloaded by the actual integrator */




  protected:
    /*vector<double> series_t;    //independent variable of the ODE solution
          XSeries series_x;   //dependent variable(s) of the ODE solution
	  //  Not using an Eigen matrix to store these vectors as columns
	  //  allows the vectors themselves to be matrices, if required.
	  */
    double tBegin = 0;
    double tEnd = 0;                 // functions can check these are set by testing tBegin == tEnd
    double dt = 0;            // either the step size dt or NumSteps should be computed internally
    unsigned long nSteps = 0;
    float order; // Integrator order. Also provided for O2scl compatibility (but it should be converted to int)
    //double initX;                  // If I decide to use this, use 'x0' instead
    // int XDim;                     // Probably should be removed
    bool initConditionsSet = false;
    ODETypes::NOISE_SHAPE noiseShape;

//    void discretize();
//    Functor dX;

    /* Returns true if the passed time 't' is within the specified bounds for the ODE
     * Must first be initialized with setOdeDoneCondition().
     */
    bool odeDone(double t) {
      return signFlipper * t < signFlipper * tEnd;
    }

  private:
    int signFlipper = 1;

    void setOdeDoneCondition() {
      if (tEnd < tBegin) {
        signFlipper = -1;  // By multiplying by this, we effectively flip the condition
                           // from t < tEnd to t > tEnd
      }
    }
  };


#include "integrator.tpp"
}

#endif // INTEGRATOR_H
