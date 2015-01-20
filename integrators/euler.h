/* Basic ODE integrator using an Euler-scheme
 */

#ifndef EULER_H
#define EULER_H

#include "integrator.h"

using std::vector;

namespace integrators {

template <class Differential>
class Euler : public Integrator<Differential>
{
private:
  using XVector = typename Differential::XVector;

    // required because this a template function
    using Integrator<Differential>::odeSeries;
    using Integrator<Differential>::tBegin;
    using Integrator<Differential>::dt;
    using Integrator<Differential>::nSteps;
    using Integrator<Differential>::ode;

 public:

    Euler<Differential>():Integrator<Differential>() {
        this->noiseShape = ODETypes::NOISE_NONE;
        this->order = 1;
    }
    virtual ~Euler() {}

    void integrate(typename Differential::Param parameters) {
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
