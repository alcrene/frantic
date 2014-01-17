#include "curve.h"

namespace cent {

  Curve::Curve(std::vector<double> xdata, std::vector<double> ydata)
  //    : plotdata(xdata, ydata)
  {
      QVector<double> Qxdata = QVector<double>::fromStdVector(xdata);
      QVector<double> Qydata = QVector<double>::fromStdVector(ydata);
      this->setSamples(Qxdata, Qydata);
  }

  /* Create a curve object from table data.
   * Data is deep-copied
   * \todo: Check table isn't empty.
   * \todo: Allow specifying columns by name
   * \todo: Allow to create "raw" curve without copy, (data only linked). Note that in this case the underlying table must not be destroyed
   *        Note also that first try wasn't reliable: data was sometimes moved even if it wasn't asked explicitly
   *        (maybe related to mention on O2scl::table that the pointers are not reliable and can be moved)
   */
  Curve::Curve(o2scl::table<std::vector<double> >& series, size_t xcol, size_t ycol)
  {
    assert(xcol != ycol);
    this->setSamples(series[xcol].data(), series[ycol].data(), series.get_nlines());
  }

}
