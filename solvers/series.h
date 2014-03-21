#ifndef SERIES_H
#define SERIES_H

#include <vector>
#include <array>
#include <set>
//#include <algorithm.h>   // Required for std::lower_bound

#include <assert.h>

namespace solvers {

  /* Specialized class for tables containing series data
   * (i.e. nD dependent vector (x) vs 1D independent variable (t))
   * Uses Laplace/Hermite interpolation to permit efficient look-back; interpolation
   * is optimised specifically for repeated sequential calls, as is the case
   * with differential equation integrators.
   * 
   */
  template <class XVector, size_t ip=4>
    class Series : public o2scl::table<std::vector<double> >
    {
    public:

    struct Statistics
    {
      std::vector<double> mean;//(XVector::SizeAtCompileTime);
      std::vector<double> max;//(XVector::SizeAtCompileTime);
      std::vector<double> min;//(XVector::SizeAtCompileTime);
      long nsteps;
    };

    Series(std::string varname="x", size_t cmaxlines=0);
    void line_of_data(double t, XVector x);
    XVector getVectorAtTime(const size_t t_idx) const;
    Statistics getStatistics();

    double max(size_t icol); using o2scl::table<std::vector<double> >::max;
    double min(size_t icol); using o2scl::table<std::vector<double> >::min;


    /* ======================================================================
       Interpolation
       These functions tacitly assume that we are dealing with delayed DE series data
       ====================================================================== */


    

    /* Convenience class for storing and comparing critical points
       (points where the data set isn't smooth enough to interpolate)
    */
    class CriticalPointList
    {
      std::vector<double>& series_t;
      std::set<double> critPointList;   // std::set keeps its values ordered; duplicates are discarded

    CriticalPointList(std::vector<double>& independent_var_vector) : 
      series_t(independent_var_vector){}

      /* Add a critical point and the higher order critical points it induces to the list
       * 'point' is the t (independant variable) at the point
       * 'delay' is the value  of the delay (or distance between each successively induced point)
       * 'max_criticality_order' is the total number of critical points (including the first) induced */
      void addCriticalPoint(const double point, const double delay, const int max_criticality_order) {
        for(int i=0; i < max_criticality_order; ++i) {
          critPointList.insert(point + i*delay);
        }
      }
      
      /* Return the critical point with the given index.
         -1 indicates initial value (no critical point below) while -2 (or a value higher than the number of points) indicates final value (no critical point above). */
      double operator[](int i) {
        assert(i >= -2);
        if (i == -1) {
          return series_t.front();
        } else if (i == -2 or i >= critPointList.size()) {
          return series_t.back();
        } else {
          return critPointList[i];
        }
      }
    };

    std::array<double, ip> coeff;        
    CriticalPointList criticalPoints;

    double interpolate(double t);

    private:

    size_t v = -1;

    size_t getV(double t);
    std::array<double, 2> getNeighbourCritPoints(double t);
    void getLaplaceCoefficients();
    void getNextLaplaceCoefficients();
    double computePoly(double t);

    };

#include "series.tpp"
}



#endif // SERIES_H
