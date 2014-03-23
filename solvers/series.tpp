#ifndef SERIES_TPP
#define SERIES_TPP

/* --------------------------------------------------------------------------
 * Series class
 * --------------------------------------------------------------------------*/

/*
 */
template <class XVector> Series<XVector>::Series(std::string varname, size_t cmaxlines) :
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

  stats.nsteps = this->get_nlines();

  for(size_t i = 1; i <  this->get_ncolumns(); ++i) {
      stats.max.push_back(this->max(i));
      stats.min.push_back(this->min(i));
      double sum = std::accumulate((*this)[i].begin(), (*this)[i].begin() + stats.nsteps, 0.0);
      stats.mean.push_back(sum/this->get_nlines());
  }



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


/* ===================================================================
   Interpolation functions
   See the Sage file InterpolationAlgo.swx for discussion, meaning of variables
   Python prototype code is in interpolation_prototype.py
   =================================================================== */

/* Reset all data in order to restart a new computation
 * Everything is reinitialized to 0 or empty, except the interpolation order, which is assumed to be the same.
 * If interpolation order is different, it should be changed separately.
 */
/*template <class XVector, int ip> void InterpolatedSeries<XVector, ip>::reset() {
      v = -1;
      for (auto itr=coeff.begin(); itr != coeff.end(); ++itr) {
          *itr = XVector::Zero();   // Strictly speaking, should not be necessary
      }
      criticalPoints = CriticalPointList();
    }*/

template <class XVector, int ip> XVector InterpolatedSeries<XVector, ip>::interpolate(double t) {
  //const std::vector<double>& tcol = (*this)[0];

  assert(t >= this->get(0,0) and t <= this->get(0,this->get_nlines()-1)); // Ensure we are interpolating within bounds

  // \todo: Might be a gain in speed if this is written from scratch (see docs), especially if we start from current position
  size_t t_found_idx = this->ordered_lookup("t", t);
  if (this->get(0, t_found_idx) == t) {
	return this->getVectorAtTime(t_found_idx);
  }

  size_t v = this->getV(t);
  if (v != this->v) {
	if (v == this->v + 1) {
	  this->v = v;
	  this->getNextLaplaceCoefficients();
	} else {
	  this->v = v;
	  this->getLaplaceCoefficients();
	}
  }

  return this->computePoly(t);
}


/* NOTE: This function assumes that the stepsize is sufficiently small to allow at least
         ip steps between any two critical points.
         It also does not choose proper points for a decreasing increment
         (can choose points such that all but the first are on same side 
         of interpolated point)
*/
template <class XVector, int ip> size_t InterpolatedSeries<XVector, ip>::getV(double t) {
  //const std::vector<double>& tcol = (*this)[0];
  size_t v = this->v;  // Don't link variables: this->v must not be modified
  
  size_t m = this->get_nlines();    //Maximum value to which we have integrated
  std::array<double, 2> xi = this->getNeighbourCritPoints(t);

  if ((v < ip - 1) or (this->get(0, v - ip + 1) > t)) {   // Order of these tests is important
        v = ip - 1;  // v is already too high: return to lowest possible value
	// \todo: make 'reverse' function for this case, instead of just restarting ?
	//        If values are going backwards, this test is actually insufficient (see above)
  }

  while (this->get(0, v) < xi[0]) {
	// Increase v until we've past the closest crit point less than t
	++v;
  }

  int l = int(ip / 2);
  while (this->get(0, v - l) <= t and v < m and this->get(0,v) < xi[1]) {
	// Increase v until t is middle of interpolation interval, or we hit a bound
	++v;
  }

  return v;
}

/* Given a time t, return the closest critical point below, and the closest critical point above, as an array:
 * std::array<double, 2>({below, above})
 * Throws an error if 't' is a critical point (should not try to interpolate in this case)
 * \todo: Treat the case of no critical point (.begin() == .end())
 * \todo: Make sure distance between points is large enough to interpolate
 */
template <class XVector, int ip> typename std::array<double, 2> InterpolatedSeries<XVector, ip>::getNeighbourCritPoints(double t) {

  static auto nextCritPointItr = criticalPoints.upper_bound(t); // Returns first element greater than t
//  this->neighbourCritPointItr[0] = std::prev(this->neighbourCritPointItr[1]);

  assert(*(std::prev(nextCritPointItr)) != t );


  // \todo Would something like the code below be more efficient, since we expect in general to only go to the next critical point ?
//  if (this->critPoints(this->neighbourCritPointIdxs[1]) < t) {
//	while (this->critPoints(this->neighbourCritPointIdxs[1]) < t) {
//	  ++(this->neighbourCritPointIdxs[1]);
//	}
//  } else if (this->critPoints(this->neighbourCritPointIdxs[0]) > t) {
//	while (this->critPoints(this->neighbourCritPointIdxs[0]) > t) {
//	  --(this->neighbourCritPointIdxs[0]);
//	}
//  }


  // \todo: Avoid creating array by having the member already existing in Series
  nextCritPointItr--;
  return std::array<double, 2>({*nextCritPointItr, *(nextCritPointItr++)});
}

template <class XVector, int ip> void InterpolatedSeries<XVector, ip>::getLaplaceCoefficients() {
  size_t& v = this->v;
//  const std::vector<double>& tcol = (*this)[0];

  static std::array<XVector, ip-1> d;

  this->coeff[0] = this->getVectorAtTime(v);

  assert(v - ip + 1 >= 0); // must have at least ip points behind v to interpolate with
	
  int i, n;
  for(i=0; i < ip - 1; ++i) {
	d[0] = (this->getVectorAtTime(v - i - 1) - this->getVectorAtTime(v - i))
	  / (this->get(0, v - i - 1) - this->get(0, v - i));
  }

  this->coeff[1] = d[0];

  for(n=2; n < ip; ++n) {
	for(i=0; i < ip - n; ++i) {
	  d[i] = ( d[i+1] - d[i] ) / (this->get(0, v - i - n) - this->get(0, v - i));
	}

	this->coeff[n] = d[0];
  }
}

template <class XVector, int ip> void InterpolatedSeries<XVector, ip>::getNextLaplaceCoefficients() {
  size_t& v = this->v;
//  const std::vector<double>& tcol = (*this)[0];

  std::array<XVector, ip> oldCoeff(this->coeff);

  this->coeff[0] = this->getVectorAtTime(v);

  int i;
  for(i=1; i < ip - 1; ++i) {
        this->coeff[i] = (oldCoeff[i-1] - this->coeff[i-1])/(this->get(0, v - i) - this->get(0, v));
  }

  this->coeff[ip - 1] = (oldCoeff[ip-2] - this->coeff[ip-2])/(this->get(0, v - ip + 1) - this->get(0, v));
}

/* Use Hörner's algorithm to compute the interpolation polynomial.
 * This function does no checking, so make sure coefficients are properly calculated beforehand.
 * \todo: Any way to implement this using only temporaries, i.e. in one line without the loop ?
 */
template <class XVector, int ip> XVector InterpolatedSeries<XVector, ip>::computePoly(double t) {
//  const std::vector<double>& tcol = (*this)[0];

  XVector b = this->coeff[ip - 1];
  for(int i=0; i < ip - 1; ++i) {
        b = (t - this->get(0, this->v - ip + 2 + i))*b + coeff[ip - 2 - i];
  }

  return b;
}

#endif
