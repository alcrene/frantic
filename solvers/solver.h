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
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/StdVector>

#include "odedef.h"

using std::vector;
using namespace::Eigen;

namespace solvers {
  /* XVector should be a class derived from Eigen/Matrix */
  template <typename Functor, typename XVector> class Solver
  {
  public:
    typedef vector<XVector, aligned_allocator<XVector> > TXSeries;

    //solver(int XDimension);
    Solver(odeDef* ODE):ode(ODE) {
      // XDim = XVector::RowsAtCompileTime;
      dX = ode->dX;
      std::cerr << "Creating 'Solver'" << std::endl;
    }
    Solver(const Solver& source) = delete;
    // We disable the copy constructor because it can lead to problems, notably:
    //      - Copying of large result data sets
    //      - Break of synchronicity between the copies, if it was made by accident and one expects a reference

    virtual ~Solver() {
      std::cerr << "Deleting 'Solver'" << std::endl;
    }

    /* setRange functions set tBegin, tEnd, tStepsize and tNumsteps to
     * compatible values for use by discretize() */
    void setRange(double begin, double end, double stepSize);
    void setRange(double begin, double end, int numSteps);
    vector<double> getTSeriesVector(int stepMultiplier=0);
    vector<double> getXSeriesVector(ptrdiff_t component);
    vector<double> getXSeriesVector(ptrdiff_t component, TXSeries xseries);
    TXSeries evalFunction(std::function<XVector(const double&)> f);
    vector<double> evalFunctionComponent(ptrdiff_t component, std::function<XVector(const double&)> f);


    // Debugging helpers
    void dump(std::string vector="", ptrdiff_t component=0);

    void solve();                /* This function should always be overloaded by the actual solver */

    /* Useful mathematical/vector functions */




  protected:
    vector<double> series_t;    //independent variable of the ODE solution
    TXSeries series_x;   //dependent variable(s) of the ODE solution
	//  Not using an Eigen matrix to store these vectors as columns
	//  allows the vectors themselves to be matrices, if required.
    double tBegin = 0;
    double tEnd = 0;                 // functions can check these are set by testing tBegin == tEnd
    double tStepSize = 0;            // either StepSize or NumSteps should be computed internally
    unsigned tNumSteps = 0;
    //double initX;                  // If I decide to use this, use 'x0' instead
    // int XDim;                     // Probably should be removed
    bool initConditionsSet = false;

    void discretize();
    odeDef* ode;
    Functor dX;
  };

  /* Generally useful functions */

  // C++ idiom for getting array size; see http://stackoverflow.com/questions/4810664/how-do-i-use-arrays-in-c
  // Doesn't seem to work with smart pointer arrays
  /*template< class Type, ptrdiff_t n >
	ptrdiff_t countOf( Type (&)[n] ) { return n; }*/

#include "solver.tpp"
}

#endif // SOLVER_H
