#include "tab.h"

namespace frantic {

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


  /* Add a button to the layout; this button must still be connected
   * Returns a pointer to the button widget
   * \todo: Accessing by name
   */
  QPushButton* BoxLayout::addButton(const QString& label)
  {
    int newRow = rowCount();
    QPushButton* button = new QPushButton(label);

    addWidget(button, newRow, 0, 1, 2, Qt::AlignCenter);  // \todo Set a max size to button ?

    return button;
  }

} // End of namespace
