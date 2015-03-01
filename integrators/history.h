#ifndef HISTORY_H
#define HISTORY_H

#include <iostream>
#include <fstream>
#include <string>
#include <assert.h>
#include <vector>
#include <array>
#include <set>
#include <iterator>        // Required for std::next
//#include <algorithm.h>   // Required for std::lower_bound

#include "o2scl/table.h"
#include "histcollection.h"

namespace frantic {
  
  /* Common parent class to all history structures.
   * The template paramater indicates the data type of a state of the system.
   * Kept very succinct and inherited virtually, as can be used at the top of
   * a diamond inheritance
   */
  struct History {
  public:
    double t0 = 0, tn = 0;
    double dt;
    double nSteps;

    /* Returns true if t is beyond the end time of the simulation (tn)
     * 'beyond' is defined is greater than if the simulation is going forward (t0 < tn),
     * and lesser than if the simulation is going backwards (t0 > tn)
     * Values equal to the end time (t == tn) return false.
     */
    bool after_end(double t) {
      return (t0 < tn) ? (t > tn) : (t < tn);
    }
    /* Returns true if t is before the beginning time of the simulation (tn)
     * 'before' is defined is lesser than if the simulation is going forward (t0 < tn),
     * and greater than if the simulation is going backwards (t0 > tn)
     * Values equal to the beginning time (t == t0) return true because the value at
     * t0 is typically given as an initial value.
     */
    bool before_begin(double t) {
      return (t0 < tn) ? (t <= t0) : (t >= t0);
    }

    /* Reset history for another simulation run */
    void reset (bool reset_range=false) {
      if (reset_range) {
        t0 = 0;
        tn = 0;
        dt = 0;
        nSteps = 0;
      }
      // Don't do anything here that shouldn't be done twice (diamond inheritance)
    }

    /* Reserve memory for the result series table;
     *   for adaptive time step algorithms, this is only an estimation.
     * Set begin, end and step size; the number of steps is calculated
     * If stepSize does not fit an integer number of times in the interval,
     *   it is reduced slightly.
     * Calculated number of steps is multiplied by 'growFactor', allowing to reserve
     *   extra memory; useful if it is known that an adaptive stepper will add steps
     */
    void set_range(double begin, double end, double stepSize) {
      double remainder;

      assert(end != begin);
      assert(stepSize != 0);
      assert((end - begin) * stepSize > 0);

      t0 = begin;
      tn = end;
      dt = stepSize;
      nSteps = std::floor( std::abs(tn - t0) / dt );
      remainder = abs(tn - t0) - nSteps*dt;
      if (remainder != 0) {
        nSteps = nSteps + 1;
        dt = (tn - t0) / nSteps;
      }
    }

    /* Reserve memory for the result series table;
     *   for adaptive time step algorithms, this is only an estimation.
     * Set begin, end and number of steps; the step size is calculated
     * Calculated number of steps is multiplied by 'growFactor', allowing to reserve
     *   extra memory; useful if it is known that an adaptive stepper will add steps
     */
    void set_range(double begin, double end, int numSteps) {
      assert(end != begin);
      assert(numSteps != 0);

      t0 = begin;
      tn = end;
      nSteps = numSteps;
      dt = (tn - t0) / nSteps;

    }

    /* Basic sanity check for initial conditions
     * Returns false if one of the initialization values is clearly improperly set
     */
    virtual bool check_initialized() {
      bool initialized = true;
      if (this->tn == this->t0 or this->dt == 0 or this->nSteps == 0) {initialized = false;}
      return initialized;
    }
  };

  /* Specialized class for tables containing series data
   * (i.e. nD dependent vector (x) vs 1D independent variable (t))
   * Uses Laplace/Hermite interpolation to permit efficient look-back; interpolation
   * is optimised specifically for repeated sequential calls, as is the case
   * with differential equation integrators.
   *
   * \todo: Implement move semantics constructor
   * \todo: Implement rvalue copy assignment with move semantics
   * \todo: Add macro for Eigen data members ? (might still be necessary for creation with 'new'
   * \todo: Implement structure(s?) to store error
   */
  template <typename XVector>
  class Series : public o2scl::table<std::vector<double> >, public virtual History
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
    
    /* An initial state is simply a class providing the () operator to evaluate
     * it over its domain.
     * \todo: include domain variable (or just tr, with t0 deduced from History)
     */
    struct InitialState {
      virtual ~InitialState() {}
      virtual XVector operator()(double t) const {return XVector::Constant(0);} // Default implementation to be overloaded
    };

    Series(std::string varname="x", size_t cmaxlines=0);
    Series(const Series& source) = delete;
    // Series& operator=(const Series& other) {o2scl::table<std::vector<double> >::operator=(other);}  // rvalue copy assignment

    bool check_initialized() {
      bool retval = true;
      if (nlines == 0) {
        std::cerr << "Call set_initial_state() before integrating : first row of series table must be pre-filled.";
        retval = false;
      } else {
        retval = History::check_initialized();
      }
      return retval;
    }

