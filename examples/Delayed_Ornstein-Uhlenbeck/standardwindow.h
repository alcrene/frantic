#ifndef STANDARDWINDOW_H
#define STANDARDWINDOW_H

#include <functional>

#include <QApplication>
#include <QMainWindow>
#include <QGridLayout>
#include <chrono>

#include "ui/qtfrantic.h"
#include "ui/tab.h"
#include "ui/curve.h"
#include "ui/plot.h"
#include "ui/histogram.h"
#include "ui/uiparameter.h"
#include "integrators/history.h"

namespace frantic {

  // \todo: add SFINAE tests to ensure type is really a parameter
  template <class Simulation>
  class StandardWindow
  {

  public:
    //using OutputFunction = std::function<void(std::string, std::string, bool, std::string)>;

    std::function<void()> run_initialization;
    std::function<void()> run_loop;

    Simulation& sim;
    typename Simulation::Differential& dX;
    typename Simulation::Differential::XHistory& history;

    struct write_to_file;

    //StandardWindow(std::function<void()> run_initialization, std::function<void()> run_loop);
    void show() {
      window.show();
    }

    using RunParameters = frantic::UIParameterTuple<
    true,
    frantic::InputUIParameter<unsigned long>,
    frantic::InfoUIParameter<int>,
    frantic::InfoUIParameter<double>,
    frantic::InputUIParameter<double>,
    frantic::InputUIParameter<int>
    >;

    using OutputParameters = frantic::UIParameterTuple<
    true,
    frantic::InputUIParameter<std::string>,
    frantic::InputUIParameter<std::string>,
    frantic::InputUIParameter<std::string>
    >;

    RunParameters run_parameters = RunParameters(
          frantic::InputUIParameter<unsigned long>("total_runs", "Total # runs: ", 4),
          frantic::InfoUIParameter<int>("current_run", "Current run: ", 10000, false),  // HACK: puts 10000 to ensure sufficient spacing
          frantic::InfoUIParameter<double>("exec_time", "Execution time", 0, false),
          frantic::InputUIParameter<double>("tn", "Simulation time", 10),
          frantic::InputUIParameter<int>("max_traces", "Max traces: ", 1000)
          );

    OutputParameters output_parameters = OutputParameters(
          frantic::InputUIParameter<std::string>("directory", "write in: ", "/home/alex/Recherche/longtin_lab/langevin/data/"),
          frantic::InputUIParameter<std::string>("filename_series", "series: ", "langevin_series"),
          frantic::InputUIParameter<std::string>("filename_density", "density: ", "langevin_probability_density")
          );

  private:
    QtWindow window;

    frantic::tab* tab_output;
    frantic::BoxLayout* status_box;
    frantic::BoxLayout* fn_parameter_box;
    frantic::BoxLayout* output_box;
    frantic::Plot* series_plot;

    bool interrupt = false;

  public:
    /* \todo: allow to connect arbitrary buttons to arbitrary functions
     *        include some specialization for output_parameters ?
     */
    StandardWindow(Simulation& simulation, typename Simulation::Differential& differential,
                   typename Simulation::Differential::XHistory& history)
      : sim(simulation), dX(differential), history(history)
    {

      // Format window
      window.setWindowTitle(window.tr("Delayed stochastic langevin equation"));
      window.resize(800, 600);

      tab_output = new frantic::tab();

      series_plot = tab_output->addPlot(0, 0, 3, 1);
      tab_output->getLayout()->setColumnMinimumWidth(0, 600);  // \todo: make tab choose sensible defaults for columns with plots, infoBoxes, etc. (define minimum height/width values for each ?)
      tab_output->getLayout()->setColumnStretch(0, 1);         // Give plots priority for filling window

      status_box = new frantic::BoxLayout();
      run_parameters.attach(status_box);
      tab_output->getLayout()->addLayout(status_box, 0, 1, Qt::AlignTop);  // \todo: make tab member function to do this

      fn_parameter_box = new frantic::BoxLayout();
      dX.parameters.attach(fn_parameter_box);
      QPushButton* refreshButton = fn_parameter_box->addButton("Run");
      refreshButton->setDefault(true);                         // Assign this button to 'enter' keypress
      window.connect_button(refreshButton, 0, run(this));
      QPushButton* cancelButton = fn_parameter_box->addButton("Cancel");
      window.connect_button(cancelButton, 1, interrupt_event(this));

      tab_output->getLayout()->addLayout(fn_parameter_box, 1, 1, 1, 1, Qt::AlignTop);

      output_box = new frantic::BoxLayout();
      output_parameters.attach(output_box);

      QPushButton* write_series_button = output_box->addButton(QString::fromStdString("Save " + history.save_0.name));
      QPushButton* write_prob_dens_button = output_box->addButton(QString::fromStdString("Save " + history.save_1.name));
      window.connect_button(write_series_button, 2,
                            write_to_file(history.save_0,
                                         output_parameters, history.save_0.name));
      window.connect_button(write_prob_dens_button, 3,
                            write_to_file(history.save_1,
                                          output_parameters, history.save_1.name));
      tab_output->getLayout()->addLayout(output_box, 2, 1, 1, 1, Qt::AlignTop);  // \todo: make tab member function to do this

      tab_output->getLayout()->setColumnMinimumWidth(1, 200);


      window.setCentralWidget(tab_output);

      tab_output->show();

    }

