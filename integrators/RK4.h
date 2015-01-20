/* 4th order Runge-Kutta ODE solver
 */

#ifndef RK4_H
#define RK4_H

#include "solver.h"

using std::vector;

namespace solvers {

template <typename Functor, typename XVector>
class RK4 : public Solver<Functor, XVector>
{
public:
    RK4<Functor, XVector>() {}
    virtual ~RK4() {}
    void solve();

private:
    ODETypes::NOISE_SHAPE noiseShape = ODETypes::NOISE_NONE;

    // required because this a template function
    using Solver<Functor, XVector>::series_t;
    using Solver<Functor, XVector>::series_x;
    using Solver<Functor, XVector>::tStepSize;
    using Solver<Functor, XVector>::tNumSteps;
    using Solver<Functor, XVector>::ode;
};

#include "RK4.tpp"

}

#endif // RK4_H
