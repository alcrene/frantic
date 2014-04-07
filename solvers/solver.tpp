#ifndef SOLVER_TPP
#define SOLVER_TPP

/* --------------------------------------------------------------------------
 * Solver class
 * --------------------------------------------------------------------------*/


/* Return an std::vector with the discretized time steps
 * If stepMultiplier is specified, insert the given number of steps between each step used in the solving algorithm.
 */
template <class ODEdef, class XVector, class XSeries> vector<double> Solver<ODEdef, XVector, XSeries>::getTSeriesVector(int stepMultiplier){

    if ((stepMultiplier == 0) or (stepMultiplier == 1)) {
	  return odeSeries["t"];
    } else if (stepMultiplier > 1) {
        // We want to interpolate points between the time steps
        // UNTESTED !!!
        // double subStepSize = dt/stepMultiplier;
        vector<double> subSeries_t;
        subSeries_t.reserve(stepMultiplier * nSteps);
        for (int i=0; i<odeSeries.get_nlines() - 1; ++i) {
            for (int j=0; j<stepMultiplier; ++j) {
                subSeries_t.push_back(odeSeries.get(0, i) * (1 - j/stepMultiplier) + odeSeries.get(0, i+1) * j / stepMultiplier);
            }
        }
        subSeries_t.push_back(odeSeries.get(0, odeSeries.get_nlines()-1));  // Add the last element separately, because we don't look beyond it
        return subSeries_t;
    //} else {
        // We want to reduce the number of points by a factor of stepMultiplier
        // TODO
    }
}

/* Return an std::vector with the time series of the specified X component
   0 specifies the first component
 */
template <class ODEdef, class XVector, class XSeries> vector<double> Solver<ODEdef, XVector, XSeries>::getXSeriesVector(ptrdiff_t component) {
    return odeSeries[component + 1];
}

/* Return an std::vector with the time series of the given x timeseries
   Obsolete ?
 */
/*template <class ODEdef, class XVector, class XSeries> vector<double> Solver<ODEdef, XVector, XSeries>::getXSeriesVector(ptrdiff_t component, XSeries xseries) {
    vector<double> vect;

    vect.reserve(xseries.size());
    for(size_t i=0;i < nSteps; ++i) {
        vect.push_back(xseries[i](component));
    }
    return vect;
	}*/

/* Evaluate a function over the solver's timesteps
 * TODO: use getTSeries to allow eval over a finer set of points
 */
template <class ODEdef, class XVector, class XSeries> Series<XVector> Solver<ODEdef, XVector, XSeries>::evalFunction(std::function<XVector(const double&)> f) {
  Series<double> result(odeSeries.get_nlines());

  for(auto t_iter=odeSeries[0].begin(); t_iter != odeSeries.end(); ++t_iter) {
	result.line_of_data(*t_iter, f(*t_iter));
  }
  
  return result;
}

/* Return an std::vector for the time series of a function, evaluated at the same points as the solving algorithm
 * TODO: use getTSeries to allow eval over a finer set of points
 */
template <class ODEdef, class XVector, class XSeries> vector<double> Solver<ODEdef, XVector, XSeries>::evalFunctionComponent(ptrdiff_t component, std::function<XVector(const double&)> f) {
  std::vector<XVector, Eigen::aligned_allocator<XVector> > result;

  result.reserve(odeSeries.size());
  for(auto t_iter=odeSeries[0].begin(); t_iter != odeSeries.end(); ++t_iter) {
        result.push_back(f(*t_iter));
  }

  return result;
}

/* Reset the state data. In particular, this clears the series table
 */
template <class ODEdef, class XVector, class XSeries> void
Solver<ODEdef, XVector, XSeries>::reset() {
  tBegin = 0;
  tEnd = 0;
  dt = 0;
  nSteps = 0;
  //initConditionsSet = false;  // Better to use algorithm-specific checkers: some might not need same conditions


  //odeSeries = XSeries(this->order, "x");
  //odeSeriesError = Series<XVector>("xerr");   // Using copy assignment turns out more complicated than just writing a proper reset function
  odeSeries.reset();
  odeSeriesError.reset();

}

