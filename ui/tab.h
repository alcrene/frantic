#ifndef TAB_H
#define TAB_H

/* TODO
 *  - Use smart pointer for dtbCurve
 */

// #include "DEET_global.h"

#include <vector>
#include <assert.h>

#include <QWidget>
#include <QVector>
#include <QGridLayout>
#include <qwt6/qwt_plot.h>

#include "dtbcurve.h"
#include "dtbplot.h"

class tab : public QWidget
{	
public:
    explicit tab(QWidget *parent = 0);
    void show();

	void drawPlot(ptrdiff_t idx);
	void drawAllPlots();
    
    // Functions for creating the interace
    DtbCurve* addCurve(std::vector<double> xdata, std::vector<double> ydata,
                    QString ylabel="",
                    std::string color="", std::string style="");   // Add a new curve object
    DtbCurve* getCurve(ptrdiff_t index);   // Get pointer to a curve object, to allow setting its properties
    DtbCurve* getCurve();
    DtbPlot* add2Dplot();               // Add an empty plot widget to the tab.

 private:
    Q_OBJECT

    QVector<DtbPlot*> m_plots;
    QVector<DtbCurve*> m_curves;
    QGridLayout* layout;
	
signals:
    
public slots:
    
};

#endif // TAB_H
