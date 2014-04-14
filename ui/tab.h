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
      Plot* getPlot(int index = -1);  // We use int to allow negative value. Also should be safe to assume that we have below 32,000 plots, so no risk of overflow

      QGridLayout* getLayout();

      // Objects to insert in containers
  //    Curve* addCurve(std::vector<double> xdata, std::vector<double> ydata,
      Curve* addCurve(o2scl::table<std::vector<double> >& series, size_t xcol, size_t ycol,
                      QString ylabel="",
                      QColor color=Qt::black, std::string style="");   // Add a new curve object
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

    void addInfo(const QString& label, const QString& infoText, const QString& name="");
    // TODO: overload to allow for mutable information
    void updateInfo(const QString& name, const QString& infoText);
    void repaint();

    QHash<QString, QList<QLabel*> > elements;  // Stores referencs to the info elements so they can be later modified
                                               // label/infoText widget* pairs as indexed by name; if no name is specified, the pair cannot be later retrieved.
  };

} // End of namespace

#endif // TAB_H
