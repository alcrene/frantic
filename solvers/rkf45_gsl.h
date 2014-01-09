/* Runge-Kutta-Fehlberg embedded Runge-Kutta ODE for error control
 */

#ifndef RKF45_GSL_H
#define RKF45_GSL_H

#include "solver.h"

using std::vector;

namespace solvers {

template <typename Functor, typename XVector>
class RKF45_gsl : public Solver<Functor, XVector>
{
public:
    RKF45_gsl<Functor, XVector>() {}
    virtual ~RKF45_gsl() {}
    void solve();

private:
    ODETypes::NOISE_SHAPE noiseShape = ODETypes::NOISE_NONE;

    // required because this a template function
    using Solver<Functor, XVector>::series_t;
    using Solver<Functor, XVector>::series_x;
    using Solver<Functor, XVector>::dt;
    using Solver<Functor, XVector>::tNumSteps;
    using Solver<Functor, XVector>::ode;
};

#include "rkf45_gsl.tpp"

}

#endif // RKF45_GSL_H
