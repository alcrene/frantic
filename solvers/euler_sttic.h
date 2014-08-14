#ifndef EULER_STTIC_H
#define EULER_STTIC_H

#include "solver.h"

using std::vector;

namespace solvers {

  template <class Differential>
  class Euler_sttic : public Solver<Differential>
  {
  private:
    using XVector = typename Differential::XVector;

  public:

    using Solver<Differential>::Solver;  // Allow parent class overloads
    Euler_sttic<Differential>() : Solver<Differential>() {
      this->noiseShape = ODETypes::NOISE_VECTOR;
      this->order = 0.5;
    }
    virtual ~Euler_sttic() {}

    /* Overloaded solve function which initializes creates a local dX total derivative from the specified parameters
       **** BUGY **** : x0 is not set this way, which will almost always be a problem
     */
    void solve(const cent::ParameterMap& parameters) {
      std::cerr << "This call to 'solve' is buggy. Please use another or, if you need this one, fix it." << std::endl;
      solve(Differential(this->odeSeries, parameters));
    }

    /* Given a total derivative functor dX, solve the DE problem.
     * To have the function create a functor from parameters, use the alternate constructor
     */
    void solve(const Differential& dX) {
      // Maybe this should be adapted to interpolate between two series_t elements, to allow
      // propagation backward in time (or even maybe uneven timesteps ?)

      ptrdiff_t i;

      assert(checkInitialized());

      double t = this->tBegin;
      XVector x = dX.x0;
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
