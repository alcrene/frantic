#ifndef IO_H
#define IO_H

#include <string>
#include <fstream>
#include <map>
#include <boost/variant.hpp>
#include <boost/bind.hpp>
#include <Eigen/Dense>


/* Some helper functions for input/output operations
 * \todo: Add possibility to merge two parameter maps (overloaded + ?)
 */

namespace frantic
{
  std::string get_free_filename(std::string filename, std::string directory="",
                                const int max_files=100);


  /* structure for storing variables and names
   * provides a common interfaces for a program's variables, for example for
   * a GUI wanting to modifify parameters or a function for writing/reading parameters
   */
  struct Parameter
  {
  public:
    using TVal = boost::variant<double, std::string>;
    enum TVal_types {TVAL_DOUBLE, TVAL_STRING};   // not used at the moment

    std::string display_str = "default";
    TVal value = 0;
    bool modifiable = true;     // Typical usage: indicate whether to display as modifiable or fixed parameter
                                // \todo: Should not be possible to modify this after parameter creation
    void* var = nullptr;        // Optional reference to an external variable to be kept in sync with 'value'
                                // Somehow if this is not initialized last, the following variable(s?) is corrupted

    Parameter() {}  // It seems implementation of std::map requires an empty constructor
    Parameter(const std::string display_str, const TVal value, const bool modifiable=true)
      : display_str(display_str), value(value), modifiable(modifiable) {}

    Parameter(double* var, const std::string display_str, const TVal value, const bool modifiable=true)
      : display_str(display_str), value(value), modifiable(modifiable), var(var)
    {
      *(static_cast<double*>(var)) = boost::get<double>(value);
    }
    Parameter(std::string* var, const std::string display_str, const TVal value, const bool modifiable=true)
      : display_str(display_str), value(value), modifiable(modifiable), var(var)
    {
      *(static_cast<std::string*>(var)) = boost::get<std::string>(value);
    }
    Parameter(const Parameter& source)
      : display_str(source.display_str), value(source.value), modifiable(source.modifiable), var(source.var) {}  // const required in order not to break implicit constructor declaration in std::map


    // Practical to be allowed to assigned directly to value
    Parameter& operator= (const double new_value) {
      value = new_value;
      if (var != nullptr) {
        *(static_cast<double*>(var)) = new_value;
      }
      return *this;
    }
    Parameter& operator= (const std::string new_value) {
      value = new_value;
      if (var != nullptr) {
        *(static_cast<std::string*>(var)) = new_value;
      }
      return *this;
    }
    Parameter& operator= (const Parameter& source) {
      var = source.var;
      display_str = source.display_str;
      if ( const double* dvalue = boost::get<double>(&source.value) ) {
        value = *dvalue;
      } else if ( const std::string* svalue = boost::get<std::string>(&source.value) ) {
        value = *svalue;
      }
      modifiable = source.modifiable;
      return *this;
    }


    class value_to_string : public boost::static_visitor<std::string>
    {
    public:
      std::string operator()(const double val) const
      {
        // Function copied from http://stackoverflow.com/a/13709929
        std::string str = std::to_string(val);
        return str.erase( str.find_last_not_of('0') + 1, std::string::npos );  // Remove trailing zeros
      }
      std::string operator() (const std::string val) const {
        return val;
      }
    };

    std::string get_value()
    {
      return boost::apply_visitor(value_to_string(), value);
    }

    
  };

  class ParameterMap
  {
  public:

    //-------------------------------

    std::map<std::string, Parameter > parameter_map;

  public:
    ParameterMap() {}
    ParameterMap(const ParameterMap& other) : parameter_map(other.parameter_map) {}

    template <typename T>
    T get (const std::string& key) const
    {
      return boost::get<T>( parameter_map.find(key)->second.value );
    }

    Parameter& operator[] (const std::string& key) {return parameter_map[key];}

    // \todo: Add check for name clashes
    void add_parameter(std::string key, std::string display_str, Parameter::TVal value) {
      parameter_map[key] = Parameter(display_str, value);
    }
    void add_parameter(double* var, std::string key, std::string display_str, Parameter::TVal value) {
      parameter_map[key] = Parameter(var, display_str, value);
    }
    void add_parameter(std::string* var, std::string key, std::string display_str, Parameter::TVal value) {
      parameter_map[key] = Parameter(var, display_str, value);
    }
    void add_parameter(const std::string& key, const Parameter& parameter) {
      parameter_map[key] = parameter;
    }
    void update(const std::string& key, const Parameter& parameter) {
      add_parameter(key, parameter);
    }
    void update(const ParameterMap& parameters_to_update) {
      for (auto itr=parameters_to_update.cbegin(); itr!=parameters_to_update.cend(); ++itr) {
        add_parameter(itr->first, itr->second);
      }
    }

    std::map<std::string, Parameter>::iterator begin() {return parameter_map.begin();}
    std::map<std::string, Parameter>::const_iterator cbegin() const {return parameter_map.cbegin();}
    std::map<std::string, Parameter>::iterator end() {return parameter_map.end();}
    std::map<std::string, Parameter>::const_iterator cend() const {return parameter_map.cend();}

    void print() {
      for (auto itr = parameter_map.begin(); itr != parameter_map.end(); ++itr) {
        std::cout << itr->second.display_str << " : " << itr->second.value << std::endl;
      }
    }
  };

}

#endif // IO_H
