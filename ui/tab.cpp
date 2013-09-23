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
//    for(QVector<DtbPlot*>::const_iterator plot_iter=m_plots.begin(); plot_iter != m_plots.end(); ++plot_iter) {
//        (*plot_iter)->replot();
//        auto a = *plot_iter;
//    }
    for(ptrdiff_t i=0; i < m_plots.size(); ++i) {
        m_plots[i]->replot();
    }
    /* Add draw all plots (option?) ? */
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

DtbPlot* tab::add2Dplot()
{
    DtbPlot* plotPtr = new DtbPlot(this);
    layout->addWidget(plotPtr);
    m_plots.push_back(plotPtr);

    return plotPtr;
}
