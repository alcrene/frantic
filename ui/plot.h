#ifndef PLOT_H
#define PLOT_H

// #include "DEET_global.h"

#include <vector>
#include <QGridLayout>
#include <qwt6/qwt_plot.h>

#include "curve.h"
#include "histogram.h"

namespace frantic {

  class Plot : public QwtPlot
  {
    Q_OBJECT
  public:
    explicit Plot(QWidget *parent = 0);
    void clear();

  private:

  signals:

  public slots:

  };

}

#endif // PLOT_H
