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
#include <eigen3/Eigen/Dense>
#include <QMainWindow>
#include <QGridLayout>

#include <iostream>

#include "solvers/solver.h"
#include "solvers/euler.h"
#include "solvers/rkf45_gsl.h"
//#include "solvers/euler_sttic.h"
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
//    typedef double TNoise;
//    typedef solvers::InterpolatedSeries<XVector, 5, 6> XSeries;

  ODETypes::NOISE_SHAPE g_shape = ODETypes::NOISE_NONE;

  typedef solvers::InterpolatedSeries<XVector, 5, 6> SeriesParent;
  class XSeries;           // Derives from SeriesParent

  double alpha;
  XVector x0;             // Initial value of the IVP


  /* The total differential of the ODE that we want to solve, i.e. dX = f(t, x)dt + g(t, x)dW
   * Should ensure that the functions in func_dX can be inlined, as they are likely to be called MANY times
   */
  struct func_dX;  // Defined below

};


/* UI driver class
 * Highest level code execution. Also provides UI.
 * Code in accompanying .cpp file
 */

class Langevin : public QMainWindow
{
    Q_OBJECT

public:
    // TODO: use generic class which unpacks to the number of parameters required for the distribution
//    typedef solvers::Noise<odeDef::Engine, odeDef::Dist, odeDef::TNoise, double, double> StticGen;
    typedef solvers::Euler<odeDef, odeDef::XVector, odeDef::XSeries> TSolver;


    explicit Langevin(QMainWindow *parent = 0);
    ~Langevin() {}

private:
    void setupUI();

    odeDef ode;

    cent::tab* tabOutput;

};





class odeDef::XSeries : public odeDef::SeriesParent //Vector type, interpolation order, # nodes
                                            //Order should match that of integrator
{
public:
  XSeries(std::string varname="x", size_t cmaxlines=0) :
          SeriesParent(varname, cmaxlines) {}

  void line_of_data(double t, XVector x) {
    // There is a measurable cost (10% at most) to overloading like this

    SeriesParent::line_of_data(t, x);
  }
};

struct odeDef::func_dX{
    double alpha;
    double tau;

    XSeries& txSeries;

    func_dX(XSeries& series)
      :txSeries(series) {
    }


    void setParameters(solvers::Param parameters) {
      auto itr = parameters.find("alpha"); assert(itr != parameters.end());
      alpha = itr->second;

      itr = parameters.find("tau"); assert(itr != parameters.end());
      tau = itr->second;
    }

    /* Initial condition function for delay equations
     */
    XVector initPhi(double t); // Define in .cpp file

    /* Drift portion of the differential equation.
     * t_idx is the index, in txSeries, of the time at which we want to calculate the derivative
     */
    // const is required to accept temporaries
    XVector f(const double& t, const XVector& X) {
      static XVector Xtau;
      if (t < tau) {
          Xtau = initPhi(t - tau);
      } else {
        // If code crashes here, check that there are at least 3 points
        // in txSeries to do the interpolation
        Xtau << txSeries.interpolate(t - tau);
        //Xtau << txSeries.interp(0, t - tau, 1);
          //idx = txSeries.ordered_lookup("t", t-tau);

      }
      return f(t, X, Xtau);
    }
    XVector f(const double& t, const XVector& X, const XVector& Xtau) {
      return alpha * Xtau;
    }

// Functions g and dS are only relevent for stochastic equations
/*        XVector g(const XVector& x, const double& t) {
        static XVector retval;
        retval << 0, D;
        return retval;
    }
*/
    /* Noise increment */
    /*TNoise dS(const dRKF45_gslouble dt) {

    }*/

};









#endif // Langevin_H
