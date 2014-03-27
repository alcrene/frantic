#include "tab.h"

namespace cent {

  tab::tab(QWidget *parent) :
      QWidget(parent)
  {
      layout = new QGridLayout;
  }

  /* \todo Get colour from plot.color and set it
   */
  void tab::show()
  {
      // Put the plot widgets on the tab
      setLayout(layout);

      // Display the plots
      for(ptrdiff_t i=0; i < m_plots.size(); ++i) {
          m_plots[i]->replot();
      }
      /* Add draw all plots (option?) ? */
  }

  Plot* tab::addPlot(int row, int column, int rowSpan, int columnSpan)
  {
      Plot* plotPtr = new Plot(this);
      layout->addWidget(plotPtr, row, column, rowSpan, columnSpan);
      m_plots.push_back(plotPtr);

      return plotPtr;
  }

  QGridLayout* tab::getLayout()
  {
    return layout;
  }

  //Curve* tab::addCurve(std::vector<double> xdata, std::vector<double> ydata,
  /* \todo: The way this is done seems like a hackish cludge to get what I want; there must be a better way
   */
  Curve* tab::addCurve(o2scl::table<std::vector<double> >& series, size_t xcol, size_t ycol,
                          QString ylabel, QColor color, std::string style)
  {
    std::vector<double>::const_iterator xdata_itr = series.get_column(series.get_column_name(xcol)).begin();
    std::vector<double>::const_iterator ydata_itr = series.get_column(series.get_column_name(ycol)).begin();
    std::vector<double> xdata(xdata_itr, xdata_itr + series.get_nlines());
    std::vector<double> ydata(ydata_itr, ydata_itr + series.get_nlines());
    Curve* curveObj = new Curve(xdata, ydata);

    //Curve* curveObj = new Curve(series, xcol, ycol);
    curveObj->ylabel = ylabel;
    //curveObj->setColor(color);
    //curveObj->plotformat.style = style;

    m_curves.push_back(curveObj);

    return curveObj;
  }

  Curve* tab::getCurve(ptrdiff_t index)
  /* Return a pointer to the curve at the specified index */
  {
      return m_curves[index];
  }

  Curve* tab::getCurve()
  /* Return a pointer to the current curve */
  {
      assert(!m_curves.empty());
      return m_curves.back();
  }


  Histogram* tab::addHist(const QString& title)
  {
    Histogram* histObj = new Histogram(title);

    m_histograms.push_back(histObj);

    return histObj;
  }


  void InfoBox::addInfo(const QString &label, const QString &infoText)
  {
    int newRow = rowCount();
    QLabel* labelWidget = new QLabel(label);
    QLabel* infoWidget = new QLabel(infoText);
    addWidget(labelWidget, newRow, 0);
    addWidget(infoWidget, newRow, 1);
  }

} // End of namespace
