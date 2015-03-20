/* Example run file for FRANTIC
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

#include "ou_process.h"
#include "standardwindow.h"

//#include "integrators/euler.h"
//#include "integrators/rkf45_gsl.h"
#include "integrators/euler_sttic.h"


class DelayedOU;

using UI = frantic::StandardWindow<DelayedOU>;

class DelayedOU
{

public:
  // These two typedefs must be here for the UI object
  using Differential = OU_Process;
  using Integrator = integrators::Euler_sttic<Differential>;

  Integrator integrator;
  Differential dX;  // If you decide to recreate a new dX object on each run,
                    // ensure the random seed isn't always reset to the same value

  /* Initial function from -r to 0 */
  struct initPhi : public Differential::XHistory::InitialState {
    Differential::XVector operator()(double t) const;
  };

  void run_initialization(UI* ui);
  void run_loop();

};

#endif // Langevin_H
