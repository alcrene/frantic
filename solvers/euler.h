/* Basic ODE solver using an Euler-scheme
 */

#ifndef EULER_H
#define EULER_H

#include "solver.h"

using std::vector;

namespace solvers {

template <typename ODEdef, typename XVector, typename XSeries>
class Euler : public Solver<ODEdef, XVector, XSeries>
{
private:
    // required because this a template function
    using Solver<ODEdef, XVector, XSeries>::odeSeries;
    using Solver<ODEdef, XVector, XSeries>::tBegin;
    using Solver<ODEdef, XVector, XSeries>::dt;
    using Solver<ODEdef, XVector, XSeries>::nSteps;
    using Solver<ODEdef, XVector, XSeries>::ode;

 public:

    Euler<ODEdef, XVector, XSeries>(ODEdef& ode):Solver<ODEdef, XVector, XSeries>(ode) {
        this->noiseShape = ODETypes::NOISE_NONE;
        this->order = 1;
    }
    virtual ~Euler() {}

    void solve(Param parameters) {
        // Maybe this should be adapted to interpolate between two time steps, to allow
        // propagation backward in time

      ptrdiff_t i;


      typename ODEdef::func_dX dX(odeSeries);
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
  };


}

#endif // EULER_H
