#include "curve.h"

namespace frantic {

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
  Curve::Curve(const o2scl::table<std::vector<double> >& series, size_t xcol, size_t ycol)
  {
    assert(xcol != ycol);

    //this->setSamples(series[xcol].data(), series[ycol].data(), 100);//series.get_nlines());
    std::vector<double> xdata = series.get_column(series.get_column_name(xcol));
    std::vector<double> ydata = series.get_column(series.get_column_name(ycol));
    QVector<double> Qxdata = QVector<double>::fromStdVector(xdata);
    QVector<double> Qydata = QVector<double>::fromStdVector(ydata);
    this->setSamples(Qxdata, Qydata);
  }


}
