/* Types (enums) classifying ode definitions, to allow for checking by solvers.
 * Defined outside solver.h to avoid name clashes
 */

//#include <tuple>

#ifndef ODETYPES_H
#define ODETYPES_H

namespace ODETypes {
// The shape of g(x,t) in the ODE definition
enum NOISE_SHAPE {
    NOISE_NONE,       // No noise
    NOISE_VECTOR,     // Only one random variable. i.e. noise weighting is deterministic, only magitude is random
    NOISE_MATRIX      // Multiple random variables.
};
}


/* Structure for defining parameter packing with a typedef
 * stackoverflow.com/questions/4691657/is-it-possible-to-store-a-template-parameter-pack-without-expanding-it
 */
/*template <typename... Args>
struct variadic_typedef
{
    // this single type represents a collection of types,
    // as the template arguments it took to define it
};

template <typename... Args>
struct convert_in_tuple
{
    // base case, nothing special,
    // just use the arguments directly
    // however they need to be used
    typedef std::tuple<Args...> type;
};

template <typename... Args>
struct convert_in_tuple<variadic_typedef<Args...> >
{
    // expand the variadic_typedef back into
    // its arguments, via specialization
    // (doesn't rely on functionality to be provided
    // by the variadic_typedef struct itself, generic)
    typedef typename convert_in_tuple<Args...>::type type;
};*/

#endif // ODETYPES_H