    /* \todo: (conditionally) store result in temporary to guard against accidental deletion
     * \todo: refresh plot by adding curve without redrawing others
     */
    struct run
    {
      StandardWindow<Simulation>* wc;
      run(StandardWindow<Simulation>* window_container) : wc(window_container) { }
      void operator () () {
        wc->dX.parameters.refresh(); // User might forgot to call this in run_initialization()
        unsigned long n_runs = wc->run_parameters.template get<unsigned long>("total_runs");

        wc->sim.run_initialization(wc);

        auto timing_t0 = std::chrono::high_resolution_clock::now();
        long duration;

        wc->series_plot->clear();
        wc->interrupt = false;
        for(unsigned long i=0; (i < n_runs) && (!wc->interrupt); ++i) {
          wc->run_parameters.update("current_run", i+1);
          wc->run_parameters.repaint();

          timing_t0 = std::chrono::high_resolution_clock::now();
          wc->sim.run_loop();
          duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - timing_t0).count();

          if (i < wc->run_parameters.template get<int>("max_traces")) {
            frantic::Curve* curve = new frantic::Curve(wc->history, 0, 1, frantic::randomColor());
            curve->attach(wc->series_plot);
          }

          if (i * duration % 1000 <= 1) {
            // Do the following roughly every second
            // (by equaling to 1, we ensure it's done at least once, at the cost of usually doing it twice)
            QApplication::processEvents();   // Don't lock UI -> have not measured the speed cost of this
          }
          wc->run_parameters.update("exec_time", duration);
        }
        wc->series_plot->replot();
        // Safety write to file

        //series_plot->setAxisScale(QwtPlot::yLeft, -0.8, 2);  // Comment out to use autoscale (default)
      }
    };
    void run_simulation() {
      run(this)();
    }

    struct interrupt_event
    { StandardWindow<Simulation>* wc;
      interrupt_event(StandardWindow<Simulation>* window_container) : wc(window_container) { }
      void operator()() {
        wc->interrupt = true;
      }
    };

    /* Functor to write data from a History class to a file
     * The parameter storing the filename should have key "filename_[name]"
     */
    struct write_to_file {

    protected:
      History::SaveHistory& function;
      OutputParameters& parameters;
      std::string filename_key;

    public:
      write_to_file(History::SaveHistory& output_function,
                    OutputParameters& output_parameters,
                    const std::string& name
                    )
        : function(output_function), parameters(output_parameters),
          filename_key("filename_" + name) {}

      /*write_to_file(const OutputFunction& output_function,
                    const OutputParameters&& output_parameters,
                    const std::string& name
                    ) = delete;*/

      void operator() () {
        return function(parameters.get<std::string>("directory"),
                        parameters.get<std::string>(filename_key));
      }
    };  // End struct write_to_file

  };  // End class StandardWindow
}

#endif // STANDARDWINDOW_H
