/* Basic ODE integrator using an Euler-scheme
 */

#ifndef EULER_H
#define EULER_H

#include "integrator.h"

using std::vector;

namespace integrators {

template <class Differential>
class Euler : public frantic::Integrator<Differential>
{
private:
  using XVector = typename Differential::XVector;

  // required because this a template function
  using Integrator<Differential>::history;
  using Integrator<Differential>::ode;

public:

  Euler<Differential>() : frantic::Integrator<Differential>() {
    this->noiseShape = ODETypes::NOISE_NONE;
    this->order = 1;
  }
  virtual ~Euler() {}

  void integrate(typename Differential::Param parameters) {
    // Maybe this should be adapted to interpolate between two time steps, to allow
    // propagation backward in time

    ptrdiff_t i;

    assert(this->history.check_initialized());

    Differential dX(odeSeries);
    dX.setParameters(parameters);

    double t = history.t0;
    XVector x = history(t);
    //odeSeries.line_of_data(t, x);

    for(i=0; i < nSteps - 1; ++i) {
      x += dX.f(t, x) * dt;
      t += dt;
      history.update(t, x);
    }
  }

};


}

#endif // EULER_H
