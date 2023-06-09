#ifndef HISTORY_TPP
#define HISTORY_TPP

// \todo: Use of SizeAtCompileTime prohibits variable length vectors, which can be useful for evaluating functions
//        But we don't want to lose the compiler optimizations for lookups during simulation

/* --------------------------------------------------------------------------
 * Series class
 * --------------------------------------------------------------------------*/

/*
 */
template <typename XVector> Series<XVector>::Series(const std::string& varname, size_t cmaxlines) :
  o2scl::table<std::vector<double> >(cmaxlines) {
  std::string rowstr;
  rowstr = "t";
  for(size_t i=1; i<=XVector::SizeAtCompileTime; ++i) {
        rowstr += " " + varname + std::to_string(i);
  }
  this->line_of_names(rowstr);
}

/* Overloaded data adding function to allow using the XVector type
 */
template <typename XVector> void Series<XVector>::set(size_t row, double t, const XVector& x) {
  super::set(0, row, t);
  for(size_t i=0; i<XVector::SizeAtCompileTime; ++i) {
    super::set(i+1, row, x(i));
  }
}

/* Overloaded data adding function to allow using the XVector type
 * \todo: reinstate error checking
 */
template <typename XVector> void Series<XVector>::line_of_data(double t, const XVector& x) {
  // Virtually a copy of void line_of_data() from o2scl/table.h
  if (maxlines==0) inc_maxlines(5);
  if (nlines>=maxlines) inc_maxlines(maxlines);

  if (intp_set) {
        intp_set=false;
        delete si;
  }

  if (nlines<maxlines && XVector::SizeAtCompileTime<=(atree.size())) {

    set_nlines(nlines+1);
    super::set(0, nlines-1, t);
    for(size_t i=0; i<XVector::SizeAtCompileTime; ++i) {
      super::set(i+1, nlines-1, x(i));
	}

	return;
  }

//  O2SCL_ERR("Not enough lines or columns in line_of_data().",exc_einval);
  return;
}

/* Return the current state of the system, i.e. the XVector most recently
 * added to the table
 */
template <typename XVector> XVector Series<XVector>::operator ()() const {
  return getVectorAtTime(nlines);
}

/* Shorthand for getting state vector at time t
 */
template <typename XVector> XVector Series<XVector>::operator ()(const double t) const {
  return getVectorAtTime(t);
}

/* Output the table as plain text. Useful if we want to examine data manually, or
 * to import into another programme that cannot read a binary format.
 * File is saved as 'filename' in the current project directory.
 * To save in the current directory, pass an empty string to 'directory'.
 * If the filename exists or cannot be opened, it is appended with a number and retried;
 * this proceded is repeated until file is successfully opened or 'max_files' is reached.
 * \todo: Add number before file extension
 * \todo: Add trailing '/' to directory if necessary
 */
