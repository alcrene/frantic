#ifndef SOLVER_TPP
#define SOLVER_TPP

//#include "solver.h"


/* Return an std::vector with the discretized time steps
 * If stepMultiplier is specified, insert the given number of steps between each step used in the solving algorithm.
 */
template <class Functor, class XVector> vector<double> Solver<Functor, XVector>::getTSeriesVector(int stepMultiplier){

    if ((stepMultiplier == 0) or (stepMultiplier == 1)) {
        return series_t;
    } else {
        double subStepSize = tStepSize/stepMultiplier;
        vector<double> subSeries_t;
        subSeries_t.reserve(stepMultiplier * tNumSteps);
        for (int i=0; i<tNumSteps; ++i) {
            for (int j=0; j<tNumSteps; ++j) {
                subSeries_t.push_back(series_t[i] + j * subStepSize);
            }
        }
        return subSeries_t;
    }
}

/* Return an std::vector with the time series of the specified X component
 */
template <class Functor, class XVector> vector<double> Solver<Functor, XVector>::getXSeriesVector(ptrdiff_t component) {
    return getXSeriesVector(component, series_x);
}

/* Return an std::vector with the time series of the given x timeseries
 */
template <class Functor, class XVector> vector<double> Solver<Functor, XVector>::getXSeriesVector(ptrdiff_t component, TXSeries xseries) {
    vector<double> vect;

    vect.reserve(xseries.size());
    for(size_t i=0;i < tNumSteps; ++i) {
        vect.push_back(xseries[i](component));
    }
    return vect;
}

/* Evaluate a function over the solver's timesteps
 */
template <class Functor, class XVector> vector<XVector, aligned_allocator<XVector> > Solver<Functor, XVector>::evalFunction(std::function<XVector(const double&)> f) {
    TXSeries result;

    result.reserve(series_t.size());
    for(auto t_iter=series_t.begin(); t_iter != series_t.end(); ++t_iter) {
        result.push_back(f(*t_iter));
    }

    return result;
}

/* Return an std::vector for the time series of a function, evaluated at the same points as the solving algorithm
 */
template <class Functor, class XVector> vector<double> Solver<Functor, XVector>::evalFunctionComponent(ptrdiff_t component, std::function<XVector(const double&)> f) {
    return getXSeriesVector(component, evalFunction(f));
}


/* Set the time range over which to propagate.
 * Set begin, end and step size; the number of steps is calculated
 * If stepSize does not fit an integer number of times in the interval,
 *   it is reduced slightly.
 */
template <class Functor, class XVector> void Solver<Functor, XVector>::setRange(double begin, double end, double stepSize) {
  double remainder;

  assert((end != begin) and (stepSize != 0));

  tBegin = begin;
  tEnd = end;
  tStepSize = stepSize;
  tNumSteps = (int) (tBegin - tEnd) / tStepSize;
  remainder = (tBegin - tEnd) - tNumSteps*tStepSize;
  if (remainder != 0) {
    tNumSteps = tNumSteps + 1;
    tStepSize = (tBegin - tEnd) / tNumSteps;
  }
}

/* Set the time range over which to propagate.
 * Set begin, end and number of steps; the step size is calculated
 */
template <class Functor, class XVector> void Solver<Functor, XVector>::setRange(double begin, double end, int numSteps) {
  assert((end != begin) and (numSteps != 0));

  tBegin = begin;
  tEnd = end;
  tNumSteps = numSteps;
  tStepSize = (tEnd - tBegin) / tNumSteps;
}

/* Prepare the result vectors series_t and series_x:
 *  - redimension them to the number of steps
 *  - fill series_t with the time step values
 */
template <class Functor, class XVector> void Solver<Functor, XVector>::discretize() {
  ptrdiff_t i;

  // Make sure the time range is properly initialized
  assert((tStepSize != 0) and (tNumSteps != 0));
  assert((tEnd - tBegin)/tStepSize > 0); // We know that tStepSize != 0


  series_t.reserve(tNumSteps);
  series_x.resize(tNumSteps);  // Calls default constructor for each XVector

  for(i=0; i<tNumSteps; ++i) {
    series_t.push_back(tBegin + i*tStepSize);
  }
}

/* Dump all or part of the vectors to cout. Designed for debugging
   TODO: - if " 'x' in vars " type parameter
         - allow multiple vector components
         - allow partial row dump (rows n to m; step k)
 */
template <class Functor, class XVector> void Solver<Functor, XVector>::dump(std::string vars, ptrdiff_t component)
{
  ptrdiff_t i = component;
  if (vars == "x") {
      std::cout << "x" << i << ": ";
      for (auto iter = series_x.begin(); iter != series_x.end(); ++iter) {
          std::cout << "      " << iter[i] << endl;
      }
      std::cout << endl;
  }
}

/* Propagate the ODE through time to produce the solution vector x(t) (named series_x)
 *
 * This function should always be overloaded by solver class
 */
template <class Functor, class XVector> void Solver<Functor, XVector>::solve() {
  ptrdiff_t i;

  // Discretize the continuous given range
  discretize();

  //assert(dX != NULL); // Make sure the function has been defined
  //  assert(initConditionsSet); // In a real solver we should check this
  for(i=1; i<tNumSteps; ++i) {
    //fill(series_x[i].begin(), series_x[i].end(), series_t[i]);
    series_x[i] = XVector::Constant(series_t[i]);
  }
}

#endif
