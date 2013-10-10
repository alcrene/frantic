/* An ODE solver for testing
 * Inputs and outputs correspond to those of a solver, but the returned vector
 * is simply a discretization of the input function
 */

#ifndef DUMMY_H
#define DUMMY_H

#include "solver.h"

using std::vector;

namespace solvers {

  template <typename Functor, typename XVector>
    class Dummy : public Solver<Functor, XVector>
	{
	public:
    // This constructor is required; call does not automatically resolve to the templated parent constructor
    Dummy<Functor, XVector>() {
        std::cout << "Creating 'Dummy'" << std::endl;
    }
      virtual ~Dummy(){
        std::cerr << "Deleting 'Dummy'" << std::endl;
      }

	  void solve();


	private:
      // required because this a template function
      using Solver<Functor, XVector>::series_t;
      using Solver<Functor, XVector>::series_x;
      using Solver<Functor, XVector>::tStepSize;
      using Solver<Functor, XVector>::tNumSteps;
      using Solver<Functor, XVector>::ode;
	};

#include "dummy.tpp"

}

#endif // DUMMY_H