template <typename XVector>
void Series<XVector>::dump_to_text_t::operator() (const std::string& directory,
                                                  const std::string& filename) {
  std::string outfilename = frantic::get_free_filename(directory, filename, max_files);  // Returns "" if unsuccessful

  if (outfilename != "") {
    // Succesfully found a free filename
    //    outfile.close();
    std::fstream outfile(outfilename.c_str(), std::ios::out);

    std::array<std::string, 3> formatStrings = getFormatStrings(format);
    std::string headChar = formatStrings[0];  // 1 or more characters that appears at the beginning of each line
    std::string sepChar = formatStrings[1];   // 1 or more characters that appears between each element on a line
    std::string tailChar = formatStrings[2];  // 1 or more characters that appears at the end of each line

    // Begin file with description comments
    outfile << "# Format: Time series" << std::endl;
    outfile << "# Details: One column per time series" << std::endl;
    outfile << "#          First column are the times" << std::endl;
    if (include_labels) {
      outfile << "# Row 1: Column names" << std::endl;
      outfile << "# Column 1: timepoints" << std::endl;
    }
    outfile << "# -- Parsing info -- " << std::endl;
    outfile << "# File info lines: " << 0 << std::endl;
    outfile << "# Block info lines: " << (include_labels ? 1 : 0) << std::endl;
    outfile << "# Number of blocks: " << 1 << std::endl;
    outfile << "# Row info lines: " << 0 << std::endl;
    outfile << "# Info columns: " << 1 << std::endl;

    outfile << std::endl;
    if (include_labels) {
      std::string line = headChar;
      std::string sepline = headChar;
      for(size_t i=0; i<object->get_ncolumns() - 1; ++i) {
        line = line + object->get_column_name(i) + sepChar;
        sepline = sepline + std::string(object->get_column_name(i).length(), '-') + "-+";
      }
      line = line + object->get_column_name(object->get_ncolumns() - 1) + tailChar; // Don't put a sep character for last column
      sepline = sepline + std::string(object->get_column_name(object->get_ncolumns() - 1).length(), '-') + tailChar;

      if (format == "org") {
        outfile << sepline << std::endl;
        outfile << line << std::endl;
        outfile << sepline << std::endl;
      } else {
        outfile << line << std::endl;
      }
    }

    for(size_t i=0; i<object->get_nlines(); ++i) {
      outfile << headChar;
      for(size_t j=0; j < object->get_ncolumns() - 1; ++j) {
        outfile << object->get(j,i) << sepChar;
      }
      outfile << object->get(object->get_ncolumns() - 1, i) << tailChar;  // Don't put a sep character for last column
      outfile << std::endl;
    }

    outfile.close();

    std::cout << "Series written to" << std::endl << outfilename << std::endl;
  } else {
    std::cerr << "Unable to open a file to export series data." << std::endl;
  }

}

template <typename XVector>
void Series<XVector>::read_from_text(const std::string& directory, const std::string& filename,
                                     const std::string& format)
{
  // \tood: format parameter currently ignored, due to issue in split
  // \todo: see http://stackoverflow.com/questions/9371238/why-is-reading-lines-from-stdin-much-slower-in-c-than-python?rq=1  ?

  std::string basename = directory + "/";  // \todo: don't add if it's already there
  std::string infilename = basename + filename;

  std::ifstream infile(infilename.c_str());

  if (!infile.is_open()) {
    std::cerr << "Couldn't find file " << infilename << "." << std::endl;

  } else {
    reset(true);   // True indicates to also reset the range

    std::string line;
    std::stringstream sstream;
    double t;
    XVector datavec;
    std::vector<std::string> linetokens;
    // \todo: make this work with the different output formats
    while (std::getline(infile, line)) {
       if (line.size() && line[0] != '#') {
         // Line is not a comment; try to parse it into the expected number of tokens
         linetokens.clear();
         frantic::split(line, ' ', linetokens);  // \FIXME: Hardcoded token delimiter !!!!!!
         if (linetokens.size() == XVector::SizeAtCompileTime + 1) {
           // Line has the expected number of tokens; assume it's properly formatted
           for(size_t i=0; i < XVector::SizeAtCompileTime; ++i) {
             sstream.clear();
             sstream << linetokens[i+1];       // We don't use stod here because it is locale dependent
             sstream >> datavec[i];
             //datavec[i] << linetokens[i+1];
           }
           sstream.clear();
           sstream << linetokens[0];           // Same comment as above
           sstream >> t;
           line_of_data(t, datavec);
         }
       }
    }
    set_range(get(0,0), get(0, get_nlines()-1), (long) get_nlines());  // Reset the range based on the data. Type specified to avoid ambiguous overload
    // \todo: check that step size is consistent with data

    infile.close();
  }

}

template <typename XVector> std::array<std::string, 3> Series<XVector>::getFormatStrings(std::string format) {
  std::array<std::string, 3> formatStrings;

  if (format == "org") {
      formatStrings[0] = "|";
      formatStrings[1] = " |";
      formatStrings[2] = "|";
    } else {
      // If it's not a special format, use the format string as separator
      formatStrings[0] = "";
      formatStrings[1] = format;
      formatStrings[2] = "";
    }

  return formatStrings;
}

