#ifndef NEWIO_H
#define NEWIO_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <Eigen/Dense>

namespace frantic
{

  std::string get_free_filename(const std::string& directory, const std::string& filename="",
                                int max_files=100);
  // Following two functions from http://stackoverflow.com/a/236803
  std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
  std::vector<std::string> split(const std::string &s, char delim);

  /* \todo: Make all but value a template parameter ?
   *        Would allow to define in typedef, shortening construction statement
   * \todo: Following above, overload tuple construction to allow specifying only values
   * \todo: Move modifiable as a template parameter to enforce it ?
   */
  template <typename T>
  class Parameter{
  public:

    using type = Parameter<T>;

    const std::string key;
    std::string display_str;

    T value;  // Only access directly in high-performance contexts; otherwise use get()

    bool modifiable;   // At the moment, not enforced: just an indication to the UI to make this field editable or not

    Parameter(const std::string& key, const std::string display_str, const T& value,
              const bool modifiable=true)
      : key(key), display_str(display_str), value(value), modifiable(modifiable) { }
    Parameter(const Parameter<T>& source) = delete;   // Probably shouldn't copy parameters
    Parameter(const Parameter<T>&& source)
      : key(std::move(source.key)), display_str(std::move(source.display_str)),
        value(std::move(source.value)), modifiable(std::move(modifiable)) {}

    Parameter& operator= (const T new_value) {
      value = new_value;
    }

    // virtual because derived classes might need to overload
    virtual T get () const {
      return value;
    }

  };

  /*
   * \todo: overload get functions
   * \todo: deduce store_internally from constructor overload somehow ?
   */
  template <bool store_internally, typename ...Params>
  struct ParameterTuple;

  template <bool store_internally, typename Param, typename ...Params>
  struct ParameterTuple<store_internally, Param, Params...> {
  protected:
    // If parameter is passed as an lvalue reference, it already exists =>
    // user expects to be able to modify it directly, so we save a reference.
    // Otherwise, we create an instance of the variable, because it has to stay
    // persistent for the life of the parameter tuple
    typename std::conditional<store_internally, Param, Param&>::type param;
    ParameterTuple<Params...> params;

  public:
    const bool empty = false;

    ParameterTuple(Param& param, Params& ...params)
      : param(param), params(params...) {
      assert(!store_internally);  // If parameter object exists outside of tuple's scope,
      // we don't want to create a duplicate
    }
    ParameterTuple(Param&& param, Params&& ...params)
      : param(std::move(param)), params(std::move(params)...) {
      assert(store_internally);  // Ensure we don't store a reference to a temporary => Bad Things
    }

    template <typename T>
    T get(const std::string& key) {
      if (param.key == key) {
        return param.value;
      } else if (!params.empty){
        return params.get(key);
      } else {
        std::cerr << "No parameter has key " << key;
        assert(false);
        return 0;
      }
    }

    template<typename T>
    void update(const std::string& key, const T new_value) {
      if (param.key == key) {
        param = new_value;
      } else if (!params.empty){
        return params.update(key);
      } else {
        std::cerr << "No parameter has key " << key;
        assert(false);
        return 0;
      }
    }

    template<template<typename...> class OtherTuple,
             typename OtherParam, class ...OtherParams>
    void update(const OtherTuple<OtherParam, OtherParams...>& other_tuple) {
      if (param.key == other_tuple.param.key) {
        param = other_tuple.param;
      } else if (!other_tuple.empty) {
        update(other_tuple);
      } else {
        std::cerr << "No parameter has key " << other_tuple.param.key;
        assert(false);
      }
    }

    void print() {
      std::cout << param.display_str << " : " << param.value << std::endl;
      if (!params.empty) {
        params.print();
      }
    }

  };

  template <bool store_internally>
  struct ParameterTuple<store_internally> {
    const bool empty = true;
  };

}


#endif // NEWIO_H
