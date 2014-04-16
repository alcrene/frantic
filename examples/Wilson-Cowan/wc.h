/* Example run file for CENT
 * Computes the time series of the delayed Langevin equation for a set of parameters
 * and plots the result.
 */

#ifndef WilsonCowan_H
#define WilsonCowan_H

#include <iostream>
#include <assert.h>

#include <map>
#include <string>
#include <vector>
#include <array>
#include <functional>
#include <eigen3/Eigen/Dense>

#include <QMainWindow>
#include <QGridLayout>

#include "ui/qtcent.h"
#include "ui/tab.h"
#include "ui/curve.h"
#include "ui/plot.h"
#include "ui/histogram.h"

#include "differential.h"


/* UI driver class
 * Highest level code execution. Also provides UI.
 * Code in accompanying .cpp file
 */

class WilsonCowan : public QMainWindow
{
  Q_OBJECT

public:
  typedef solvers::Euler_sttic<Differential> TSolver;
  TSolver solver;

  explicit WilsonCowan(QMainWindow *parent = 0);
  void setDefaults();  // Sets the input boxes' values to defaults
  Differential::ParamType getParameters();
  std::array<std::string, 2> getFilename();
  ~WilsonCowan() {}

private:

  cent::tab* tabOutput;
  cent::InfoBox* statusBox;
  cent::InputBox* inputBox;
  cent::Plot* r1Plot;
  cent::Plot* r2Plot;

  //signals:

public slots:
  void run();
  void writeToFile();

};









#endif // WilsonCowan_H
