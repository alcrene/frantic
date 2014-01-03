#include "dtbplot.h"

DtbPlot::DtbPlot(QWidget *parent) :
    QwtPlot(parent)
{
}

void DtbPlot::attachCurve(DtbCurve *curve)
{
  curve->attach(this);
}

void DtbPlot::attachHist(Histogram *histogram)
{
  histogram->attach(this);
}
