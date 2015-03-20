#ifndef TAB_H
#define TAB_H

// #include "DEET_global.h"

#include <vector>
#include <assert.h>
#include <memory>

#include <QWidget>
#include <QMainWindow>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <qwt6/qwt_plot.h>

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

  };  // End BoxLayout

  class QtWindow : public QMainWindow
  {
    Q_OBJECT

  public:
    QtWindow(QMainWindow* parent = 0) : QMainWindow(parent) {}

    std::function<void()> slot0_function;  // Assign callables to these variables
    std::function<void()> slot1_function;  // and then connect them to
    std::function<void()> slot2_function;  // a button passing the corresponding
    std::function<void()> slot3_function;  // slot index to 'connect_button'

    void connect_button(QPushButton* button, int slot_idx,
                        std::function<void()> slot_function) {
      if (slot_idx == 0) {
        slot0_function = slot_function;
        connect(button, SIGNAL(clicked()), this, SLOT(slot0()));
      } else if (slot_idx == 1){
        slot1_function = slot_function;
        connect(button, SIGNAL(clicked()), this, SLOT(slot1()));
      } else if (slot_idx == 2){
        slot2_function = slot_function;
        connect(button, SIGNAL(clicked()), this, SLOT(slot2()));
      } else if (slot_idx == 3){
        slot3_function = slot_function;
        connect(button, SIGNAL(clicked()), this, SLOT(slot3()));
      } else {
        std::cerr << "No slot has index " << slot_idx << "." << std::endl;
        assert(false);
      }
    }

  public slots:
    //void run();
    //void interrupt_event();
    //void write_series_to_file();
    //void write_prob_dens_to_file();
    void slot0();  // Allow to connect up to 4 buttons to arbitrary functions
    void slot1();
    void slot2();
    void slot3();
  }; // End QtWindow

} // End of namespace

#endif // TAB_H
