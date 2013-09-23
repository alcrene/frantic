#include "dtbcurve.h"


DtbCurve::DtbCurve(std::vector<double> xdata, std::vector<double> ydata)
//    : plotdata(xdata, ydata)
{
    QVector<double> Qxdata = QVector<double>::fromStdVector(xdata);
    QVector<double> Qydata = QVector<double>::fromStdVector(ydata);
	this->setSamples(Qxdata, Qydata);
}

/*DtbCurve::cls_plotdata::cls_plotdata(std::vector<double> xdata_input, std::vector<double> ydata_input)
{
    xdata = QVector<double>::fromStdVector(xdata_input);
    ydata = QVector<double>::fromStdVector(ydata_input);
}
*/
