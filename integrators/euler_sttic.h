#ifndef EULER_STTIC_H
#define EULER_STTIC_H

#include "integrator.h"

using std::vector;

namespace integrators {

  /* An Euler-Maruyama integrator for stochastic differential equations
   * (weak order: 1, strong order : 1/2)
   * The required signature for the update function is
   * void update(double t, double x)
   * where t is current time and x current state.
   * \todo: test benefit of saving a reference to this->history
   */
  template <class Differential>
  class Euler_sttic : public frantic::Integrator<Differential>
  {
  private:
    using XVector = typename Differential::XVector;

  public:

    using frantic::Integrator<Differential>::Integrator;  // Allow parent class overloads
    Euler_sttic<Differential>() : frantic::Integrator<Differential>() {
      this->order = 0.5;
    }
    virtual ~Euler_sttic() {}

    /* Overloaded integrate function which initializes creates a local dX total derivative from the specified parameters
       **** BUGY **** : x0 is not set this way, which will almost always be a problem
     */
//    void integrate(const frantic::ParameterMap& parameters) {
//      std::cerr << "This call to 'integrate' is buggy. Please use another or, if you need this one, fix it." << std::endl;
//      integrate(Differential(parameters));
//    }

    /* Given a total derivative functor dX, integrate the DE problem.
     * To have the function create a functor from parameters, use the alternate constructor
     */
    void integrate(const Differential& dX) {
      // Maybe this should be adapted to interpolate between two series_t elements, to allow
      // propagation backward in time (or even maybe uneven timesteps ?)

      ptrdiff_t i;

      assert(this->history.check_initialized());


      double t = this->history.t0;
      XVector x = this->history(t);

      for(i=0; i < this->history.nSteps - 1; ++i) {
        //      XVector test = dX.g(series_x[i], i*tStepSize);
        // \todo: Check if we should specify Eigen matrix multiplication
        // \todo: Why is t incremented before saving data ?
        x += dX.drift(t, x, this->history) * this->history.dt
            + dX.diffusion_coeffs(t, x, this->history).sum_products(dX.diffusion_differentials(this->history.dt));
        t += this->history.dt;
        this->history.update(t, x);
      }

    }

  };

}

#endif // EULER_STTIC_H
