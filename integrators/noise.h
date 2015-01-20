/* Stochastic noise generator class
 *
 * Provides helper functions for generating and keeping track of the stochastics of a simulation
 */

#ifndef NOISE_H
#define Noise_H

#include <vector>
#include <numeric>

#include "solver.h"

namespace solvers {

  template <typename TEngine, typename TDist, typename TNoise, typename... TDistParams>
    class Noise
	{
	public:
      Noise(const int seed, const TDistParams... params):
          m_seed(seed),
          m_engine(m_seed),
          m_dist(TDist(m_engine, params...)) {
      }

      /* Implement this as rvalue reference ! */
/*      Noise(const Noise& source) {
          &m_seed = &source.m_seed;
          &source.m_seed = 0;
          &m_engine = &source.m_engine;
          &source.m_engine = 0;
          &m_dist = &source.m_dist;
          &source.m_dist = 0;
      }*/

      virtual ~Noise() {}

      TNoise fire();
      void changeDistParams(const TDistParams... params);
      void flush(const bool also_free_memory=false);

      /* Statistics functions */
      double mean();
      double std();


      // Is this all that useful ?
      void setSeed(const int seed) {
           m_seed = seed;
      }
      vector<TNoise> getSeries() {return S;}


    private:

      int m_seed;                // Necessary ?
      TEngine m_engine;
      TDist m_dist;
      std::vector<TNoise> S;
    };

    /* Pull a new random number from the distribution.
     * Class keeps track of all the numbers that were pulled, in order to produce statistics
     */
    template <typename TEngine, typename TDist, typename TNoise, typename... TDistParams>
    TNoise Noise<TEngine, TDist, TNoise, TDistParams...>::fire() {
        TNoise dS = m_dist.fire();
        S.push_back(dS);
        return dS;
    }

    /* Change the parameters of the distribution, e.g. mean or variance.
     * In effect we create a new distribution object, discarding the old one.
     * For this reason, might become bottleneck if called really often.
     */
    template <typename TEngine, typename TDist, typename TNoise, typename... TDistParams>
    void Noise<TEngine, TDist, TNoise, TDistParams...>::changeDistParams(const TDistParams... params) {
        m_dist = TDist(m_engine, params...);
    }

    /* Flush the vector containing the pulled numbers
     * By default, memory is NOT deallocated; to do this as well, pass a true value as parameter
     */
    template <typename TEngine, typename TDist, typename TNoise, typename... TDistParams>
    void Noise<TEngine, TDist, TNoise, TDistParams...>::flush(const bool also_free_memory) {
        S.clear();
        if (also_free_memory) {
            // S.shrink_to_fit();  // Ideal, but doesn't seem implemented in current gcc
            std::vector<TNoise>().swap(S);   // Technically, this way doesn't even require the above.clear();
        }
    }

    /* Compute the mean of the generated numbers since the last call to flush()
     */
    template <typename TEngine, typename TDist, typename TNoise, typename... TDistParams>
    double Noise<TEngine, TDist, TNoise, TDistParams...>::mean() {
        /*v = std::vector<double>(105, 4);
        double init = 0;
        return std::accumulate<double>(v.begin(), v.end(), init);*/
        return std::accumulate(S.begin(), S.end(), (double) 0)/S.size();
    }

    /* Compute the standard deviation of the generated numbers since the last call to flush()
     */
    template <typename TEngine, typename TDist, typename TNoise, typename... TDistParams>
    double Noise<TEngine, TDist, TNoise, TDistParams...>::std() {
        double m = mean();
        return sqrt((double) std::accumulate(S.begin(), S.end(), (double) 0,
                            [m](double partialSum, double newTerm){
                                return partialSum + pow((newTerm - m), 2);
                            })/S.size());
    }
}

#endif // Noise_H
