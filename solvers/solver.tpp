#ifndef SOLVER_TPP
#define SOLVER_TPP

//#include "solver.h"

template <class XVector> Series<XVector>::Series(std::string varname, size_t cmaxlines):
  o2scl::table<std::vector<double> >(cmaxlines) {
  std::string rowstr;
  rowstr = "t";
  for(size_t i=1; i<=XVector::SizeAtCompileTime; ++i) {
        rowstr += " " + varname + std::to_string(i);
  }
  this->line_of_names(rowstr);
}

// Overloaded data adding function to allow using the XVector type
template <class XVector> void Series<XVector>::line_of_data(double t, XVector x) {
  // Virtually a copy of void line_of_data() from o2scl/table.h
  if (maxlines==0) inc_maxlines(5);
  if (nlines>=maxlines) inc_maxlines(maxlines);
      
  if (intp_set) {
	intp_set=false;
	delete si;
  }
      
  if (nlines<maxlines && XVector::SizeAtCompileTime<=(atree.size())) {

	set_nlines(nlines+1);
	this->set(0, nlines-1, t);
	for(size_t i=0; i<XVector::SizeAtCompileTime; ++i) {
	  this->set(i+1, nlines-1, x(i));
	}
	
	return;
  }
      
//  O2SCL_ERR("Not enough lines or columns in line_of_data().",exc_einval);
  return;
}

template <class XVector> typename Series<XVector>::Statistics Series<XVector>::getStatistics() {

  Statistics stats;

  for(size_t i = 1; i <  this->get_ncolumns(); ++i) {
      stats.max.push_back(this->max(i));
      stats.min.push_back(this->min(i));
      double sum = std::accumulate((*this)[i].begin(), (*this)[i].begin() + this->get_nlines(), 0.0);
      stats.mean.push_back(sum/this->get_nlines());
  }

  stats.nsteps = this->get_nlines();

  return stats;
}

template <class XVector> XVector Series<XVector>::getVectorAtTime(const size_t t_idx) const {
  static XVector retval;
  for(size_t i=0; i<XVector::SizeAtCompileTime; ++i) {
      retval(i) = get(i+1, t_idx);
  };
  return retval;
}

// Convenience overloads
template <class XVector> double Series<XVector>::max(size_t icol) {
  return this->max(this->get_column_name(icol));
}

template <class XVector> double Series<XVector>::min(size_t icol) {
  return this->min(this->get_column_name(icol));
}



/* Return an std::vector with the discretized time steps
 * If stepMultiplier is specified, insert the given number of steps between each step used in the solving algorithm.
 */