    template <typename T>
    void set_range(T stepSize_or_numSteps, double growFactor = 1) {
      set_range(t0, tn, stepSize_or_numSteps, growFactor);
    }
    template <typename T>
    void set_range(double begin, double end, T stepSize_or_numSteps, double growFactor = 1) {
      History::set_range(begin, end, stepSize_or_numSteps);

      long minlines=(nSteps + 1) * growFactor; // +1 for the initial condition (which is not a step)
      if (get_maxlines() < minlines) {
        inc_maxlines(minlines - get_maxlines());  // inc_maxlines(n) appends n lines to the existing ones
      }
    }

    void set(size_t row, double t, const XVector& x);
    // Passed by rvalue reference (std::move or construct in argument)
    void set_initial_state( std::shared_ptr<InitialState> state) {
      (*state)(0);
      initial_state = std::move(state);
      (*initial_state)(0);
      set(0, t0, (*initial_state)(t0)); // The integrator expects the first row to be set
    }
    void line_of_data(double t, XVector& x);  // overloaded data adding function to allow using the XVector type
    void update(double t, XVector& x) { line_of_data(t, x); }  // alias for common interface. Might want to check http://stackoverflow.com/questions/3053561/how-do-i-assign-an-alias-to-a-function-name-in-c
    
    XVector operator ()() const; // Return the current state vector
    XVector getVectorAtTime(const size_t t_idx) const;
    void dumpToText(const std::string filename, const std::string pathname="",
                    const bool include_labels=true, const std::string format=", ", const int max_files=100);
    
    Statistics getStatistics();
    void reset(bool reset_range=false) {
      clear_data(); // Reset all data in order to restart a new computation
      History::reset(reset_range);  // Also reset t0, tn if reset_range == true
    }
    
    double max(size_t icol); using o2scl::table<std::vector<double> >::max;
    double min(size_t icol); using o2scl::table<std::vector<double> >::min;
    
  protected:
    std::array<std::string, 3> getFormatStrings(std::string format);
    std::shared_ptr<InitialState> initial_state;
    
  };

  
  /* ======================================================================
       Series with local interpolation
       This class tacitly assumes that we are dealing with delayed DE series data
       'order' is the (min) interpolation order, 'ip' the number of nodes used for interpolation
       'order' is mostly used to add the correct number number of associated critical points;
       in a DE scheme, it should be at least as large as the order of the integrator.
       \todo: Do we need to use special Eigen STL allocator for coeff ?
       \todo: Allow prehistory to be defined by series, not just function
       \todo: Deal with initial times different than 0 ?
       ====================================================================== */
  template <typename XVector, int order, int ip=4>
  class InterpolatedSeries : public Series<XVector>
  {
    typedef Series<XVector> super;
    
  public:
    
    class CriticalPointList;

    /* Constructor. The 'varname' and 'cmaxlines' are forwarded to the parent's (Series) constructor.
     * If this structure will be used to integrate a delayed system, the initial state
     * (value for r < t <= 0) should be specified as a function with signature
     * XVector initial_state(double t)
     * \todo Refine assert to check that ip is sufficient for interpolation (consider schemes with different order than ip - 1) ? */
    InterpolatedSeries(std::string varname="x", size_t cmaxlines=0)
      : Series<XVector>(varname, cmaxlines) {
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

    /* Return the state vector at any time in the past.
     * Will perform interpolation when required
     * \todo: move to .tpp file
     */
    XVector operator () (double t) const {
      if (t < History::t0) {
        return (*Series<XVector>::initial_state)(t);  // Should fail if initial_state isn't initialized
      } else {
        return this->interpolate(t);
      }
    }
    
    XVector interpolate(double t) const;
    void addPrimaryCriticalPoint(const double point, const double delay) {criticalPoints.addCriticalPoint(point, delay, order);}
    void addSecondaryCriticalPoint(const double point, const double delay) {criticalPoints.addCriticalPoint(point, delay, order - 1);}
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
    
    mutable size_t v = 0;                           // Avoid using v=-1 : size_t is strictly positive
    mutable std::array<XVector, ip> coeff;          // Making these two internal variables mutable allows calling interpolate as a const function
    CriticalPointList criticalPoints;

    size_t getV(double t) const;
    std::array<double, 2> getNeighbourCritPoints(double t) const;
    void getLaplaceCoefficients() const;
    void getNextLaplaceCoefficients() const;
    XVector computePoly(double t) const;
    

  };


  /* ======================================================================
       Probability density is stored as a collection of histograms,
       one per time point.
       At present this is an empty subclass of HistCollection
       I expect functions meaningful for a probability density will
       be added at some point.
     ====================================================================== */
  template <typename XVector>
  class ProbabilityDensity : public HistCollection<XVector>, public virtual History
  {
  public:
    ProbabilityDensity(size_t estimated_snapshots=0)
      : HistCollection<XVector>(estimated_snapshots) {}
    void reset(bool reset_range=false) {
      HistCollection<XVector>::reset();
      History::reset(reset_range);
    }
  };

#include "history.tpp"
}



#endif // HISTORY_H
