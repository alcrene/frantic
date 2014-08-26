
template <typename XVector> HistCollection<XVector>::HistCollection(size_t estimated_snapshots) {
  // If estimated_snapshots is provided, reserve appropriate space
  if (estimated_snapshots > 0) {
    reserve(estimated_snapshots);
  }

  // \todo: implement the extra over/underflow bins
}

template <typename XVector> void HistCollection<XVector>::reserve(size_t n) {
  tValues.reserve(n);
  xValues.reserve(n);
}

/* Set requirements to determine binning of the histograms
   * bin_limit_function should take two arguments, the time and component,
   * and return an array of two values: the lower limit of the first bin, and upper limit of the last bin
   * nbins is the number of bins desired between this interval
   * At the moment all histograms, at all times and for each component, use the same number of bins
   */
template <typename XVector>
void HistCollection<XVector>::set_binning(std::function<std::array<double, 2>(double, size_t)> bin_limit_function, int nbins, BinningMode mode)
{
  get_bin_limits = bin_limit_function;
  binningMode = mode;
  assert(nbins > 0);
  this->nbins = nbins;
}

/* Set the bin edges for all histograms. Expects an array of vectors,
   * one vector per component that is stored in a histogram.
   * Histogram bins are made the same for each time point
   */
// template <typename XVector>
// void HistCollection<XVector>::set_bin_edges(std::array<std::vector<double>, XVector::SizeAtCompileTime> edges) {

// }

/* Update the histogram(s) for time t with the value of the components of x.
   * If 'val' is specified, increment by the value of val; otherwise by 1.
   * \todo: Deal with over/underflow bins
   */
template <typename XVector> void HistCollection<XVector>::update(double t, XVector& x, double val) {
  size_t t_idx = find_t_idx(t);
  assert(t_idx < tValues.size() + 1);   // We don't deal with cases where t should be added to the begining (i.e. going backwards in time)

  if (t_idx == tValues.size()) {
    // t is larger than largest stored value: We need to add a set of histograms
    tValues.push_back(t);
    xValues.push_back(XState());
    // Initialize each histogram at this time point
    // Set their extend properties to true so we don't throw away under/overflows
    for (size_t c=0; c < XVector::SizeAtCompileTime; ++c) {
      xValues[t_idx][c].extend_rhs = true;
      xValues[t_idx][c].extend_lhs = true;
      set_bin_edges(xValues[t_idx][c], t, c);
    }
  }

  for (size_t i=0; i < XVector::SizeAtCompileTime; ++i) {
    xValues[t_idx][i].update(x[i], val);
  }
}

/* Return the index corresponding to time t
   * Returns next index value if t is larger than largest index (i.e. tValues.size())
   * Returns  if t is smaller than smallest index
   * A low tolerance is used just to guard against numerical issues where
   * values aren't quite equal
   * Requires tValues to be ordered; no check is made to this effect
   * \todo: Do something so optimization doesn't blow in face if we go backwards in time
   */
template <typename XVector> size_t HistCollection<XVector>::find_t_idx(double t, double tol) {
  // Start searching from the last index, since we are most likely to go forward in time
  static size_t last_t_idx = 0;

  assert(tValues.size() < (size_t) - 2);  // If tValues has as many entries as it can take,
  //a) anything more will make it burst and b) the return value format becomes ambiguous

  if (tValues.size() == 0) {
    return 0;  // If we don't quit here, tValues.back() and tValues.front() are undefined
  }

  if (t > tValues.back()) {
    last_t_idx = tValues.size();
    return last_t_idx;
  } else if (t < tValues.front()) {
    last_t_idx = tValues.size() + 1;
    return last_t_idx;
  }

  for (size_t i=last_t_idx; i < tValues.size(); ++i) {
    if (std::abs(tValues[i] - t) < tol) {
      last_t_idx = i;
      return last_t_idx;
    }
  }
  for (size_t i=0; i < last_t_idx; ++i) {
    if (std::abs(tValues[i] - t) < tol) {
      last_t_idx = i;
      return last_t_idx;
    }
  }

  // \todo: replace assert with "not found" exception
  assert(false);
}

