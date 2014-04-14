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

  /* Return a pointer to the plot widget associated with the specified index
   * (indexes are assigned incrementally in the order plots are added)
   * Specifying no index returns the most recently added plot.
   */
  Plot* tab::getPlot(int index)
  {
    assert(index >= -1); assert(index < m_plots.size());
    if (index == -1) {
        return m_plots.back();
    } else {
        return m_plots[index];
    }
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
    curveObj->setPen(color);

    m_curves.push_back(curveObj);

    return curveObj;
  }

  Curve* tab::getCurve(ptrdiff_t index)
  /* Return a pointer to the curve at the specified index */
  {
    assert(index >= 0  and  index < m_curves.size());
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


  void InfoBox::addInfo(const QString& label, const QString& infoText, const QString& name)
  {
    int newRow = rowCount();
    QLabel* labelWidget = new QLabel(label);
    QLabel* infoWidget = new QLabel(infoText);

    if (name == "") {
        elements.insertMulti("", QList<QLabel*>({labelWidget, infoWidget}));   // insertMulti because we don't want to replace any previous entry with name "".
      } else {
        assert(!elements.contains(name)); // Make sure this key doesn't already exist
        elements.insert(name, QList<QLabel*>({labelWidget, infoWidget}));
      }

    addWidget(labelWidget, newRow, 0);
    addWidget(infoWidget, newRow, 1);
  }

  void InfoBox::updateInfo(const QString &name, const QString &infoText) {
    auto el_itr = elements.find(name);
    assert(el_itr != elements.end());
    (*el_itr)[1]->setText(infoText);
  }

  /* Force an immediate repaint of each label in the infobox */
  void InfoBox::repaint() {
    foreach (QList<QLabel*> labellst, elements) {
        foreach(QLabel* label, labellst) {
            label->repaint();
          }
      }

  }

} // End of namespace
