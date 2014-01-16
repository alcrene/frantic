/* Generic solver class.
 *
 * All solvers should inherit from this one; it provides a generic interface,
 * as well as general functions that may be overloaded if needed.
 * 
 * Implements the few elements of o2scl/ode/ode_step.h to allow near copy-paste
 * implementations of algorithms from o2scl.
 */

#ifndef SOLVER_H
#define SOLVER_H

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

// TODO: Mark appropriate functions as const (get__, for example)


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

namespace solvers {

  typedef std::map<std::string, double> Param;

  /* Specialized class for tables containing series data 
   * (i.e. nD dependent vector (x) vs 1D independent variable (t))
   */
  template <typename XVector> 
  class Series : public o2scl::table<std::vector<double> >
  {
  public:

    struct Statistics
    {
      std::vector<double> mean;//(XVector::SizeAtCompileTime);
      std::vector<double> max;//(XVector::SizeAtCompileTime);
      std::vector<double> min;//(XVector::SizeAtCompileTime);
      long nsteps;
    };

    Series(std::string varname="x", size_t cmaxlines=0);
    void line_of_data(double t, XVector x);
    XVector getVectorAtTime(const size_t t_idx) const;
    Statistics getStatistics();
    double max(size_t icol); using o2scl::table<std::vector<double> >::max;
    double min(size_t icol); using o2scl::table<std::vector<double> >::min;
  };

  /* XVector should be a class derived from Eigen/Matrix
   * \todo: Implement move semantics
   *        Careful not to break references (e.g. txSeries in dX).
   */
  template <typename ODEdef, typename XVector> class Solver
  {
  public:
    //typedef vector<XVector, aligned_allocator<XVector> > TXSeries;

    Series<XVector> odeSeries;
    Series<XVector>& odeSeriesRef = odeSeries;  // I can't figure out why I need this, but without it, the runnning .cpp
                                                // complains that "odeSeries isn't accessible within this context".
    Series<XVector> odeSeriesError;
    Series<XVector>& odeSeriesErrorRef = odeSeriesError;

    Solver(ODEdef& ode):ode(ode), odeSeries("x"), odeSeriesError("xerr") {
	  order = 0; //Provided for O2scl compatibility
    }
    Solver(const Solver& source) = delete;
    // We disable the copy constructor because it can lead to problems, notably:
    //      - Copying of large result data sets
    //      - Non-synchronious copies, if it was made by accident and one expects a reference
    //      - e.g. dX objects expect a reference to odeSeries, which would be broken on the copy

    virtual ~Solver() {}

	/* Provided for O2scl compatibility */
	virtual int get_order() {
	  return order;
	}

    void reset();
    /* setRange functions set tBegin, tEnd, dt and tNumsteps to
     * compatible values for use by discretize() */
    void setRange(double begin, double end, double stepSize, double growFactor=1.0);
    void setRange(double begin, double end, int numSteps, double growFactor=1.0);
    vector<double> getTSeriesVector(int stepMultiplier=0);
    vector<double> getXSeriesVector(ptrdiff_t component);
    //vector<double> getXSeriesVector(ptrdiff_t component, TXSeries xseries);
    Series<XVector> evalFunction(std::function<XVector(const double&)> f);
    vector<double> evalFunctionComponent(ptrdiff_t component, std::function<XVector(const double&)> f);


    // Debugging helpers
    void dump(std::string cmpntName);

    void solve(Param parameters);                /* This function should always be overloaded by the actual solver */




  protected:
    /*vector<double> series_t;    //independent variable of the ODE solution
	  TXSeries series_x;   //dependent variable(s) of the ODE solution
	  //  Not using an Eigen matrix to store these vectors as columns
	  //  allows the vectors themselves to be matrices, if required.
	  */
    double tBegin = 0;
    double tEnd = 0;                 // functions can check these are set by testing tBegin == tEnd
    double dt = 0;            // either StepSize or NumSteps should be computed internally
    unsigned long tNumSteps = 0;
	int order; // Provided for O2scl compatibility
    //double initX;                  // If I decide to use this, use 'x0' instead
    // int XDim;                     // Probably should be removed
    bool initConditionsSet = false;
    ODETypes::NOISE_SHAPE noiseShape;

//    void discretize();
    ODEdef& ode;
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


#include "solver.tpp"
}

#endif // SOLVER_H
