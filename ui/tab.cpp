#include "tab.h"

tab::tab(QWidget *parent) :
    QWidget(parent)
{
    layout = new QGridLayout;
}

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

DtbPlot* tab::addPlot(int row, int column, int rowSpan, int columnSpan)
{
    DtbPlot* plotPtr = new DtbPlot(this);
    layout->addWidget(plotPtr, row, column, rowSpan, columnSpan);
    m_plots.push_back(plotPtr);

    return plotPtr;
}

QGridLayout* tab::getLayout()
{
  return layout;
}

DtbCurve* tab::addCurve(std::vector<double> xdata, std::vector<double> ydata, QString ylabel, std::string color, std::string style)
{
  DtbCurve* curveObj = new DtbCurve(xdata, ydata);
  curveObj->ylabel = ylabel;
  curveObj->plotformat.color = color;
  curveObj->plotformat.style = style;

  m_curves.push_back(curveObj);

  return curveObj;
}

DtbCurve* tab::getCurve(ptrdiff_t index)
/* Return a pointer to the curve at the specified index */
{
    return m_curves[index];
}

DtbCurve* tab::getCurve()
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
