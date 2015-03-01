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

#include "history.h"
#include "io.h"

/* \todo: Mark appropriate functions as const (get__, for example)
 * \todo: Use child class for InterpolatedIntegrator, similar as InterpolatedSeries ?
 */



using std::vector;
using namespace::Eigen;

namespace frantic {


  /* XVector should be a class derived from Eigen/Matrix
   * XHistory is the type of the series for the result variable (mostly, whether interpolated (and with how many points) or not)
   * \todo: Implement move semantics
   *        Careful not to break references (e.g. txSeries in dX).
   */
  template <class Differential>
  class Integrator
  {
  protected:
    using XVector = typename Differential::XVector;
    using XHistory = typename Differential::XHistory;

  public:

    XHistory history;

    Integrator(std::string varname = "x") : history(varname) {
      order = 0;     //Provided for O2scl compatibility
    }
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
    vector<double> getXHistoryVector(ptrdiff_t component);
    //vector<double> getXHistoryVector(ptrdiff_t component, XHistory history);
    Series<XVector> evalFunction(std::function<XVector(const double&)> f);
    vector<double> evalFunctionComponent(ptrdiff_t component, std::function<XVector(const double&)> f);

    // Debugging helpers
    void dump(std::string cmpntName);

    // \todo: When Differential class is moved to FRANTIC, use Differential call signature
    void integrate(Differential dX);                /* This function should always be overloaded by the actual integrator */


  protected:
    float order; // Integrator order. Also provided for O2scl compatibility (but it should be converted to int)
  };

#include "integrator.tpp"
}

#endif // INTEGRATOR_H
