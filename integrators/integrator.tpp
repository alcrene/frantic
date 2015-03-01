#ifndef INTEGRATOR_TPP
#define INTEGRATOR_TPP

/* --------------------------------------------------------------------------
 * Integrator class
 * --------------------------------------------------------------------------*/


/* Return an std::vector with the discretized time steps
 * If stepMultiplier is specified, insert the given number of steps between each step used in the solving algorithm.
 * \todo: deprecate in favour of Series method
 */
template <class Differential> vector<double> Integrator<Differential>::getTSeriesVector(int stepMultiplier){

    if ((stepMultiplier == 0) or (stepMultiplier == 1)) {
    return history["t"];
    } else if (stepMultiplier > 1) {
        // We want to interpolate points between the time steps
        // UNTESTED !!!
        // double subStepSize = dt/stepMultiplier;
        vector<double> subSeries_t;
        subSeries_t.reserve(stepMultiplier * this->history.nSteps);
        for (int i=0; i<history.get_nlines() - 1; ++i) {
            for (int j=0; j<stepMultiplier; ++j) {
                subSeries_t.push_back(history.get(0, i) * (1 - j/stepMultiplier) + history.get(0, i+1) * j / stepMultiplier);
            }
        }
        subSeries_t.push_back(history.get(0, history.get_nlines()-1));  // Add the last element separately, because we don't look beyond it
        return subSeries_t;
    //} else {
        // We want to reduce the number of points by a factor of stepMultiplier
        // TODO
    }
}

/* Return an std::vector with the time series of the specified X component
   0 specifies the first component
   \todo: deprecate in favour of Series method
 */
template <class Differential> vector<double> Integrator<Differential>::getXHistoryVector(ptrdiff_t component) {
    return history[component + 1];
}

/* Evaluate a function over the integrator's timesteps
 * TODO: use getTSeries to allow eval over a finer set of points
 */
template <class Differential> Series<typename Differential::XVector> Integrator<Differential>::evalFunction(std::function<XVector(const double&)> f) {
  Series<double> result(history.get_nlines());

	for(auto t_iter=history[0].begin(); t_iter != history.end(); ++t_iter) {
	result.line_of_data(*t_iter, f(*t_iter));
  }
  
  return result;
}

/* Return an std::vector for the time series of a function, evaluated at the same points as the solving algorithm
 * TODO: use getTSeries to allow eval over a finer set of points
 */
template <class Differential> vector<double> Integrator<Differential>::evalFunctionComponent(ptrdiff_t component, std::function<XVector(const double&)> f) {
  std::vector<XVector, Eigen::aligned_allocator<XVector> > result;

  result.reserve(history.size());
  for(auto t_iter=history[0].begin(); t_iter != history.end(); ++t_iter) {
        result.push_back(f(*t_iter));
  }

  return result;
}

/* Reset the state data. In particular, this clears the series table
 */
template <class Differential> void
Integrator<Differential>::reset() {
  history.reset();
}

/* Dump all or part of the vectors to cout. Designed for debugging
   TODO: - if " 'x' in vars " type parameter
         - allow multiple vector components
         - allow partial row dump (rows n to m; step k)
 */
template <class Differential> void Integrator<Differential>::dump(std::string cmpntName)
{
  //  ptrdiff_t i = component;
  std::cout << cmpntName << ": " << std::endl;;
  for (size_t i=0; i < history.get_nlines(); ++i) {
        std::cout << "      " << history.get(cmpntName, i) << std::endl;
  }
  std::cout << std::endl;
}

/* Propagate the ODE through time to produce the solution vector x(t) (named series_x)
 *
 * This function should always be overloaded by the specific integrator class
 */
template <class Differential> void Integrator<Differential>::integrate(Differential dX) {
  ptrdiff_t i;

  for(i=1; i < this->history.nSteps; ++i) {
    history.update(this->history.t0 + i * this->history.dt, XVector::Constant(this->history.get(0, i)));
  }
}

#endif
