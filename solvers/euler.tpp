#ifndef EULER_TPP
#define EULER_TPP

/*template <typename Functor, typename XVector> euler<Functor, XVector>::euler (Functor Rhs)
    :Solver<Functor, XVector>(Rhs) {

}*/

template <typename Functor, typename XVector> void euler<Functor, XVector>::solve() {
    // Maybe this should be adapted to interpolate between two series_t elements, to allow
    // propagation backward in time (or even maybe uneven timesteps ?)

  ptrdiff_t i;

  // Discretize the continuous given range
  this->discretize();

  //assert(dX != NULL); // Make sure the function has been defined
  //assert(initConditionsSet); // In a real solver we should check this

  series_x[0] = ode->x0;

  for(i=0; i < this->tNumSteps - 1; ++i) {
      series_x[i+1] = series_x[i] + this->dX(series_x[i],i*tStepSize);
  }
}

#endif
