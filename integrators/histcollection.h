#ifndef HISTCOLLECTION_H
#define HISTCOLLECTION_H

#include <assert.h>
#include <iostream>
#include <string>
#include <fstream>
#include <array>
#include <vector>

#include "o2scl/hist.h"

namespace frantic {
  /* Container for a series of snapshot histograms
   * Histograms refer to a state when the indexing variable (typically time) is a certain value
   * There is one histogram per component in XVector per time point
   * Note that it is not required for 'XVector' to be the same type as the simulation's XVector:
   *    A different Eigen type can be declared, if for e.g. only a portion of the components need to be stored
   * \todo: bin searching optimizations which exploit continuity: next point to
   * add is assumed close to the last one ?
   * \todo: Allow more dimensions (use vector? of histograms); separate class?
   * \todo: Implement overflow/underflow bins that match usage (in terms of indexes and size_x) of o2scl::hist_2d
   *        -> use extend_rhs, extend_lhs
   * \todo: Subclass to allow variable step integrators (which won't have all the same t values)
   *        -> store, or allow extraction with the t values collected in a histogram
   *        -> (very long term)
   * \todo: Add clear_wgts function which applies clear_wgts to every histogram
   */
  template <typename XVector>
  class HistCollection
  {
  public:
    // \todo: include support for log, power?, 'critical point'? (as written in python) scales
    enum BinningMode {
      UNIFORM
    };

    using XState = std::array<o2scl::hist, XVector::SizeAtCompileTime>;

    HistCollection(size_t estimated_snapshots=0);
    void dumpToText(const std::string filename, const std::string pathname="",
                    const bool include_labels=true, const std::string format=", ", const int max_files=100);
    void update(double t, XVector& x, double val=1.0);
    void set_binning(std::function<std::array<double, 2>(double, size_t)> bin_limit_function, int nbins, BinningMode mode=UNIFORM);
    void reserve(size_t n);
    void reset();

  private:
    std::vector<double> tValues;
    std::vector<XState> xValues;
    BinningMode binningMode;
    int nbins;   // Number of bins in each histogram
    std::function<std::array<double, 2>(double, size_t)> get_bin_limits;
    // User-specified function which, given a time, returns the lower and upper limits
    // for the histogram corresponding to the specified component

    std::array<std::string, 3> getFormatStrings(std::string format);
    size_t find_t_idx(double t, double tol=1e-9);
    void set_bin_edges(o2scl::hist& hist, double t, size_t c);
  };

#include "histcollection.tpp"

} // End namespace frantic

#endif // HISTCOLLECTION_H
