#include <chrono>

#include "langevin.h"

#include <QtGui>

/* TODO
 */

Langevin::Langevin(QMainWindow *parent) :
    QMainWindow(parent)
{

    setupUI();

    ode.x0 << 1;

    std::vector<double> alphas = {-1};//, -0.5, -0.4, -0.3, -0.1, 0.3};
    QVector<QColor> colours = {Qt::blue, Qt::cyan, Qt::green, Qt::red, Qt::yellow, Qt::black};

    cent::Plot* seriesPlot = tabOutput->addPlot(0, 0, alphas.size(), 1);

    solvers::Param parameters;

    QVector<cent::Curve*> seriesCurves;

    TSolver solver(ode);


    for(ptrdiff_t i=0; i < alphas.size(); ++i) {
      parameters["alpha"] = alphas[i];
      parameters["tau"] = 1;

      solver.reset();
      solver.setRange(0, 50, 0.01);
      solver.odeSeriesRef.addPrimaryCriticalPoint(0, parameters["tau"]);
      auto t1 = std::chrono::high_resolution_clock::now();
      solver.solve(parameters);
      auto t2 = std::chrono::high_resolution_clock::now();
      //solver.odeSeriesRef.dumpToText("Euler.org", "/home/alex/tmp/");
      std::cout << "solver took "
           << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count()
           << " milliseconds\n";
      seriesCurves.push_back(tabOutput->addCurve(solver.odeSeriesRef, 0, 1));
      seriesCurves[i]->setPen(* new QPen(colours[i]));
      seriesPlot->attachCurve(seriesCurves[i]);

/*      solvers::Series<odeDef::XVector>::Statistics stats = solver.odeSeriesErrorRef.getStatistics();
      cent::InfoBox* statBox = new cent::InfoBox;
      statBox->addInfo("Maximum step error: ", QString("%1").arg(stats.max[0], 0, 'g', 4));
      statBox->addInfo("Average step error: ", QString("%1").arg(stats.mean[0], 0, 'g', 4));
      statBox->addInfo("Total number of steps: ", QString("%1").arg(stats.nsteps, 0, 'g', 4));
      tabOutput->getLayout()->addLayout(statBox, i, 1, Qt::AlignTop);*/
    }

    seriesPlot->setAxisScale(QwtPlot::yLeft, -0.8, 2);


    tabOutput->show();
}

odeDef::XVector odeDef::func_dX::initPhi(double t) {
  static odeDef::XVector X_out;
  X_out << 1;
  return X_out;
}

/* Create and position the various widgets
 */
void Langevin::setupUI() {

    setWindowTitle(tr("Differential Equation ToolBox"));
    resize(800, 600);

    tabOutput = new cent::tab();

    setCentralWidget(tabOutput);

}
