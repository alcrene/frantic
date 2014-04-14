/* Basic ODE solver using an Euler-scheme
 */

#ifndef EULER_H
#define EULER_H

#include "solver.h"

using std::vector;

namespace solvers {

template <class Differential>
class Euler : public Solver<Differential>
{
private:
  using XVector = typename Differential::XVector;

    // required because this a template function
    using Solver<Differential>::odeSeries;
    using Solver<Differential>::tBegin;
    using Solver<Differential>::dt;
    using Solver<Differential>::nSteps;
    using Solver<Differential>::ode;

 public:

    Euler<Differential>():Solver<Differential>() {
        this->noiseShape = ODETypes::NOISE_NONE;
        this->order = 1;
    }
    virtual ~Euler() {}

    void solve(typename Differential::Param parameters) {
        // Maybe this should be adapted to interpolate between two time steps, to allow
        // propagation backward in time

      ptrdiff_t i;

      assert(checkInitialized());

      Differential dX(odeSeries);
      dX.setParameters(parameters);

      double t = tBegin;
      XVector x = ode.x0;
      odeSeries.line_of_data(t, x);

      for(i=0; i < nSteps - 1; ++i) {
          x += dX.f(t, x) * dt;
          t += dt;
          odeSeries.line_of_data(t, x);
      }
    }

    /* Basic sanity check for initial conditions
     * Returns false if one of the initialization values is clearly improperly set
     */
    bool checkInitialized() {
      bool initialized = true;
      if (this->tEnd == this->tBegin or this->dt == 0 or this->nSteps == 0) {initialized = false;}
      return initialized;
    }
  };


}

#endif // EULER_H