template <typename XVector>
void HistCollection<XVector>::set_bin_edges(o2scl::hist& hist, double t, size_t c)
{
  if (binningMode == UNIFORM) {
    std::array<double, 2> limits = get_bin_limits(t, c);
    hist.set_bin_edges(o2scl::uniform_grid_end<double>(limits[0], limits[1], nbins));
  } else {
    assert(false);
  }
}

/* Dump the collection of histograms to a text file
   * \todo: add determination of extension according to format
   * \todo: allow to specify what labels to include by flags; e.g. per row or per block bin edges
   */
template <typename XVector>
void HistCollection<XVector>::dumpToText(const std::string pathname, const std::string filename,
                                         const bool include_labels, const std::string format, const int max_files) {

  std::string outfilename = cent::get_free_filename(pathname, filename, max_files);  // Returns "" if unsuccessful

  if (outfilename != "") {
    // Succesfully found a free filename
//    outfile.close();
    std::fstream outfile(outfilename.c_str(), std::ios::out);

    std::array<std::string, 3> formatStrings = getFormatStrings(format);
    std::string headChar = formatStrings[0];  // 1 or more characters that appears at the beginning of each line
    std::string sepChar = formatStrings[1];   // 1 or more characters that appears between each element on a line
    std::string tailChar = formatStrings[2];  // 1 or more characters that appears at the end of each line

    // Begin file with description comments
    outfile << "# Format: State snapshots" << std::endl;
    outfile << "# Details: One histogram per snapshot per state variable component." << std::endl;
    outfile << "#          Each histogram is preceded by a comment indicating snapshot time" << std::endl;
    outfile << "#          Sets of histograms are separated by a comment indicating the component they relate to." << std::endl;
    outfile << "# Row 1: bin edges" << std::endl;
    outfile << "# Row 2: bin weights" << std::endl;
    outfile << "# -- Parsing info -- " << std::endl;
    outfile << "# File info lines: " << 0 << std::endl;
    outfile << "# Block info lines: " << 0 << std::endl;
    outfile << "# Number of blocks: " << XVector::SizeAtCompileTime << std::endl;
    outfile << "# Row info lines: " << (include_labels ? 1 : 0) << std::endl;
    outfile << "# Info columns: " << 0 << std::endl;

    for (size_t c=0; c < XVector::SizeAtCompileTime; ++c) {             // c: "component"
      if (include_labels) {outfile << std::endl << "# Component: " << c << std::endl;}

      for(size_t t_idx=0; t_idx < tValues.size(); ++t_idx) {

        if (include_labels) {
          outfile << "# t: " << tValues[t_idx] << std::endl;
          outfile << headChar;
          for (size_t i=0; i < xValues[t_idx][c].size(); ++i) {
            outfile << xValues[t_idx][c].get_bin_low_i(i) << sepChar;
          }
          outfile << xValues[t_idx][c].get_bin_high_i(xValues[t_idx][c].size() - 1) << tailChar;
          outfile << std::endl;
        }

        outfile << headChar;

        for(size_t i=0; i < xValues[t_idx][c].size() - 1; ++i) {
          outfile << xValues[t_idx][c].get_wgt_i(i) << sepChar;
        }
        outfile << xValues[t_idx][c].get_wgt_i(xValues[t_idx][c].size() - 1) << tailChar;  // Don't put a sep character for last column
        outfile << std::endl;
      }

      outfile.close();

      std::cout << "Histogram snapshots written to \n" + outfilename + "\n";
    }
  } else {
    std::cerr << "Unable to open a file to export histogram snapshots data" << std::endl;
  }
}

template <typename XVector>
std::array<std::string, 3> HistCollection<XVector>::getFormatStrings(std::string format) {
  std::array<std::string, 3> formatStrings;

  if (format == "org") {
    formatStrings[0] = "|";
    formatStrings[1] = " |";
    formatStrings[2] = "|";
  } else {
    formatStrings[0] = "";
    formatStrings[1] = format;
    formatStrings[2] = "";
  }

  return formatStrings;
}
