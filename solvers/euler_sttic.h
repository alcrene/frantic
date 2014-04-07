#ifndef EULER_STTIC_H
#define EULER_STTIC_H

#include "solver.h"

using std::vector;

namespace solvers {

  template <typename ODEdef, typename XVector, typename XSeries>
  class Euler_sttic : public Solver<ODEdef, XVector, XSeries>
  {
  public:

    Euler_sttic<ODEdef, XVector, XSeries>(ODEdef& ode) : Solver<ODEdef, XVector, XSeries>(ode) {
          this->noiseShape = ODETypes::NOISE_VECTOR;
        this->order = 0.5;
      }
      virtual ~Euler_sttic() {}

      void solve(Param parameters) {
          // Maybe this should be adapted to interpolate between two series_t elements, to allow
          // propagation backward in time (or even maybe uneven timesteps ?)

        ptrdiff_t i;

        assert(checkInitialized());

        typename ODEdef::func_dX dX(this->odeSeries, this->ode);
        dX.setParameters(parameters);

        double t = this->tBegin;
        XVector x = this->ode.x0;
        this->odeSeries.line_of_data(t,x);

        for(i=0; i < this->nSteps - 1; ++i) {
      //      XVector test = dX.g(series_x[i], i*tStepSize);
            // \todo: Check if we should specify Eigen matrix multiplication
            x += dX.f(t, x) * this->dt + dX.g(t, x) * dX.dS(this->dt);
            t += this->dt;
            this->odeSeries.line_of_data(t, x);
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

#endif // EULER_STTIC_H
