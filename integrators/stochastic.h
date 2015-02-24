#ifndef STOCHASTIC_H
#define STOCHASTIC_H

#include <random>

namespace frantic {

  // shape should be derived from Eigen::DenseBase
  template <typename shape>
  class GaussianWhiteNoise
  {
    mutable std::mt19937 generator{};
    mutable std::normal_distribution<> dist;
    mutable double lastdt = 0;

  public:
    // const required for this to be used in an rvalue
    shape operator () (double dt) const {
      // \todo: check that normal lambda is really updated when dt changes
      // Based on code from here: http://eigen.tuxfamily.org/dox-devel/classEigen_1_1DenseBase.html#a15f13ef961b2c0709c8904281260222f
      static auto normal = [&] (double) {return dist(generator);};

      if (lastdt != dt) {
        // For multi-threading: ensure that the lines below are an atomic
        std::normal_distribution<>::param_type p(0, sqrt(dt));
        dist.param(p);
        lastdt = dt;
      }

      return shape::NullaryExpr(normal);
    }
  };

  // Specialization for single-valued doubles
  template <>
  class GaussianWhiteNoise <double>
  {
    mutable std::mt19937 generator{};
    mutable std::normal_distribution<> dist;
    mutable double lastdt = 0;

  public:
    double operator () (double dt) const {
      if (lastdt != dt) {
        std::normal_distribution<>::param_type p(0, sqrt(dt));
        dist.param(p);
        lastdt = dt;
      }

      double a = dist(generator);
      return a;
    }
  };

  // Primary declaration states that template can have as little as one type
  // All cases actually resolve to either one of the two specializations
  template <typename T1, typename ...Ts>
  struct Tuple {};

  template <typename T1, typename T2, typename ...Ts>
  struct Tuple<T1, T2, Ts...> {
    T1 val1;
    Tuple<T2, Ts...> vals;

    Tuple(T1 val1, T2 val2, Ts... vals) : val1(val1), vals(val2, vals...) {}

    template<typename TOther>
    auto sum_products(TOther other) {
      return val1 * other.val1 + vals.multiply(other.vals);
    }
  };

  template<typename T1>
  struct Tuple<T1> {
    T1 val1;
    Tuple(T1 val1) : val1(val1) {}

    template<typename TOther>
    auto sum_products (TOther other) {
      return val1 * other.val1;
    }
  };

}

#endif // STOCHASTIC_H
