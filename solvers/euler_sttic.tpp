#ifndef EULER_STTIC_TPP
#define EULER_STTIC_TPP

/*template <typename Functor, typename XVector> euler<Functor, XVector>::euler (Functor Rhs)
    :Solver<Functor, XVector>(Rhs) {

}*/

template <typename Functor, typename XVector, typename StticGen>
void euler_sttic<Functor, XVector, StticGen>::solve() {
    // Maybe this should be adapted to interpolate between two series_t elements, to allow
    // propagation backward in time (or even maybe uneven timesteps ?)
    // For uneven timesteps, noise parameters need to be changed every step.

  ptrdiff_t i;

  // Discretize the continuous given range
  this->discretize();

  // For Gaussian noise, the variance is equal to the step size
  // FOR NON-GAUSSIAN NOISE, THIS IS PROBABLY WRONG
  nbrGen->changeDistParams(0, tStepSize);

  //assert(dX != NULL); // Make sure the function has been defined
  //assert(initConditionsSet); // In a real solver we should check this

  series_x[0] = ode->x0;

  for(i=0; i < this->tNumSteps - 1; ++i) {
//      XVector test = dX.g(series_x[i], i*tStepSize);
      series_x[i+1] = series_x[i] + dX.f(series_x[i],i*tStepSize) * tStepSize + dX.g(series_x[i], i*tStepSize)*nbrGen->fire();
  }

}


/*template <typename Functor, typename XVector> vector<double> euler_sttic<Functor, XVector>::getWSeriesVector() {
    return W;
}*/



#endif
