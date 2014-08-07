#include <chrono>

#include "brown_stats.h"

#include <QtGui>


/* \todo: variable sized histogram limits
 */

// constructor below

/* Main run function */
void BrownStats::run()
{
  /* Declared in header file:
     odeDef ode;
     cent::tab* tabOutput;
  */

  std::string outputDirectory = "/home/alex/tmp/";
  std::string outputFilename = "probDist";

  unsigned long n_runs = 100;
  double stepsize = 0.01;

  ode.x0 << 0;
  ode.t0 = 0;
  ode.tn = 10;

  int ntbins = 1000;
  int nxbins = 50;

  //std::function<std::array<double, 2>(double, size_t)> binFunction = [](int t, size_t c){return std::array<double, 2>({-7*sqrt(t), 7*sqrt(t)});};
  std::function<std::array<double, 2>(double, size_t)> binFunction = [](int t, size_t c){return std::array<double, 2>({-17, 17});};  // Temporary hack until such a function is properly written

  solvers::Param parameters;

  cent::Plot* seriesPlot = tabOutput->getPlot();
  QVector<cent::Curve*> seriesCurves;

  TSolver solver(ode);

  statusBox->updateInfo("total_runs", QString("%1").arg(n_runs));

  solver.odeSeriesRef.probDens.reserve(ntbins);
  solver.odeSeriesRef.probDens.set_binning(binFunction, nxbins);


  auto t1 = std::chrono::high_resolution_clock::now();
  auto t2 = std::chrono::high_resolution_clock::now();

  for(unsigned long i=0; i < n_runs; ++i) {
    t1 = std::chrono::high_resolution_clock::now();
    statusBox->updateInfo("current_run", QString("%1").arg(i+1));
    statusBox->repaint();

    parameters["alpha"] = -1;
    parameters["tau"] = 1;
    parameters["D"] = 1;

    solver.reset();
    solver.setRange(ode.t0, ode.tn, stepsize);
    solver.odeSeriesRef.addPrimaryCriticalPoint(0, parameters["tau"]);
    solver.solve(parameters);


    seriesCurves.push_back(tabOutput->addCurve(solver.odeSeriesRef, 0, 1, "", QtCENT::randomColor()));
    seriesPlot->attachCurve(seriesCurves[i]);

    //std::cout << i << std::endl;
    //      if ( (i+1) % 25 == 0) {
    //        // Force an occasional repaint so we can monitor progress,
    //        // \todo: get this to refresh the plot, and not just the number
    //        // \todo: better yet, add the new curve to the plot, without redrawing all other ones
    //        seriesPlot->repaint();

    //      }
    t2 = std::chrono::high_resolution_clock::now();
    //      std::cout << "computation loop took "
    //                << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count()
    //                << " milliseconds\n";
    statusBox->updateInfo("run_time", QString("%1 ms").arg(std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count()));
  }

  // Write to file
  //    o2scl_hdf::hdf_file hfile;
  //    hfile.open("SDE_probDens");
  //    o2scl_hdf::hdf_output(hfile, solver.odeSeries.probDens, "");
  //    hfile.close();
  solver.odeSeries.probDens.dumpToText(outputFilename, outputDirectory, true, ", ");

  //seriesPlot->setAxisScale(QwtPlot::yLeft, -0.8, 2);  // Comment out to use autoscale (default)

}

/* Overload line_of_data to add data to the cumulative vector
 * Overloading does entail a measurable cost, but this is at most 10%.
 */
void odeDef::XSeries::line_of_data(double t, odeDef::XVector x){
  SeriesParent::line_of_data(t, x);

  probDens.update(t, x);
}

odeDef::XVector odeDef::func_dX::initPhi(double t) {
  static odeDef::XVector X_out;
  X_out << 1;
  return X_out;
}


/* Create and position the various widgets
 */
BrownStats::BrownStats(QMainWindow *parent) :
  QMainWindow(parent)
{

  setWindowTitle(tr("Delayed stochastic langevin equation"));
  resize(800, 600);

  tabOutput = new cent::tab();

  tabOutput->addPlot(0, 0, 1, 1);

  statusBox = new cent::InfoBox();
  statusBox->addInfo("Total # runs: ", "", "total_runs");
  statusBox->addInfo("Current run: ", "100000", "current_run");  // HACK: puts 1000 to ensure sufficient spacing
  statusBox->addInfo("Run time: ", "", "run_time");
  // \todo: set some minimum width for statusBox instead of above hack

  tabOutput->getLayout()->addLayout(statusBox, 0, 1, Qt::AlignTop);  // \todo: make tab member function to do this

  setCentralWidget(tabOutput);

  tabOutput->show();


}
