#ifndef EULER_TPP
#define EULER_TPP

/*template <typename Functor, typename XVector> euler<Functor, XVector>::euler (Functor Rhs)
    :Solver<Functor, XVector>(Rhs) {

}*/

template <typename Functor, typename XVector> void Euler<Functor, XVector>::solve() {
    // Maybe this should be adapted to interpolate between two time steps, to allow
    // propagation backward in time

  ptrdiff_t i;

  // Discretize the continuous given range
//  this->discretize();

  //assert(dX != NULL); // Make sure the function has been defined
  //assert(initConditionsSet); // In a real solver we should check this

  odeSeries.line_of_data(tBegin, ode->x0);

  for(i=0; i < tNumSteps - 1; ++i) {
      odeSeries.line_of_data(tBegin + (i+1)*tStepSize, odeSeries.getVectorAtTime(i) + ode->dX.f(i, odeSeries) * tStepSize);
  }
}

#endif
