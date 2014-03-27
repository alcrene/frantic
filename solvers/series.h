#ifndef SERIES_H
#define SERIES_H

#include <iostream>
#include <fstream>
#include <string>

#include <vector>
#include <array>
#include <set>
#include <iterator>        // Required for std::next
//#include <algorithm.h>   // Required for std::lower_bound

#include <assert.h>

namespace solvers {

  /* Specialized class for tables containing series data
   * (i.e. nD dependent vector (x) vs 1D independent variable (t))
   * Uses Laplace/Hermite interpolation to permit efficient look-back; interpolation
   * is optimised specifically for repeated sequential calls, as is the case
   * with differential equation integrators.
   *
   * \todo: Add macro for Eigen data members ? (might still be necessary for creation with 'new'
   * 
   */
  template <class XVector>
    class Series : public o2scl::table<std::vector<double> >
    {
    private:
      typedef o2scl::table<std::vector<double> > super;

    public:

    struct Statistics
    {     
      std::vector<double> mean;//(XVector::SizeAtCompileTime);
      std::vector<double> max;//(XVector::SizeAtCompileTime);
      std::vector<double> min;//(XVector::SizeAtCompileTime);
      long nsteps;
    };

    Series(std::string varname="x", size_t cmaxlines=0);
    Series& operator=(const Series& other) {o2scl::table<std::vector<double> >::operator=(other);}  // Copy assignment
    void line_of_data(double t, XVector x);
    XVector getVectorAtTime(const size_t t_idx) const;
    void dumpToText(const std::string filename, const std::string pathname="", const int max_files=100);
    Statistics getStatistics();
    /* Reset all data in order to restart a new computation */
    void reset() {clear_data();}

    double max(size_t icol); using o2scl::table<std::vector<double> >::max;
    double min(size_t icol); using o2scl::table<std::vector<double> >::min;

  };


    /* ======================================================================
       Series with local interpolation
       This class tacitly assumes that we are dealing with delayed DE series data
       'order' is the (min) interpolation order, 'ip' the number of nodes used for interpolation
       'order' is mostly used to add the correct number number of associated critical points;
       in a DE scheme, it should match the order of the integrator.
       \todo: Do we need to use special Eigen STL allocator for coeff ?
       ====================================================================== */
  template <class XVector, int order, int ip=4>
    class InterpolatedSeries : public Series<XVector>
    {
    private:
      typedef Series<XVector> super;

    public:

      /* \todo Refine assert to check that ip is sufficient for interpolation (consider schemes with different order than ip - 1) ? */
      InterpolatedSeries(std::string varname="x", size_t cmaxlines=0) :
        Series<XVector>(varname, cmaxlines) {
//          std::cout << v << std::endl;
//        std::cout << ip << std::endl;
//        std::cout << interpolationOrder << std::endl;
        assert(ip - 1 >= order);
      }
      /* \todo: Implement swap / move semantics */
      InterpolatedSeries& operator=(const InterpolatedSeries& other) {
        criticalPoints = other.criticalPoints;
        v = other.v;
        coeff = other.coeff;
        Series<XVector>::operator=(other);
        return *this;
      }

    /* Overloaded set class for critical points (points where the data set isn't smooth enough to interpolate)
     */
    class CriticalPointList : public std::set<double>
    {
    public:
//      std::vector<double>& series_t;
//      std::set<double> critPointList;   // std::set keeps its values ordered; duplicates are discarded

//    CriticalPointList(std::vector<double>& independent_var_vector) :
//      series_t(independent_var_vector){}

      /* Add a critical point and the higher order critical points it induces to the list
       * 'point' is the t (independant variable) at the point
       * 'delay' is the value  of the delay (or distance between each successively induced point)
       * 'max_criticality_order' is the total number of critical points (including the first) induced */
      void addCriticalPoint(const double point, const double delay, const int max_criticality_order) {
        for(int i=0; i < max_criticality_order; ++i) {
          this->insert(point + i*delay);
        }
      }
      
//      /* Return the critical point with the given index.
//         -1 indicates initial value (no critical point below) while -2 (or a value higher than the number of points) indicates final value (no critical point above). */
//      NOTE: Don't use front() / back(): back especially can return 0 if nlines != maxlines.
//      double operator[](int i) {
//        assert(i >= -2);
//        if (i == -1) {
//          return series_t.front();
//        } else if (i == -2 or i >= critPointList.size()) {
//          return series_t.back();
//        } else {
//          return critPointList[i];
//        }
//      }
    };

    XVector interpolate(double t);
    void addPrimaryCriticalPoint(const double point, const double delay) {criticalPoints.addCriticalPoint(point, delay, order);}
    void addSecondaryCriticalPoint(const double point, const double delay) {criticalPoints.addCriticialPOint(point, delay, order - 1);}
    /* Reset all data in order to restart a new computation
     * Everything is reinitialized to 0 or empty, except the interpolation order, which is assumed to be the same.
     * If interpolation order is different, it should be changed separately.
     */
    void reset() {
      v = 0;
      for (auto itr=coeff.begin(); itr != coeff.end(); ++itr) {
          *itr = XVector::Zero();   // Strictly speaking, should not be necessary
      }
      criticalPoints = CriticalPointList();

      super::reset();
    }

    private:

    size_t v = 0;  // *Don't* use v=-1 : size_t is strictly positive
    std::array<XVector, ip> coeff;
    CriticalPointList criticalPoints;

    size_t getV(double t);
    std::array<double, 2> getNeighbourCritPoints(double t);
    void getLaplaceCoefficients();
    void getNextLaplaceCoefficients();
    XVector computePoly(double t);

    };

#include "series.tpp"
}



#endif // SERIES_H
