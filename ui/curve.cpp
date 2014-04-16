#include "curve.h"

namespace cent {

  Curve::Curve(const std::vector<double>& xdata, const std::vector<double>& ydata)
  //    : plotdata(xdata, ydata)
  {
      QVector<double> Qxdata = QVector<double>::fromStdVector(xdata);
      QVector<double> Qydata = QVector<double>::fromStdVector(ydata);
      this->setSamples(Qxdata, Qydata);
  }

  /* Create a curve object from table data.
   * Data is deep-copied
   * THIS DOES NOT WORK: somehow the whole vector is copied anyway.
   * \todo: Add version where columns are specified by name
   * \todo: Check table isn't empty.
   * \todo: Allow specifying columns by name
   * \todo: Allow to create "raw" curve without copy, (data only linked). Note that in this case the underlying table must not be destroyed
   *        Note also that first try wasn't reliable: data was sometimes moved even if it wasn't asked explicitly
   *        (maybe related to mention on O2scl::table that the pointers are not reliable and can be moved)
   */
  Curve::Curve(const o2scl::table<std::vector<double> >& series, const size_t xcol, const size_t ycol)
  {
    assert(xcol != ycol);

    //this->setSamples(series[xcol].data(), series[ycol].data(), 100);//series.get_nlines());
    std::vector<double> xdata = series.get_column(series.get_column_name(xcol));
    std::vector<double> ydata = series.get_column(series.get_column_name(ycol));
    QVector<double> Qxdata = QVector<double>::fromStdVector(xdata);
    QVector<double> Qydata = QVector<double>::fromStdVector(ydata);
    this->setSamples(Qxdata, Qydata);
  }

  /* \todo: The way this is done seems like a hackish cludge to get what I want; there must be a better way
   *        (Actually there might not be: http://stackoverflow.com/questions/13521745/qvector-object-construction-with-iterators)
   */
  Curve::Curve(o2scl::table<std::vector<double> >& series, size_t xcol, size_t ycol,
               QColor color, std::string style)
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
}
