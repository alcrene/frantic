#ifndef IO_H
#define IO_H

#include <string>
#include <fstream>
#include <map>
#include <boost/variant.hpp>
#include <Eigen/Dense>


/* Some helper functions for input/output operations
 * \todo: Add possibility to merge two parameter maps (overloaded + ?)
 */

namespace cent
{
  std::string getFreeFilenameToWrite(std::string filename, std::string directory="",
                                     const int max_files=100);
  /* structure for storing variables and names
   * provides a common interfaces for a programme's variables, for example for
   * a GUI wanting to modifify parameters or a function for writing/reading parameters
   */

  struct Parameter
  {
  public:
    using TVal = boost::variant<double, std::string>;
    enum TVal_types {TVAL_DOUBLE, TVAL_STRING};   // not used at the moment

    std::string display_str = "default";
    boost::variant<double, std::string> value = 0;
    bool modifiable = true;     // Typical usage: indicate whether to display as modifiable or fixed parameter
                                // Should not be possible to modify this after parameter creation

    Parameter() {}  // It seems implementation of std::map requires an empty constructor
    Parameter(const std::string display_str, const TVal value, const bool modifiable=true)
      : display_str(display_str), value(value), modifiable(modifiable) {}
    Parameter(const Parameter& source)
      : display_str(source.display_str), value(source.value), modifiable(source.modifiable) {}  // const required in order not to break implicit constructor declaration in std::map

    // Practical to be allowed to assigned directly to value
    Parameter& operator= (const TVal new_value){
      value = new_value;
      return *this;
    }
    Parameter& operator= (const Parameter& source) {
      display_str = source.display_str;
      value = source.value;
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

    
    // Functions to convert the value to a form for printing, displaying in text boxes, etc.
//    std::string export_value()
//    {
//      return export_value(value);
//    }
//    std::string export_value(std::string val)
//    {
//      return val;
//    }
//    std::string export_value(double val)
//    {
//      return export_value(std::string(val));
//    }

    
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
