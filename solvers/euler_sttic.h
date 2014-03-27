#ifndef EULER_STTIC_H
#define EULER_STTIC_H

#include "solver.h"

using std::vector;

namespace solvers {

  template <typename ODEdef, typename XVector, typename XSeries>
  class Euler_sttic : public Solver<ODEdef, XVector, XSeries>
  {
  public:
      Euler_sttic() {
          this->noiseShape = ODETypes::NOISE_VECTOR;
      }
      virtual ~euler_sttic() {}

      void setStochasticGenerator(StticGen* NumberGeneratorPointer) {
          nbrGen = NumberGeneratorPointer;
      }

      void solve(Param parameters) {
          // Maybe this should be adapted to interpolate between two series_t elements, to allow
          // propagation backward in time (or even maybe uneven timesteps ?)

        ptrdiff_t i;

        typename ODEdef::func_dX dX(this->odeSeries);
        dX.setParameters(parameters);

        double t = this->tBegin;
        XVector x = this->ode.x0;
        this->odeSeries.line_of_data(t,x);

        for(i=0; i < this->nSteps - 1; ++i) {
      //      XVector test = dX.g(series_x[i], i*tStepSize);
            // \todo: Check if we should specify Eigen matrix multiplication
            x += dX.f(t, x) * this->dt + dX.g(t, x) * dX.dS(dt);
            t += dt;
            this->odeSeries.line_of_data(t, x);
        }

      }

  };

}

#endif // EULER_STTIC_H
