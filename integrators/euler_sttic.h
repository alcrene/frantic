#ifndef EULER_STTIC_H
#define EULER_STTIC_H

#include "integrator.h"

using std::vector;

namespace integrators {

  template <class Differential>
  class Euler_sttic : public Integrator<Differential>
  {
  private:
    using XVector = typename Differential::XVector;

  public:

    using Integrator<Differential>::Integrator;  // Allow parent class overloads
    Euler_sttic<Differential>() : Integrator<Differential>() {
      this->order = 0.5;
    }
    virtual ~Euler_sttic() {}

    /* Overloaded integrate function which initializes creates a local dX total derivative from the specified parameters
       **** BUGY **** : x0 is not set this way, which will almost always be a problem
     */
    void integrate(const frantic::ParameterMap& parameters) {
      std::cerr << "This call to 'integrate' is buggy. Please use another or, if you need this one, fix it." << std::endl;
      integrate(Differential(this->odeSeries, parameters));
    }

    /* Given a total derivative functor dX, integrate the DE problem.
     * To have the function create a functor from parameters, use the alternate constructor
     */
    void integrate(const Differential& dX) {
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
        // \todo: Why is t incremented before saving data ?
        x += dX.drift(t, x) * this->dt
            + dX.diffusion_coeffs(t).sum_products(dX.diffusion_differentials(t,this->dt));
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
