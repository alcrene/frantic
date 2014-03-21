#ifndef SERIES_TPP
#define SERIES_TPP

/* --------------------------------------------------------------------------
 * Series class
 * --------------------------------------------------------------------------*/

/*
 */
template <class XVector, size_t ip> Series<XVector, size_t ip>::Series(std::string varname, size_t cmaxlines) :
  o2scl::table<std::vector<double> >(cmaxlines),
  criticalPoints((*this)[0]){
  std::string rowstr;
  rowstr = "t";
  for(size_t i=1; i<=XVector::SizeAtCompileTime; ++i) {
        rowstr += " " + varname + std::to_string(i);
  }
  this->line_of_names(rowstr);

  // Interpolation initializations
  this->v = -1;
  
}

// Overloaded data adding function to allow using the XVector type
template <class XVector, size_t ip> void Series<XVector, size_t ip>::line_of_data(double t, XVector x) {
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

template <class XVector, size_t ip> typename Series<XVector, size_t ip>::Statistics Series<XVector, size_t ip>::getStatistics() {

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

template <class XVector, size_t ip> XVector Series<XVector, size_t ip>::getVectorAtTime(const size_t t_idx) const {
  static XVector retval;
  for(size_t i=0; i<XVector::SizeAtCompileTime; ++i) {
      retval(i) = get(i+1, t_idx);
  };
  return retval;
}

// Convenience overloads
template <class XVector, size_t ip> double Series<XVector, size_t ip>::max(size_t icol) {
  return this->max(this->get_column_name(icol));
}

template <class XVector, size_t ip> double Series<XVector, size_t ip>::min(size_t icol) {
  return this->min(this->get_column_name(icol));
}


/* ===================================================================
   Interpolation functions
   See the Sage file InterpolationAlgo.swx for discussion, meaning of variables
   and Python prototype code
   =================================================================== */

template <class XVector, size_t ip> double Series<XVector, size_t ip>::interpolate(double t) {
  std::vector<double>& tcol = (*this)[0];

  assert(t >= tcol.front() and t <= tcol.back()); // Ensure we are interpolating within bounds

  // \todo: Might be a gain in speed if this is written from scratch (see docs)
  t_found_idx = this->ordered_lookup("t", t);
  if (tcol[t_found_idx] == t) {
	return this->getVectorAtTime(t_found_idx);
  }

  size_t v = this->getV(t);
  if (v != this->v) {
	if (v == this->v + 1) {
	  this->v = v;
	  this->getNextHermiteCoefficients();
	} else {
	  this->v = v;
	  self->getHermiteCoefficients();
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
template <class XVector, size_t ip> double series<XVector, size_t ip>::getV(double t) {
  std::vector<double>& tcol = (*this)[0];
  size_t v = this->v;  // Don't link variables: this->v must not be modified
  
  m = this->get_nlines();    //Maximum value to which we have integrated
  std::array<double, 2> xi = this->getNeighbourCritPoints(t);

  if (tcol[v - this->ip + 1] > t) {
	v = -1;  // v is already too high
	// \todo: make 'reverse' function for this case, instead of just restarting ?
	//        If values are going backwards, this test is actually insufficient (see above)
  }

  if (v == - 1) {
	// Start at the lowest value possible
	v = this->ip - 1;
  }

  while (tcol[v] < xi[0]) {
	// Increase v until we've past the closest crit point less than t
	++v;
  }

  l = int(this->ip / 2);
  while (tcol[v - l] <= t and v < m and tcol[v] < xi[1]) {
	// Increase v until t is middle of interpolation interval, or we hit a bound
	++v;
  }

  return v;
}

template <class XVector, size_t ip> std::array<double, 2> series<XVector, size_t ip>::getNeighbourCritPoints(t) {
  if (this->critPoints(this->neighbourCritPointIdxs[1]) < t) {
	while (this->critPoints(this->neighbourCritPointIdxs[1]) < t) {
	  ++(this->neighbourCritPointIdxs[1]);
	}
  } else if (this->critPoints(this->neighbourCritPointIdxs[0]) > t) {
	while (this->critPoints(this->neighbourCritPointIdxs[0]) > t) {
	  --(this->neighbourCritPointIdxs[0]);
	}
  }

  return (this->critPoints(this->neighbourCritPointIdxs[0]),
		  this->critPoints(this->neighbourCritPointIdxs[1]));
}

template <class XVector, size_t ip> void series<XVector, size_t ip>::getHermiteCoefficients() {
  size_t& v = this->v;
  std::vector<double>& tcol = (*this)[0];

  static std::array<double, ip-1> d;

  this->coeff[0] = this->getVectorAtTime(v);

  assert(v - ip + 1 >= 0); // must have at least ip points behind v to interpolate with
	
  int i;
  for(i=0; i < ip - 1; ++i) {
	d[0] = (this->getVectorAtTime(v - i - 1) - this->getVectorAtTime(v - i))
	  / (tcol[v - i - 1] - tcol[v - i]);
  }

  this->coeff[1] = d[0];

  for(n=2; n < ip; ++n) {
	for(i=0; i < ip - n; ++i) {
	  d[i] = ( d[i+1] - d[i] ) / (tcol[v - i - n] - tcol[v - i]);
	}

	this->coeff[n] = d[0];
  }
}

template <class XVector, size_t ip> void series<XVector, size_t ip>::getNextHermiteCoefficients() {
  size_t& v = this->v;
  std::vector<double>& tcol = (*this)[0];

  std::array<double, ip> oldCoeff(this->coeff);

  this->coeff[0] = this->getVectorAtTime(v);

  int i;
  for(i=1; i < ip - 1; ++i) {
	this->coeff[i] = (oldCoeff[i-1] - this->coeff[i-1])/(tcol[v - i] - tcol[v]);
  }

  this->coeff[ip - 1] = (oldCoeff[ip-2] - this->coeff[ip-2])/(tcol[v - ip + 1] - tcol[v]);
}

/* Use Hörner's algorithm to compute the interpolation polynomial.
 * This function does no checking, so make sure coefficients are properly calculated beforehand.
 * \todo: Any way to implement this using only temporaries, i.e. in one line without the loop ?
 */
template <class XVector, size_t ip> double series<XVector, size_t ip>::computePoly(t) {
  std::vector<double>& tcol = (*this)[0];

  double b = this->coeff[ip - 1];
  for(int i=0; i < ip - 1; ++i) {
	b = (t - tcol[this->v - ip + 2 + i])*b + coeff[ip - 2 - i];
  }

  return b;
}

#endif
