/* Generic solver class.
 *
 * All solvers should inherit from this one; it provides a generic interface,
 * as well as general functions that may be overloaded if needed.
 */

#ifndef SOLVER_H
#define SOLVER_H

//#include "DEET_global.h"

#include <iostream>
#include <memory>
#include <utility>
#include <vector>
#include <functional>
#include <string>
#include <assert.h>

#include <o2scl/table.h>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/StdVector>

#include "odetypes.h"

// The consequence of this is that odedef.h cannot itself include any class derived from solver.h

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

  /* Specialized class for tables containing series data 
   * (i.e. nD dependent vector (x) vs 1D independent variable (t))
   */
  template <typename XVector> 
  class Series : public o2scl::table<std::vector<double> >
  {
  public:
    Series(size_t cmaxlines=0);
    void line_of_data(double t, XVector x);
    XVector getVectorAtTime(const size_t t_idx) const;
  };


  /* XVector should be a class derived from Eigen/Matrix */
  template <typename ODEdef, typename XVector> class Solver
  {
  public:
    //typedef vector<XVector, aligned_allocator<XVector> > TXSeries;

    Solver() {
      std::cerr << "Creating 'Solver'" << std::endl;
    }
    Solver(const Solver& source) = delete;
    // We disable the copy constructor because it can lead to problems, notably:
    //      - Copying of large result data sets
    //      - Non-synchronious copies, if it was made by accident and one expects a reference

    virtual ~Solver() {
      std::cerr << "Deleting 'Solver'" << std::endl;
    }

    /* Link the ODE instance to the solver instance.
     */
    void setODE(ODEdef* ODE) {

	  ode = ODE;
//	  dX = ode->dX;

	  // Check that the format of the ode corresponds to what the solver expects.
	  // If the format can be adapted by discarding part of the ODE (eg. noise component), emit a warning
	  if ((noiseShape == ODETypes::NOISE_NONE) and (ode->g_shape != ODETypes::NOISE_NONE)) {
		std::cerr << "Solver is not stochastic. Only the deterministic component of the ODE will be considered." << std::endl;
	  } else {
		assert(noiseShape == ode->g_shape);
	  }
    }


    /* setRange functions set tBegin, tEnd, tStepsize and tNumsteps to
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

    void solve();                /* This function should always be overloaded by the actual solver */




  protected:
    /*vector<double> series_t;    //independent variable of the ODE solution
	  TXSeries series_x;   //dependent variable(s) of the ODE solution
	  //  Not using an Eigen matrix to store these vectors as columns
	  //  allows the vectors themselves to be matrices, if required.
	  */
    Series<XVector> odeSeries;
    double tBegin = 0;
    double tEnd = 0;                 // functions can check these are set by testing tBegin == tEnd
    double tStepSize = 0;            // either StepSize or NumSteps should be computed internally
    unsigned long tNumSteps = 0;
    //double initX;                  // If I decide to use this, use 'x0' instead
    // int XDim;                     // Probably should be removed
    bool initConditionsSet = false;
    ODETypes::NOISE_SHAPE noiseShape;

    void discretize();
    ODEdef* ode;
//    Functor dX;
  };


#include "solver.tpp"
}

#endif // SOLVER_H
