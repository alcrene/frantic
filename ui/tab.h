#ifndef TAB_H
#define TAB_H

// #include "DEET_global.h"

#include <vector>
#include <assert.h>

#include <QWidget>
#include <QVector>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
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


   private:
      Q_OBJECT

      QVector<Plot*> m_plots;          // Collection of plots
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

  /* Provides an easy to use layout block for parameter tuning widgets
   * (label/textbox pairs, sliders (todo), buttons)
   */
  class InputBox : public QGridLayout
  {
  public:
    struct InputPair {   // Contains pointers to the label and text input box(es?) of one line
      QLabel* label;
      QLineEdit* box;
      InputPair(QLabel* label, QLineEdit* box) : label(label), box(box){}
    };

    QLineEdit* addInputLine(const QString& label, const QString& name="", const int minWidth=30);
    QLineEdit* addInputLine(const QList<QString>& labels, const QList<QString>& names=QList<QString>(), const int minWidth=30);
    QString getValueString(const QString& name) const;
    double getValue(const QString& name) const;
    void setValue(const QString& name, const QString& value);
    void setValue(const QString& name, const int value);
    void setValue(const QString& name, const double value);
    QPushButton* addButton(const QString& label, const QString& name="");

    QHash<QString, InputPair > elements;
  };

} // End of namespace

#endif // TAB_H
