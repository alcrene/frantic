#ifndef EULER_STTIC_H
#define EULER_STTIC_H

#include "CLHEP/Random/TripleRand.h"
#include "CLHEP/Random/RandGaussT.h"

#include "solver.h"
//#include "stochastic.h"

using std::vector;

namespace solvers {

template <typename Functor, typename XVector, typename StticGen>
class euler_sttic : public Solver<Functor, XVector>
{
public:
    euler_sttic() {
        this->noiseShape = ODETypes::NOISE_VECTOR;
    }
    virtual ~euler_sttic() {}

    void setStochasticGenerator(StticGen* NumberGeneratorPointer) {
        nbrGen = NumberGeneratorPointer;
    }

    void solve();



private:
    StticGen* nbrGen;

    // required because this a template function
    using Solver<Functor, XVector>::series_t;
    using Solver<Functor, XVector>::series_x;
    using Solver<Functor, XVector>::tStepSize;
    using Solver<Functor, XVector>::tNumSteps;
    using Solver<Functor, XVector>::ode;
    using Solver<Functor, XVector>::dX;
};

#include "euler_sttic.tpp"

}

#endif // EULER_STTIC_H
