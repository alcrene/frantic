#ifndef CURVE_H
#define CURVE_H

//#include "DEET_global.h"

#include <assert.h>
#include <string>
#include <vector>
#include <QVector>
#include <QString>
#include <qwt6/qwt_plot_curve.h>
#include <o2scl/table.h>

/* TODO :
 *   - Constructor to set default value for plotdata?, plotformat
 */

namespace cent {

  class Curve : public QwtPlotCurve
  /* Class for preparing and aggregating a single set of information to be presented,
   * usually in a plot.
   * Could contain 'template' functions, which set many format parameters at once.
   */
  {
   private:


   public:
    Curve(const std::vector<double>& xdata, const std::vector<double>& ydata);
    Curve(const o2scl::table<std::vector<double> >& series, const size_t xcol, const size_t ycol);
    Curve(o2scl::table<std::vector<double> >& series, size_t xcol, size_t ycol,
          QColor color=Qt::black, std::string style="");

    QString xlabel;
    QString ylabel;
    QString name;      // Internal reference name

   signals:

   public slots:

  };

}
#endif // CURVE_H