/* Reserve memory for the result series table; 
 *   for adaptive time step algorithms, this is only an estimation.
 * Set begin, end and step size; the number of steps is calculated
 * If stepSize does not fit an integer number of times in the interval,
 *   it is reduced slightly.
 * Calculated number of steps is multiplied by 'growFactor', allowing to reserve
 *   extra memory; useful if it is known that an adaptive stepper will add steps
 */
template <class ODEdef, class XVector, class XSeries> void
Solver<ODEdef, XVector, XSeries>::setRange(double begin, double end,
                                   double stepSize, double growFactor) {
  double remainder;

  assert((end != begin) and (stepSize != 0));
  assert((end - begin) * stepSize > 0);

  tBegin = begin;
  tEnd = end;
  dt = stepSize;
  nSteps = std::floor( std::abs(tEnd - tBegin) / dt );
  remainder = abs(tEnd - tBegin) - nSteps*dt;
  if (remainder != 0) {
    nSteps = nSteps + 1;
    dt = (tEnd - tBegin) / nSteps;
  }

  long minlines=(nSteps + 1) * growFactor; // +1 for the initial condition (which is not a step)
  if (odeSeries.get_maxlines() < minlines) {
      odeSeries.inc_maxlines(minlines - odeSeries.get_maxlines());  // inc_maxlines(n) appends n lines to the existing ones
    }
  setOdeDoneCondition();
}

/* Set the time range over which to propagate.
 * Set begin, end and number of steps; the step size is calculated
 * Of course, for a solver with adaptive step size, this is an estimation
 * Calculated number of steps is multiplied by 'growFactor', allowing to reserve
 *   extra memory; useful if it is known that an adaptive stepper will add steps
 */
template <class ODEdef, class XVector, class XSeries>
void Solver<ODEdef, XVector, XSeries>::setRange(double begin, double end,
                                       int numSteps, double growFactor) {
  assert((end != begin) and (numSteps != 0));

  tBegin = begin;
  tEnd = end;
  nSteps = numSteps;
  dt = (tEnd - tBegin) / nSteps;

  odeSeries.inc_maxlines((nSteps + 1) * growFactor);
  // +1 for the initial condition (which is not a step)

  setOdeDoneCondition();
}

/* Prepare the result vectors series_t and series_x:
 * TODO: this seems deprecated
 *  - sanity check on stepping bounds
 *  - fill independent column of ODE series with the time step values
 */
/*template <class ODEdef, class XVector, class XSeries> void Solver<ODEdef, XVector, XSeries>::discretize() {
  ptrdiff_t i;

  // Make sure the time range is properly initialized
  assert((dt != 0) and (nSteps != 0));
  assert((tEnd - tBegin)/dt > 0); // We know that dt != 0
  assert(nSteps <= odeSeries.get_maxlines());


  //series_t.reserve(nSteps);
  //series_x.resize(nSteps);  // Calls default constructor for each XVector


  for(i=0; i<nSteps; ++i) {
    odeSeries.set(0, i, tBegin + i*dt);
  }
}*/

/* Dump all or part of the vectors to cout. Designed for debugging
   TODO: - if " 'x' in vars " type parameter
         - allow multiple vector components
         - allow partial row dump (rows n to m; step k)
 */
template <class ODEdef, class XVector, class XSeries> void Solver<ODEdef, XVector, XSeries>::dump(std::string cmpntName)
{
  //  ptrdiff_t i = component;
  std::cout << cmpntName << ": " << std::endl;;
  for (size_t i=0; i < odeSeries.get_nlines(); ++i) {
        std::cout << "      " << odeSeries.get(cmpntName, i) << std::endl;
  }
  std::cout << std::endl;
}

/* Propagate the ODE through time to produce the solution vector x(t) (named series_x)
 *
 * This function should always be overloaded by the specific solver class
 */
template <class ODEdef, class XVector, class XSeries> void Solver<ODEdef, XVector, XSeries>::solve(Param parameters) {
  ptrdiff_t i;

  // Discretize the continuous given range
  //discretize();

  //assert(dX != NULL); // Make sure the function has been defined
  //  assert(initConditionsSet); // In a real solver we should check this
  for(i=1; i<nSteps; ++i) {
    //fill(series_x[i].begin(), series_x[i].end(), series_t[i]);
    odeSeries.line_of_data(tBegin + i*dt, XVector::Constant(odeSeries.get(0, i)));
  }
}

#endif