template <class ODEdef, class XVector> vector<double> Solver<ODEdef, XVector>::getTSeriesVector(int stepMultiplier){

    if ((stepMultiplier == 0) or (stepMultiplier == 1)) {
	  return odeSeries["t"];
    } else if (stepMultiplier > 1) {
        // We want to interpolate points between the time steps
        // UNTESTED !!!
        // double subStepSize = dt/stepMultiplier;
        vector<double> subSeries_t;
        subSeries_t.reserve(stepMultiplier * tNumSteps);
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
template <class ODEdef, class XVector> vector<double> Solver<ODEdef, XVector>::getXSeriesVector(ptrdiff_t component) {
    return odeSeries[component + 1];
}

/* Return an std::vector with the time series of the given x timeseries
   Obsolete ?
 */
/*template <class ODEdef, class XVector> vector<double> Solver<ODEdef, XVector>::getXSeriesVector(ptrdiff_t component, TXSeries xseries) {
    vector<double> vect;

    vect.reserve(xseries.size());
    for(size_t i=0;i < tNumSteps; ++i) {
        vect.push_back(xseries[i](component));
    }
    return vect;
	}*/

/* Evaluate a function over the solver's timesteps
 * TODO: use getTSeries to allow eval over a finer set of points
 */
template <class ODEdef, class XVector> Series<XVector> Solver<ODEdef, XVector>::evalFunction(std::function<XVector(const double&)> f) {
  Series<double> result(odeSeries.get_nlines());

  for(auto t_iter=odeSeries[0].begin(); t_iter != odeSeries.end(); ++t_iter) {
	result.line_of_data(*t_iter, f(*t_iter));
  }
  
  return result;
}

/* Return an std::vector for the time series of a function, evaluated at the same points as the solving algorithm
 * TODO: use getTSeries to allow eval over a finer set of points
 */
template <class ODEdef, class XVector> vector<double> Solver<ODEdef, XVector>::evalFunctionComponent(ptrdiff_t component, std::function<XVector(const double&)> f) {
  std::vector<XVector, Eigen::aligned_allocator<XVector> > result;

  result.reserve(odeSeries.size());
  for(auto t_iter=odeSeries[0].begin(); t_iter != odeSeries.end(); ++t_iter) {
        result.push_back(f(*t_iter));
  }

  return result;
}

/* Reset the state data. In particular, this clears the series table
 */
template <class ODEdef, class XVector> void
Solver<ODEdef, XVector>::reset() {
  tBegin = 0;
  tEnd = 0;
  dt = 0;
  tNumSteps = 0;
  initConditionsSet = false;

  odeSeries.clear_data();

}

/* Reserve memory for the result series table; 
 *   for adaptive time step algorithms, this is only an estimation.
 * Set begin, end and step size; the number of steps is calculated
 * If stepSize does not fit an integer number of times in the interval,
 *   it is reduced slightly.
 * Calculated number of steps is multiplied by 'growFactor', allowing to reserve
 *   extra memory; useful if it is known that an adaptive stepper will add steps
 */
template <class ODEdef, class XVector> void
Solver<ODEdef, XVector>::setRange(double begin, double end,
                                   double stepSize, double growFactor) {
  double remainder;

  assert((end != begin) and (stepSize != 0));
  assert((end - begin) * stepSize > 0);

  tBegin = begin;
  tEnd = end;
  dt = stepSize;
  tNumSteps = std::floor( std::abs(tEnd - tBegin) / dt );
  remainder = abs(tEnd - tBegin) - tNumSteps*dt;
  if (remainder != 0) {
    tNumSteps = tNumSteps + 1;
    dt = (tEnd - tBegin) / tNumSteps;
  }

  odeSeries.inc_maxlines((tNumSteps + 1) * growFactor);
  // +1 for the initial condition (which is not a step)

  setOdeDoneCondition();
}

/* Set the time range over which to propagate.
 * Set begin, end and number of steps; the step size is calculated
 * Of course, for a solver with adaptive step size, this is an estimation
 * Calculated number of steps is multiplied by 'growFactor', allowing to reserve
 *   extra memory; useful if it is known that an adaptive stepper will add steps
 */
template <class ODEdef, class XVector>
void Solver<ODEdef, XVector>::setRange(double begin, double end,
                                       int numSteps, double growFactor) {
  assert((end != begin) and (numSteps != 0));

  tBegin = begin;
  tEnd = end;
  tNumSteps = numSteps;
  dt = (tEnd - tBegin) / tNumSteps;

  odeSeries.inc_maxlines((tNumSteps + 1) * growFactor);
  // +1 for the initial condition (which is not a step)

  setOdeDoneCondition();
}

/* Prepare the result vectors series_t and series_x:
 * TODO: this seems deprecated
 *  - sanity check on stepping bounds
 *  - fill independent column of ODE series with the time step values
 */
/*template <class ODEdef, class XVector> void Solver<ODEdef, XVector>::discretize() {
  ptrdiff_t i;

  // Make sure the time range is properly initialized
  assert((dt != 0) and (tNumSteps != 0));
  assert((tEnd - tBegin)/dt > 0); // We know that dt != 0
  assert(tNumSteps <= odeSeries.get_maxlines());


  //series_t.reserve(tNumSteps);
  //series_x.resize(tNumSteps);  // Calls default constructor for each XVector


  for(i=0; i<tNumSteps; ++i) {
    odeSeries.set(0, i, tBegin + i*dt);
  }
}*/

/* Dump all or part of the vectors to cout. Designed for debugging
   TODO: - if " 'x' in vars " type parameter
         - allow multiple vector components
         - allow partial row dump (rows n to m; step k)
 */
template <class ODEdef, class XVector> void Solver<ODEdef, XVector>::dump(std::string cmpntName)
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
template <class ODEdef, class XVector> void Solver<ODEdef, XVector>::solve(Param parameters) {
  ptrdiff_t i;

  // Discretize the continuous given range
  //discretize();

  //assert(dX != NULL); // Make sure the function has been defined
  //  assert(initConditionsSet); // In a real solver we should check this
  for(i=1; i<tNumSteps; ++i) {
    //fill(series_x[i].begin(), series_x[i].end(), series_t[i]);
    odeSeries.line_of_data(tBegin + i*dt, XVector::Constant(odeSeries.get(0, i)));
  }
}

#endif
