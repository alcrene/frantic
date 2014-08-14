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

//  InputBox::InputBox(parameters)
//  {

//  }

  /* Add an input line to the layout
   * Returns a pointer to the created QLineEdit widget
   * 'minWidth' specifies the minimum width of the LineEdit box
   */
  QLineEdit* InputBox::addInputLine(const QString& label, const QString& name, const int minWidth)
  {
    int newRow = rowCount();
    QLabel* labelWidget = new QLabel(label);
    QLineEdit* lineWidget = new QLineEdit();

    if (name == "") {
      elements.insertMulti("", InputPair(labelWidget, lineWidget));   // insertMulti because we don't want to replace any previous entry with name "".
    } else {
      assert(!elements.contains(name)); // Make sure this key doesn't already exist
      elements.insert(name, InputPair(labelWidget, lineWidget));
    }

    addWidget(labelWidget, newRow, 0);
    addWidget(lineWidget, newRow, 1);

    if (columnMinimumWidth(1) < minWidth) {
      setColumnMinimumWidth(1, minWidth);
    }

    return lineWidget;
  }

  /* Add a collection of input pairs on the same line
   * 'names' must either be left unspecified, or have the same size as 'labels'
   * 'minWidth' specifies the minimum width of the LineEdit boxes
   */
  QLineEdit* InputBox::addInputLine(const QList<QString>& labels, const QList<QString>& names, const int minWidth)
  {
    if (names.size() > 0) {assert(labels.size() == names.size());}

    int newRow = rowCount();

    QLabel* labelWidget;
    QLineEdit* lineWidget;
    for (int i=0; i<labels.size(); ++i) {
      labelWidget = new QLabel(labels[i]);
      lineWidget = new QLineEdit();

      if (names[i] == "") {
        elements.insertMulti("", InputPair(labelWidget, lineWidget));   // insertMulti because we don't want to replace any previous entry with name "".
      } else {
        assert(!elements.contains(names[i])); // Make sure this key doesn't already exist
        elements.insert(names[i], InputPair(labelWidget, lineWidget));
      }

      addWidget(labelWidget, newRow, 2*i);
      addWidget(lineWidget, newRow, 2*i + 1);

      if (columnMinimumWidth(2*i + 1) < minWidth) {
        setColumnMinimumWidth(2*i + 1, minWidth);
      }
    }

    return lineWidget;
  }

  /* Return the value of the text box (QLineEdit) associated with 'name'
   */
  QString InputBox::getValueString(const QString& name) const
  {
    auto el_itr = elements.find(name);
    assert(el_itr != elements.end());
    return el_itr->box->text();
  }
  double InputBox::getValue(const QString& name) const
  {
    QString strVal = getValueString(name);
    return strVal.toDouble();
  }

  /* Set the specified input box's text to 'value'
   */
  void InputBox::setValue(const QString& name, const QString& value)
  {
    auto el_itr = elements.find(name);
    assert(el_itr != elements.end());
    el_itr->box->setText(value);
  }
  void InputBox::setValue(const QString& name, const int value)
  {
    auto el_itr = elements.find(name);
    assert(el_itr != elements.end());
    el_itr->box->setText(QString("%1").arg(value));
  }
  void InputBox::setValue(const QString& name, const double value)
  {
    auto el_itr = elements.find(name);
    assert(el_itr != elements.end());
    el_itr->box->setText(QString("%1").arg(value));
  }

  /* Add a button to the layout; this button must still be connected
   * Returns a pointer to the button widget
   * \todo: Accessing by name
   */
  QPushButton* InputBox::addButton(const QString& label, const QString& name)
  {
    int newRow = rowCount();
    QPushButton* button = new QPushButton(label);

    addWidget(button, newRow, 0, 1, 2, Qt::AlignCenter);  // \todo Set a max size to button ?

    return button;
  }
} // End of namespace
