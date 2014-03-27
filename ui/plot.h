#ifndef PLOT_H
#define PLOT_H

// #include "DEET_global.h"

#include <vector>
#include <QGridLayout>
#include <qwt6/qwt_plot.h>

#include "curve.h"
#include "histogram.h"

namespace cent {

  class Plot : public QwtPlot
  {
      Q_OBJECT
  public:
      explicit Plot(QWidget *parent = 0);
      void attachCurve(Curve* curve);
      void attachHist(Histogram* histogram);

  private:
      std::vector<Curve*> m_curves;
      std::vector<Histogram*> m_histograms;
  signals:

  public slots:

  };

}

#endif // PLOT_H
