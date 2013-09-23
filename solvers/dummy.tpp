#ifndef DUMMY_TPP
#define DUMMY_TPP

/* Propagate the ODE through time to produce the solution vector x(t) (named series_x)
* For this Dummy solver, this just means evaluating the given function at each point t
*/
template <typename Functor, typename XVector> void Dummy<Functor, XVector>::solve() {
    ptrdiff_t i;

    // Discretize the continuous given range
    this->discretize();

    //assert(dX != NULL); // Make sure the function has been defined
    //  assert(initConditionsSet); // In a real solver we should check this

    for(i=1; i<this->tNumSteps; ++i) {
        this->series_x[i] = this->dX(this->series_x[i], this->series_t[i]);
    }
}

#endif
