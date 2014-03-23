#ifndef EULER_TPP
#define EULER_TPP

/*template <typename Functor, typename XVector> euler<Functor, XVector>::euler (Functor Rhs)
    :Solver<Functor, XVector>(Rhs) {

}*/

template <typename ODEdef, typename XVector, typename XSeries> void Euler<ODEdef, XVector, XSeries>::solve(Param parameters) {
    // Maybe this should be adapted to interpolate between two time steps, to allow
    // propagation backward in time

  ptrdiff_t i;

  // Discretize the continuous given range
//  this->discretize();

  //assert(dX != NULL); // Make sure the function has been defined
  //assert(initConditionsSet); // In a real solver we should check this

  typename ODEdef::func_dX dX(odeSeries);
  dX.setParameters(parameters);

  double t = tBegin;
  XVector x = ode.x0;
  odeSeries.line_of_data(t, x);

  for(i=0; i < tNumSteps - 1; ++i) {
      x += dX.f(t, x) * dt;
      t += dt;
      odeSeries.line_of_data(t, x);
  }
}

#endif
