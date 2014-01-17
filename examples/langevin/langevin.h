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

//#include "plot_tseries.h"
#include "solvers/solver.h"
//#include "solvers/dummy.h"
//#include "solvers/RK4.h"
#include "solvers/euler.h"
#include "solvers/rkf45_gsl.h"
//#include "solvers/euler_sttic.h"
//#include "solvers/noise.h"
#include "ui/tab.h"
#include "ui/curve.h"
#include "ui/plot.h"
#include "ui/histogram.h"
//#include "ui/tab_output.h"

/* TODO:
 */

class odeDef
{
protected:

public:
    typedef Matrix<double, 1, 1> XVector;
//  typedef Vector2d XVector;
//    typedef double TNoise;
//    typedef CLHEP::TripleRand Engine;
//    typedef CLHEP::RandGaussT Dist;
    //typedef solvers::Stochastic<Engine, Dist, TNoise, double, double>;

  ODETypes::NOISE_SHAPE g_shape = ODETypes::NOISE_NONE;

  odeDef(){
//    solver.setODE(this);
  }
    //odeDef(XVector initial_value) : solution(initial_value) {}

    double alpha;
    XVector x0;             // Initial value
    bool initialized=false;


    void init();

    // Functions here should always be inlined, as they are likely to be called MANY times


    /* The total differential of the ODE that we want to solve, i.e. dX = f(t, x)dt + g(t, x)dW
     */
    struct func_dX{
        double alpha;
        double tau;

        solvers::Series<XVector>& txSeries;

        func_dX(solvers::Series<XVector>& series)
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
            Xtau << txSeries.interp("t", t - tau, "x1");
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

};

inline void odeDef::init() {

    //dX.alpha = alpha;


    // Give initial value to the solution function
    //solution.x0 = x0;

    initialized = true;
}






/* Driver class
 */

class Langevin : public QMainWindow
{
    Q_OBJECT
    
public:
    // TODO: use generic class which unpacks to the number of parameters required for the distribution
//    typedef solvers::Noise<odeDef::Engine, odeDef::Dist, odeDef::TNoise, double, double> StticGen;
    typedef solvers::RKF45_gsl<odeDef, odeDef::XVector> TSolver;


    explicit Langevin(QMainWindow *parent = 0);
    ~Langevin() {}
    
private:
    void setupUI();

//    TSolver solver;
    odeDef ode;


//    tab_output<TSolver> tabOutput;
    cent::tab* tabOutput;

};








#endif // Langevin_H
