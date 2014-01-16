#include "dtbplot.h"

DtbPlot::DtbPlot(QWidget *parent) :
    QwtPlot(parent)
{
}

void DtbPlot::attachCurve(DtbCurve* curve)
{
  curve->attach(this);
  m_curves.push_back(curve);
}

void DtbPlot::attachHist(Histogram* histogram)
{
  histogram->attach(this);
  m_histograms.push_back(histogram);
}
