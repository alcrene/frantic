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

namespace frantic {

  class Curve : public QwtPlotCurve
  /* Class for preparing and aggregating a single set of information to be presented,
   * usually in a plot.
   * Could contain 'template' functions, which set many format parameters at once.
   */
  {
   private:


   public:
    Curve(const std::vector<double>& xdata, const std::vector<double>& ydata);
    Curve(const o2scl::table<std::vector<double> >& series, size_t xcol, size_t ycol);
    Curve(const o2scl::table<std::vector<double> >& series, size_t xcol, size_t ycol,
          const QColor& color=Qt::black, const std::string& style="")
    /* \todo: Move this back to .cpp file. At present (25/03/2015), causes
     *   linker errors when trying to compile delayed_ou's standardwindow.h.
     *   (at frantic::Curve* curve = new frantic::Curve(series, 0, icol, frantic::randomColor());)
     *   Seems to have happened when I allowed series to be an rvalue.
     * \todo: The way this is done seems like a hackish cludge to get what I want; there must be a better way
     *        (Actually there might not be: http://stackoverflow.com/questions/13521745/qvector-object-construction-with-iterators)
     */
//    Curve::Curve(const o2scl::table<std::vector<double> >& series, size_t xcol, size_t ycol,
//                 const QColor& color, const std::string& style)
    {
      std::vector<double>::const_iterator xdata_itr = series.get_column(series.get_column_name(xcol)).begin();
      std::vector<double>::const_iterator ydata_itr = series.get_column(series.get_column_name(ycol)).begin();
      QVector<double> Qxdata = QVector<double>::fromStdVector(std::vector<double>(xdata_itr, xdata_itr + series.get_nlines()));
      QVector<double> Qydata = QVector<double>::fromStdVector(std::vector<double>(ydata_itr, ydata_itr + series.get_nlines()));
      this->setSamples(Qxdata, Qydata);

      //Curve* this = new Curve(series, xcol, ycol);
      this->ylabel = QString::fromStdString(series.get_column_name(ycol));
      this->xlabel = QString::fromStdString(series.get_column_name(xcol));
      this->setPen(color);
    }

    QString xlabel;
    QString ylabel;
    QString name;      // Internal reference name

   signals:

   public slots:

  };

}
#endif // CURVE_H
