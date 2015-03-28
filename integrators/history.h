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
     * 'beyond' is defined as greater than if the simulation is going forward (t0 < tn),
     * and lesser than if the simulation is going backwards (t0 > tn)
     * Values equal to the end time (t == tn) return false.
     * If t0 == tn, forward direction is assumed.
     */
    bool after_end(double t) {
      return (t0 <= tn) ? (t > tn) : (t < tn);
    }
    /* Returns true if t is before the beginning time of the simulation (t0)
     * 'before' is defined as lesser than if the simulation is going forward (t0 < tn),
     * and greater than if the simulation is going backwards (t0 > tn)
     * Values equal to the beginning time (t == t0) return true because the value at
     * t0 is typically given as an initial value.
     * If t0 == tn, forward direction is assumed.
     */
    bool before_begin(double t) {
      return (t0 <= tn) ? (t <= t0) : (t >= t0);
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
     * If begin == end, the series will have exactly one row
     */
    void set_range(double begin, double end, double stepSize) {
      double remainder;

      t0 = begin;
      tn = end;
      dt = stepSize;
      if (end != begin) {
        assert(stepSize != 0);
        assert((end - begin) * stepSize > 0);
        nSteps = std::floor( std::abs(tn - t0) / dt );
        remainder = abs(tn - t0) - nSteps*dt;
        if (remainder != 0) {
          nSteps = nSteps + 1;
          dt = (tn - t0) / nSteps;
        }
      } else {
        nSteps = 0;
        dt = 0;
      }
    }

    /* Reserve memory for the result series table;
     *   for adaptive time step algorithms, this is only an estimation.
     * Set begin, end and number of steps; the step size is calculated
     * Calculated number of steps is multiplied by 'growFactor', allowing to reserve
     *   extra memory; useful if it is known that an adaptive stepper will add steps
     * If begin == end, the series will have exactly one row
     */
    void set_range(double begin, double end, int numSteps) {

      t0 = begin;
      tn = end;
      if (end != begin) {
        assert(numSteps > 0);
        nSteps = numSteps;
        dt = (tn - t0) / nSteps;
      } else if (numSteps) {
        nSteps = 0;
        dt = 0;
      }

    }

    /* Basic sanity check for initial conditions
     * Returns false if one of the initialization values is clearly improperly set
     */
    virtual bool check_initialized() {
      bool initialized = true;
      if (this->tn == this->t0 or this->dt == 0 or this->nSteps == 0) {initialized = false;}
      return initialized;
    }

    struct SaveHistory
    {
      // Implementations should include an 'object' pointer pointing to the containing object
      std::string name;  // This string used by UI to identify related elements (both internally and visually)
      bool include_labels;
      std::string format;
      int max_files;

      virtual void operator() (const std::string& directory, const std::string& filename) = 0;
    };

  }; // History

  /* Specialized class for tables containing series data
   * (i.e. nD dependent vector (x) vs 1D independent variable (t))
   * An \c InitialState class gives the initial condition of the process;
   *   it must provide:
   *   - the () operator to evaluate over its domain
   *
   * \todo: Specialize class for InitialState == XVector (for non-delayed processes)
   * \todo: Implement move semantics constructor
   * \todo: Implement rvalue copy assignment with move semantics
   * \todo: Add macro for Eigen data members ? (might still be necessary for creation with 'new'
   * \todo: Implement structure(s?) to store error
   */
  template <typename XVector>
  class Series : public o2scl::table<std::vector<double> >, public virtual History
  {
  private:
    using super = o2scl::table<std::vector<double> >;

  public:

    using History::t0;
    using History::tn;
    using History::dt;
    using History::nSteps;
    
    struct Statistics
    {     
      std::vector<double> mean;//(XVector::SizeAtCompileTime);
      std::vector<double> max;//(XVector::SizeAtCompileTime);
      std::vector<double> min;//(XVector::SizeAtCompileTime);
      long nsteps;
    };

    Series(const std::string& varname="x", size_t cmaxlines=0);
    Series(const Series& source) = delete;
    Series(const Series&& source)
      : super(source)  // \todo: check that this is implemented with move semantics
    {
      initial_state = std::move(source.initial_state);
    }

    virtual bool check_initialized() {
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

    /* Low-level function that allows to set the time and value of a particular row
     * The onus is on the caller to ensure that \c t is valid at this \c row.
     */
    void set(size_t row, double t, const XVector& x);
    /* Set the values over the entire range to the result of \c function.
     * \c function should take a value of time (\c double) and return a state value (\c XVector).
     * Note: A more optimized function should probably be used within performance dependent loops.
     * \todo: Specify optional start and end times
     */
    void set(std::function<XVector(double)> function) {
      for(long n = 0; n < nSteps + 1; ++n) {  // There are always nSteps + 1 rows,
        set(n, t0 + n*dt, function(t0 + n*dt));
      }
    }

    void set_initial_state(const XVector& initial_state) {
      this->initial_state = initial_state;
      set(0, t0, initial_state); // The integrator expects the first row to be set
    }
    void line_of_data(double t, const XVector& x);  // overloaded data adding function to allow using the XVector type
    void update(double t, const XVector& x) { line_of_data(t, x); }  // alias for common interface. Might want to check http://stackoverflow.com/questions/3053561/how-do-i-assign-an-alias-to-a-function-name-in-c
    
    XVector operator ()() const; // Return the current state vector
    XVector getVectorAtTime(const size_t t_idx) const;

    struct dump_to_text_t : public SaveHistory {
      Series<XVector>* object;
      dump_to_text_t(Series<XVector>* containing_object,
                     const std::string& name = "series", bool include_labels = true,
                     const std::string& format = ", ", int max_files = 100) {
        object = containing_object; // We need a reference to the object instance
        this->name = name;
        this->include_labels = include_labels,
        this->format = format;
        this->max_files = max_files;
      }
      virtual void operator() (const std::string& directory, const std::string& filename);
    };
    dump_to_text_t dump_to_text(const std::string& name = "series", bool include_labels = true,
                                const std::string& format = ", ", int max_files = 100) {
      return dump_to_text_t(this, name, include_labels, format, max_files);
    }

    Statistics getStatistics();
    void reset(bool reset_range=false) {
      clear_data(); // Reset all data in order to restart a new computation
      History::reset(reset_range);  // Also reset t0, tn if reset_range == true
    }
    
    template <typename XXVector>  // The result of the function could have a different vector type
    Series<XXVector> eval_function(std::function<XXVector(double, XVector)> f) const {
      Series<XXVector> result("x", get_nlines());
      XVector cur_x;

      for(size_t irow=0; irow < get_nlines(); ++irow) {
        for(size_t icol=0; icol < XVector::SizeAtCompileTime; ++icol) {
          cur_x(icol) = get(icol + 1, irow);
        }
        result.line_of_data(get(0, irow), f(get(0, irow), cur_x));
      }

      return std::move(result);
    }

    double max(size_t icol); using o2scl::table<std::vector<double> >::max;
    double min(size_t icol); using o2scl::table<std::vector<double> >::min;
    
  protected:
    static std::array<std::string, 3> getFormatStrings(std::string format);
    XVector initial_state;  // if initial_state is defined by the value at more than one time point,
                            // one should be use InterpolatedSeries
    
  }; // End Series

  
  /* ======================================================================
       Series with local interpolation
       This class tacitly assumes that we are dealing with delayed DE series data
       'order' is the (min) interpolation order, 'ip' the number of nodes used for interpolation
       'order' is mostly used to add the correct number number of associated critical points;
       in a DE scheme, it should be at least as large as the order of the integrator.

       Uses Laplace/Hermite interpolation to permit efficient look-back; interpolation
       is optimised specifically for repeated sequential calls, as is the case
       with differential equation integrators.

       \todo: Allow \c initial_state to have different interpolation parameters.
              Should be a template parameter with a default type
       \todo: Add special case for when critical points are too close for interpolation order
              If we jumps, we might want to set successive points as critical
              Interpolation could fall back to linear interpolation in this case
       \todo: Do we need to use special Eigen STL allocator for coeff ?
       \todo: Allow prehistory to be defined by series, not just function
       \todo: Deal with initial times different than 0 ?
       ====================================================================== */
  template <typename XVector, int order, int ip=4>
  class InterpolatedSeries : public Series<XVector>
  {
    typedef Series<XVector> super;
    
  public:

    /* Constructor. The 'varname' and 'cmaxlines' are forwarded to the parent's (Series) constructor.
     * If this structure will be used to integrate a delayed system, the initial state
     * (value for r < t <= 0) should be set with set_initial_state.
     * \todo Refine assert to check that ip is sufficient for interpolation (consider schemes with different order than ip - 1) ? */
    InterpolatedSeries(std::string varname="x", size_t cmaxlines=0)
      : Series<XVector>(varname, cmaxlines) {
      assert(ip - 1 >= order);
    }
    /* \todo: Implement swap / move semantics */
    InterpolatedSeries& operator=(const InterpolatedSeries& other) {
      critical_points = other.critical_points;
      v = other.v;
      coeff = other.coeff;
      Series<XVector>::operator=(other);
      return *this;
    }


    /* Set the initial state (either initial value or initial function, if this
     *   is a delayed system.
     * \c state is an instance of InterpolatedSeries defined over the appropriate time domain.
     * \c state can be either a temporary (aka rvalue) (as produced by
     *   std::make_shared<Struct name>()) or an a shared_ptr that is persistent
     *   in the caller (i.e. an already declared lvalue we intend to reuse).
     *   Use of shared_ptr ensures that in both cases memory is properly deallocated.
     */
    void set_initial_state(std::shared_ptr<InterpolatedSeries<XVector, order, ip> > state) {
      initial_state = state;
      super::set(0, this->t0, (*initial_state)(this->t0)); // The integrator expects the first row to be set
    }

    /* Return the state vector at any time in the past.
     * Will perform interpolation when required
     * \todo: move to .tpp file
     */
    XVector operator () (double t) const {
      if (t < History::t0) {
        return (*initial_state)(t);  // Should fail if initial_state isn't initialized
      } else {
        return this->interpolate(t);
      }
    }
    
    XVector interpolate(double t) const;
    void add_critical_point(const double point, const double delay, const int max_criticality_order);
    void add_primary_critical_point(const double point, const double delay) {add_critical_point(point, delay, order);}
    void add_secondary_critical_point(const double point, const double delay) {add_critical_point(point, delay, order - 1);}
    /* Reset all data in order to restart a new computation
     * Everything is reinitialized to 0 or empty, except the interpolation order, which is assumed to be the same.
     * If interpolation order is different, it should be changed separately.
     */
    void reset() {
      v = 0;
      for (auto itr=coeff.begin(); itr != coeff.end(); ++itr) {
        *itr = XVector::Zero();   // Strictly speaking, should not be necessary
      }
      critical_points.clear();
      
      super::reset();
    }
    
  private:
    
    mutable size_t v = 0;                           // Avoid using v=-1 : size_t is strictly positive
    mutable std::array<XVector, ip> coeff;          // Making these two internal variables mutable allows calling interpolate as a const function
    std::set<double> critical_points;

    size_t getV(double t) const;
    std::array<double, 2> getNeighbourCritPoints(double t) const;
    void getLaplaceCoefficients() const;
    void getNextLaplaceCoefficients() const;
    XVector computePoly(double t) const;

  protected:
    std::shared_ptr<InterpolatedSeries<XVector, order, ip> > initial_state;

  }; // End InterpolatedSeries


  /* ======================================================================
       Probability density is stored as a collection of histograms,
       one per time point.
       At present this is an empty subclass of HistCollection
       I expect functions meaningful for a probability density will
       be added at some point.
     ====================================================================== */
  template <typename XVector>
  class ProbabilityDensity : public HistCollection<XVector>, virtual History
  {
  public:
    ProbabilityDensity(size_t estimated_snapshots=0)
      : HistCollection<XVector>(estimated_snapshots) {}
    void reset(bool reset_range=false) {
      HistCollection<XVector>::reset();
      History::reset(reset_range);
    }
    struct dump_to_text_t : public SaveHistory {
      ProbabilityDensity<XVector>* object;
      dump_to_text_t(ProbabilityDensity<XVector>* containing_object,
                     const std::string& name = "series", bool include_labels = true,
                     const std::string& format = ", ", int max_files = 100) {
        object = containing_object;
        this->name = name;
        this->include_labels = include_labels,
        this->format = format;
        this->max_files = max_files;
      }
      virtual void operator() (const std::string& directory, const std::string& filename) {
        object->HistCollection<XVector>::dump_to_text(directory, filename, include_labels, format, max_files);
      }
    };
    dump_to_text_t dump_to_text(const std::string& name = "series", bool include_labels = true,
                                const std::string& format = ", ", int max_files = 100) {
      return dump_to_text_t(this, name, include_labels, format, max_files);
    }
  };

#include "history.tpp"
}



#endif // HISTORY_H
