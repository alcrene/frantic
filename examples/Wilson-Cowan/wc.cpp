#include <QApplication>
#include <QtGui>

#include <chrono>

#include "wc.h"



/* \todo:
 */

// constructor below

/* Main run function */
void WilsonCowan::run()
{
  Differential::ParamType parameters;

  parameters = getParameters();
  Differential dX(solver.odeSeriesRef, parameters);

  dX.x0 << dX.initPhi(parameters.tau);
  dX.t0 = 0;
  dX.tn = 40;
  double stepsize = 0.01;


  solver.reset();
  solver.setRange(dX.t0, dX.tn, stepsize);
  solver.odeSeriesRef.addPrimaryCriticalPoint(0, parameters.tau);
  solver.odeSeriesRef.addSecondaryCriticalPoint(1, parameters.tau);
  solver.solve(dX);


  cent::Curve* re_curve = new cent::Curve(solver.odeSeriesRef, 0, 1, Qt::blue);
  cent::Curve* ri_curve = new cent::Curve(solver.odeSeriesRef, 0, 2, Qt::red);
  r1Plot->clear();
  r2Plot->clear();
  re_curve->attach(r1Plot);
  ri_curve->attach(r2Plot);

  tabOutput->show();

  // Write to file
//  solver.odeSeries.dumpToText(outputFilename, outputDirectory, true, ", ");

  //seriesPlot->setAxisScale(QwtPlot::yLeft, -0.8, 2);  // Comment out to use autoscale (default)

}

void WilsonCowan::writeToFile()
{
  std::array<std::string, 2> outputFilename = getFilename();
  solver.odeSeries.dumpToText(outputFilename[1], outputFilename[0], true, ", ");
}

Differential::XVector Differential::initPhi(double t) const {
  static Differential::XVector X_out;
  X_out << 1, 1;
  return X_out;
}

Differential::XVector Differential::h(double t) const {
  static Differential::XVector X_out;
  X_out << (t < 1 ? 1 : 0), 0;
  return X_out;
}

/* Overload line_of_data to add data to the cumulative vector
 * Overloading does entail a measurable cost, but it seems to be at most 10%.
 */
void Differential::XSeries::line_of_data(double t, Differential::XVector x) {
  SeriesParent::line_of_data(t, x);
}

/* Set the input boxes' values to defaults
 */
void WilsonCowan::setDefaults() {
  Differential::ParamType defaults;
  defaults.alpha << 1, 1;
  defaults.beta << 1, 1;
  defaults.tau = 1;
  defaults.w << 1, -1,
                1, -1;

  QString outputDirectory = "/home/alex/tmp/";
  QString outputFilename = "wc_series";

  inputBox->setValue("alpha_e", defaults.alpha(0));
  inputBox->setValue("alpha_i", defaults.alpha(1));
  inputBox->setValue("beta_e", defaults.beta(0));
  inputBox->setValue("beta_i", defaults.beta(1));
  inputBox->setValue("tau", defaults.tau);
  inputBox->setValue("w_11", defaults.w(0,0));
  inputBox->setValue("w_12", defaults.w(0,1));
  inputBox->setValue("w_21", defaults.w(1,0));
  inputBox->setValue("w_22", defaults.w(1,1));

  inputBox->setValue("outputDirectory", outputDirectory);
  inputBox->setValue("outputFilename", outputFilename);
}

Differential::ParamType WilsonCowan::getParameters() {
  Differential::ParamType parameters;

  parameters.alpha << inputBox->getValue("alpha_e"), inputBox->getValue("alpha_i");
  parameters.beta << inputBox->getValue("beta_e"), inputBox->getValue("beta_i");
  parameters.tau = inputBox->getValue("tau");
  parameters.w << inputBox->getValue("w_11"), inputBox->getValue("w_12"),
                  inputBox->getValue("w_21"), inputBox->getValue("w_22");

  return parameters;
}

std::array<std::string, 2> WilsonCowan::getFilename(){
  return std::array<std::string, 2>({inputBox->getValueString("outputDirectory").toStdString(),
                                     inputBox->getValueString("outputFilename").toStdString()});
}


/* Create and position the various widgets
 */
WilsonCowan::WilsonCowan(QMainWindow *parent) :
  QMainWindow(parent), solver("r_")
{

  setWindowTitle(tr("Delayed stochastic langevin equation"));
  resize(800, 600);

  tabOutput = new cent::tab();

  r1Plot = tabOutput->addPlot(0, 0);
  r2Plot = tabOutput->addPlot(1, 0);
  tabOutput->getLayout()->setColumnMinimumWidth(0, 600);  // \todo: make tab choose sensible defaults for columns with plots, infoBoxes, etc. (define minimum height/width values for each ?)
  tabOutput->getLayout()->setColumnStretch(0, 1);         // Give plots priority for filling window

  inputBox = new cent::InputBox;
  inputBox->addInputLine(QString::fromUtf8("τ"), "tau");
  inputBox->addInputLine(QList<QString>({QString::fromUtf8("α<sub>e</sub> :"), QString::fromUtf8("α<sub>i</sub> :")}),
                         QList<QString>({"alpha_e", "alpha_i"}));
  inputBox->addInputLine(QList<QString>({QString::fromUtf8("β<sub>e</sub> :"), QString::fromUtf8("β<sub>i</sub> :")}),
                         QList<QString>({"beta_e", "beta_i"}));
  inputBox->addInputLine(QList<QString>({"w :", ""}),
                         QList<QString>({"w_11", "w_12"}));
  inputBox->addInputLine(QList<QString>({"",""}),
                         QList<QString>({"w_21", "w_22"}));
  QPushButton* refreshButton = inputBox->addButton("Run");
  refreshButton->setDefault(true);                         // Assign this button to 'enter' keypress

  inputBox->addInputLine(QList<QString>({"write to :", " /"}),
                         QList<QString>({"outputDirectory", "outputFilename"}), 100);
  QPushButton* writeButton = inputBox->addButton("Write to file");

  tabOutput->getLayout()->addLayout(inputBox, 0, 1, 2, 1, Qt::AlignTop);  // \todo: make tab member function to do this
  tabOutput->getLayout()->setColumnMinimumWidth(1, 200);

  statusBox = new cent::InfoBox();
//  statusBox->addInfo("alpha_e :", "", "alpha_e");
//  tabOutput->getLayout()->addLayout(statusBox, 1, 1,Qt::AlignTop);
//  statusBox->addInfo("Total # runs: ", "", "total_runs");
//  statusBox->addInfo("Current run: ", "100000", "current_run");  // HACK: puts 1000 to ensure sufficient spacing
//  statusBox->addInfo("Run time: ", "", "run_time");
//  // \todo: set some minimum width for statusBox instead of above hack

//  tabOutput->getLayout()->addLayout(statusBox, 0, 1, Qt::AlignTop);  // \todo: make tab member function to do this

  setCentralWidget(tabOutput);

  setDefaults();

  tabOutput->show();

  // Connect the signals and slots
  connect(refreshButton, SIGNAL(clicked()), this, SLOT(run()));
  connect(writeButton, SIGNAL(clicked()), this, SLOT(writeToFile()));

}


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    WilsonCowan mainUI;
    mainUI.show();

    //mainUI.run();

    return app.exec();
}
