#include <QApplication>

#include "delayed_ou.h"

#include <QtGui>


/*
 * \todo: variable sized histogram limits
 */


/* Whenever the UI runs the simulation, it first calls run_initialization,
 * which should set up all variable values.
 * A pointer to the UI is passed to allow querying the interface for values.
 * Once initialized, it will loop for the number of runs the user has specified
 * on the UI; for each loop, it executes run_loop, which actually solves the problem.
 */
void DelayedOU::run_initialization(UI* ui) {

  integrator.history.set_range(0, ui->run_parameters.get<double>("tn"), 0.003);

  int ntbins = 1000;
  int nxbins = 75;

  //std::function<std::array<double, 2>(double, size_t)> binFunction = [](int t, size_t c){return std::array<double, 2>({-7*sqrt(t), 7*sqrt(t)});};
  //double variance = 10.66;  // variance for alpha=1.45
  double variance = 100;     // variance for alpha=1.56
  std::function<std::array<double, 2>(double, size_t)> binFunction =
      [variance](int t, size_t c){return std::array<double, 2>({-5*sqrt(variance), 5*sqrt(variance)});};  // Temporary hack until such a function is properly written

  integrator.history.ProbabilityDensity::reserve(ntbins);
  integrator.history.ProbabilityDensity::set_binning(binFunction, nxbins);

}

/* This is the core of the code, where we do the actual work.
 * One pass through this function corresponds to solving the integration problem once.
 * It's probably the only function worth optimizing.
 * We shouldn't need to connect to the UI here, as we have run_initialization for that.
 */
void DelayedOU::run_loop()
{

    integrator.reset();
    integrator.history.set_initial_state(std::make_shared<initPhi>());  // Could also use a permanent shared_ptr to an InitPhi object
    integrator.history.addPrimaryCriticalPoint(0, dX.tau.get());
    integrator.integrate(dX);

}

DelayedOU::Differential::XVector DelayedOU::initPhi::operator ()(double t) const {
  static Differential::XVector X_out;
  X_out << 1;
  return X_out;
}

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  DelayedOU sim;

  UI ui(sim, sim.dX, sim.integrator.history);

  ui.show();

  ui.run_simulation();

  return app.exec();
}
