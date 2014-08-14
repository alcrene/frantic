#ifndef IO_H
#define IO_H

#include <string>
#include <fstream>
#include <map>
#include <boost/variant.hpp>
#include <Eigen/Dense>


/* Some helper functions for input/output operations */

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

    std::string display_str;
    TVal value;

    Parameter() : display_str("default"), value(0) {}  // It seems implementation of std::map requires an empty constructor
    Parameter(const std::string display_str, const TVal value) : display_str(display_str), value(value) {}
    Parameter(const Parameter& source) : display_str(source.display_str), value(source.value) {}  // const required in order not to break implicit constructor declaration in std::map

    // Practical to be allowed to assigned directly to value
    Parameter& operator= (const TVal new_value){
      value = new_value;
      return *this;
    }
    Parameter& operator= (const Parameter& source) {
      display_str = source.display_str;
      value = source.value;
      return *this;
    }
  };

  class ParameterMap
  {
  private:

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
    void add_parameter(std::string name, std::string display_str, Parameter::TVal value) {
      parameter_map[name] = Parameter(display_str, value);
    }
  };

}

#endif // IO_H
