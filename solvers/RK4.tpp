#ifndef RK4_TPP
#define RK4_TPP

/*template <typename Functor, typename XVector> RK4<Functor, XVector>::RK4 (Functor Rhs)
    :Solver<Functor, XVector>(Rhs) {

}*/

template <typename Functor, typename XVector> void RK4<Functor, XVector>::solve() {
    // Maybe this should be adapted to interpolate between two series_t elements, to allow
    // propagation backward in time (or even maybe uneven timesteps ?)

  ptrdiff_t i;

  XVector k1, k2, k3, k4;

  // Discretize the continuous given range
  this->discretize();

  //assert(dX != NULL); // Make sure the function has been defined
  //assert(initConditionsSet); // In a real solver we should check this

  series_x[0] = ode->x0;

  for(i=0; i < this->tNumSteps - 1; ++i) {
    k1 = tStepSize * ode->dX.f(series_x[i], i*tStepSize);
    k2 = tStepSize * ode->dX.f(series_x[i] + k1/2, (i + 0.5)*tStepSize);
    k3 = tStepSize * ode->dX.f(series_x[i] + k2/2, (i + 0.5)*tStepSize);
    k4 = tStepSize * ode->dX.f(series_x[i] + k3, (i + 1)*tStepSize);
    series_x[i+1] = series_x[i] + (k1 + 2*k2 + 2*k3 + k4)/6;
  }
}

#endif
