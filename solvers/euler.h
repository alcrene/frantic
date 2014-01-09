/* Basic ODE solver using an Euler-scheme
 */

#ifndef EULER_H
#define EULER_H

#include "solver.h"

using std::vector;

namespace solvers {

template <typename ODEdef, typename XVector>
class Euler : public Solver<ODEdef, XVector>
{
public:
    Euler<ODEdef, XVector>() {
        this->noiseShape = ODETypes::NOISE_NONE;
    }
    virtual ~Euler() {}
    void solve();

private:
    // required because this a template function
    using Solver<ODEdef, XVector>::odeSeries;
    using Solver<ODEdef, XVector>::tBegin;
    using Solver<ODEdef, XVector>::tStepSize;
    using Solver<ODEdef, XVector>::tNumSteps;
    using Solver<ODEdef, XVector>::ode;
};

#include "euler.tpp"

}

#endif // EULER_H
