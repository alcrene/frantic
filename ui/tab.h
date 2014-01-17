#ifndef TAB_H
#define TAB_H

// #include "DEET_global.h"

#include <vector>
#include <assert.h>

#include <QWidget>
#include <QVector>
#include <QGridLayout>
#include <QLabel>
#include <qwt6/qwt_plot.h>

#include "curve.h"
#include "histogram.h"
#include "plot.h"

namespace cent {

  /* Manage a tab with plot widgets and input fields
   * TODO: Add memory managements for curves, histograms, etc. that arent attached?
   */
  class tab : public QWidget
  {
  public:
      explicit tab(QWidget *parent = 0);
      void show();

      void drawPlot(ptrdiff_t idx);
      void drawAllPlots();

      // Container objects
      Plot* addPlot(int row, int column, int rowSpan=1, int columnSpan=1);   // Add an empty plot widget to the tab.
      QGridLayout* getLayout();

      // Objects to insert in containers
  //    Curve* addCurve(std::vector<double> xdata, std::vector<double> ydata,
      Curve* addCurve(o2scl::table<std::vector<double> >& series, size_t xcol, size_t ycol,
                      QString ylabel="",
                      std::string color="", std::string style="");   // Add a new curve object
      Curve* getCurve(ptrdiff_t index);   // Get pointer to a curve object, to allow setting its properties
      Curve* getCurve();
      Histogram* addHist(const QString& title = QString::null);                 //


   private:
      Q_OBJECT

      QVector<Plot*> m_plots;          // Collection of plots
      QVector<Curve*> m_curves;        // Collection of curves that can be added to a plot
      QVector<Histogram*> m_histograms;   // Collection of histograms that can be added to a plot
      QGridLayout* layout;

  signals:

  public slots:

  };

  /* Provides an easy to use layout block for displaying information
   * in label/info pairs
   * Insert into a QGridLayout using that layout's addLayout() method
   */
  class InfoBox : public QGridLayout
  {
  public:
    // Default constructor is sufficient

    void addInfo(const QString& label, const QString& infoText);
    // TODO: overload to allow for mutable information
  };

} // End of namespace

#endif // TAB_H
