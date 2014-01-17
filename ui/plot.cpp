#include "plot.h"

namespace cent {

  Plot::Plot(QWidget *parent) :
      QwtPlot(parent)
  {
  }

  void Plot::attachCurve(Curve* curve)
  {
    curve->attach(this);
    m_curves.push_back(curve);
  }

  void Plot::attachHist(Histogram* histogram)
  {
    histogram->attach(this);
    m_histograms.push_back(histogram);
  }

}
