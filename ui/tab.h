#ifndef TAB_H
#define TAB_H

// #include "DEET_global.h"

#include <vector>
#include <assert.h>
#include <memory>

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <qwt6/qwt_plot.h>

#include "uiparameter.h"
#include "curve.h"
#include "histogram.h"
#include "plot.h"

namespace frantic {

  class BoxLayout : public QGridLayout {
  public:
    QPushButton* addButton(const QString& label);
  };


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


   private:
      Q_OBJECT

      QVector<Plot*> m_plots;          // Collection of plots
      QGridLayout* layout;

  signals:

  public slots:

  };

} // End of namespace

#endif // TAB_H
