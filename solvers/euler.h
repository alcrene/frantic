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
public:

    Euler<ODEdef, XVector, XSeries>(ODEdef& ode):Solver<ODEdef, XVector, XSeries>(ode) {
        this->noiseShape = ODETypes::NOISE_NONE;
        this->order = 1;
    }
    virtual ~Euler() {}
    void solve(Param parameters);

private:
    // required because this a template function
    using Solver<ODEdef, XVector, XSeries>::odeSeries;
    using Solver<ODEdef, XVector, XSeries>::tBegin;
    using Solver<ODEdef, XVector, XSeries>::dt;
    using Solver<ODEdef, XVector, XSeries>::tNumSteps;
    using Solver<ODEdef, XVector, XSeries>::ode;
};

#include "euler.tpp"

}

#endif // EULER_H
