/* 4th order Runge-Kutta ODE solver
 */

#ifndef EULER_H
#define EULER_H

#include "solver.h"

using std::vector;

namespace solvers {

template <typename Functor, typename XVector>
class euler : public Solver<Functor, XVector>
{
public:
    euler<Functor, XVector>(odeDef* ODE):Solver<Functor, XVector>(ODE) {}
    virtual ~euler() {}
    void solve();

private:
    // required because this a template function
    using Solver<Functor, XVector>::series_t;
    using Solver<Functor, XVector>::series_x;
    using Solver<Functor, XVector>::tStepSize;
    using Solver<Functor, XVector>::tNumSteps;
    using Solver<Functor, XVector>::ode;
};

#include "euler.tpp"

}

#endif // EULER_H