template <typename XVector> typename Series<XVector>::Statistics Series<XVector>::getStatistics() {

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

template <typename XVector> XVector Series<XVector>::getVectorAtTime(const size_t t_idx) const {
  static XVector retval;
  for(size_t i=0; i<XVector::SizeAtCompileTime; ++i) {
      retval(i) = get(i+1, t_idx);
  };
  return retval;
}

/* Convenience function that searches the history for a time 't' and returns the corresponding vector.
 * IMPORTANT: 't' must be _exactly_ equal to value in the series -- no interpolation is performed.
 * If you need interpolation, use the InterpolatedSeries class.
 */
template <typename XVector> XVector Series<XVector>::getVectorAtTime(const double t) const {
    bool failed = false;

    if (nlines < 2) {
        // ordered lookup needs at least 2 rows
        if (nlines == 0) {
            std::cerr << "Attempted to query an empty series !" << std::endl;
            assert(false);
        } else {
            if (this->get(0, 0) == t) {
                return this->getVectorAtTime(static_cast<size_t>(0));
            } else {
                failed = true;
            }
        }
    }
    size_t t_found_idx = this->ordered_lookup("t", t);  // ordered_lookup returns the closest index --
    if (this->get(0, t_found_idx) == t) {
      return this->getVectorAtTime(t_found_idx);
    } else {
      failed = true;
    }

    if (failed) {
      std::cerr << "You attempted to read series data at time " << t << ", which does not correspond to any time point." << std::endl;
      std::cerr << "If you need interpolation between time points, use the InterpolatedSeries class." << std::endl;
      assert(false);
    }
}

// Convenience overloads
template <typename XVector> double Series<XVector>::max(size_t icol) {
  return this->max(this->get_column_name(icol));
}

template <typename XVector> double Series<XVector>::min(size_t icol) {
  return this->min(this->get_column_name(icol));
}


/* ===================================================================
   Interpolation functions
   See the Sage file InterpolationAlgo.swx for discussion, meaning of variables
   Python prototype code is in interpolation_prototype.py
   =================================================================== */

template <typename XVector, int order, int ip> XVector InterpolatedSeries<XVector, order, ip>::interpolate(double t) const {
  //const std::vector<double>& tcol = (*this)[0];

  // There's sometimes some offset between the actual series bounds and the requested ones, which can result in the assertion failing
  if ((this->get(0,0) - super::dt <= t) and (t <= this->get(0,0))) { t = this->get(0,0); }
  if ((this->get(0,this->get_nlines()-1) <= t) and  (t <= this->get(0,this->get_nlines()-1) + super::dt)) { t = this->get(0,this->get_nlines()-1); }
  assert(t >= this->get(0,0) and t <= this->get(0,this->get_nlines()-1)); // Ensure we are interpolating within bounds

  size_t t_found_idx;
  if (t == 0) {
    // If we set delay at 0, "t" column is filled with zeros and ordered_lookup fails for t=0. This hackishly circumvents that
    // \todo: Make a cleaner solution
    // \todo: FIXME!! Wrong if t[0] is not 0, e.g. for initial state. Currently only "ok" because we use constant initial function
    t_found_idx = 0;
  } else {
    // \todo: Might be a gain in speed if this is written from scratch (see docs), especially if we start from current position
    //        Could also avoid problem of multiple 0 times, by returning first found one
    t_found_idx = this->ordered_lookup("t", t);
  }
  if (this->get(0, t_found_idx) == t) {
    return this->getVectorAtTime(t_found_idx);
  }

 /* if (t_found_idx > 0) {
    assert(this->get(0, t_found_idx - 1) < t);
    assert(this->get(0, t_found_idx) > t);
  }*/

  //this->v = ip - 2;  // DEBUG ONLY !!!!
  size_t v = this->getV(t);
  if (v != this->v) {
        if (v == this->v + 1) {  // \todo: make sure reset in getV never makes this accidentally verified
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
         It SHOULD not choose proper points for a decreasing increment
         (It used to in this case be able to choose points such that all but
         the first are on same side of interpolated point; I *think* this is fixed now, somewhat overzealously.)
*/
template <typename XVector, int order, int ip> size_t InterpolatedSeries<XVector, order, ip>::getV(double t) const {
  //const std::vector<double>& tcol = (*this)[0];
  static size_t v;    // temporary placeholder: this->v must not be modified
  static size_t m;    // Maximum value to which we have integrated
  static const int l = int(ip / 2);   // Half of the interpolation with
  static std::array<double, 2> xi;

  v = this->v;
  m = this->get_nlines();
  xi = this->getNeighbourCritPoints(t);

  // Check if v is already too high, and reset to lowest possible value
  // \todo: make 'reverse' function for this case, instead of just restarting ?
  //        If values are going backwards, this test might still be insufficient, or overkill (see above)
  if (v < ip - 1) {
      v = ip - 1;
    } else if (this->get(0, v - l) > t) {
      while(this->get(0, v - ip + 1) > t) {
        // Somewhat agressive resetting of v
        --v;
      }
  }


  while (this->get(0, v) < xi[0]) {
	// Increase v until we've past the closest crit point less than t
	++v;
  }


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
template <typename XVector, int order, int ip> typename std::array<double, 2> InterpolatedSeries<XVector, order, ip>::getNeighbourCritPoints(double t) const {

  static double nextCritPoint, prevCritPoint;

  if (critical_points.size() == 0) {
    // There are no critical points, so just return the begin and end times
    nextCritPoint = this->get(0, this->get_nlines()-1);
    prevCritPoint = this->get(0,0);
  } else {

    static std::set<double>::iterator CritPointItr;
    CritPointItr = critical_points.upper_bound(t); // Returns first element greater than t, or .end() if none
    //  this->neighbourCritPointItr[0] = std::prev(this->neighbourCritPointItr[1]);
    // We shouldn't need to check for getting critical point beyond current t, because that's what the 'm' does in getV()


    assert(*(std::prev(CritPointItr)) != t );
    assert(CritPointItr != critical_points.begin());   // The initial point should always be a critical point for DDEs, and if we are evaluating at it *exactly*, we don't need to interpolate.


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
    if (CritPointItr == critical_points.end()) {
      nextCritPoint = this->get(0, this->get_nlines()-1);
    } else {
      nextCritPoint = *CritPointItr;
    }
    CritPointItr--;
    if (CritPointItr == critical_points.begin()) {
      prevCritPoint = this->get(0,0);
    } else {
      prevCritPoint = *CritPointItr;
    }
  }

  return std::array<double, 2>({prevCritPoint, nextCritPoint});
}

template <typename XVector, int order, int ip> void InterpolatedSeries<XVector, order, ip>::getLaplaceCoefficients() const {
  size_t& v = this->v;
//  const std::vector<double>& tcol = (*this)[0];

  static std::array<XVector, ip-1> d;

  this->coeff[0] = this->getVectorAtTime(v);

  assert(v - ip + 1 >= 0); // must have at least ip points behind v to interpolate with
	
  int i, n;
  for(i=0; i < ip - 1; ++i) {
        d[i] = (this->getVectorAtTime(v - i - 1) - this->getVectorAtTime(v - i))
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

template <typename XVector, int order, int ip> void InterpolatedSeries<XVector, order, ip>::getNextLaplaceCoefficients() const {
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
template <typename XVector, int order, int ip> XVector InterpolatedSeries<XVector, order, ip>::computePoly(double t) const {
//  const std::vector<double>& tcol = (*this)[0];

  XVector b = this->coeff[ip - 1];
  for(int i=0; i < ip - 1; ++i) {
        b = (t - this->get(0, this->v - ip + 2 + i))*b + coeff[ip - 2 - i];
  }

  return b;
}

/* Add a critical point and the higher order critical points it induces to the list
 * 'point' is the t (independant variable) at the point
 * 'delay' is the value  of the delay (or distance between each successively induced point)
 * 'max_criticality_order' is the total number of critical points (including the first) induced */
template <typename XVector, int order, int ip>
void InterpolatedSeries<XVector, order, ip>::add_critical_point(const double point, const double delay, const int max_criticality_order) {
  for(int i=0; i < max_criticality_order; ++i) {
    critical_points.insert(point + i*delay);
  }
}

#endif
