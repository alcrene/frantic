#ifndef DTBPLOT_H
#define DTBPLOT_H

// #include "DEET_global.h"

#include <vector>
#include <QGridLayout>
#include <qwt6/qwt_plot.h>

#include "ui/dtbcurve.h"

class DtbPlot : public QwtPlot
{
    Q_OBJECT
public:
    explicit DtbPlot(QWidget *parent = 0);
    void attachCurve(DtbCurve* curve);

private:
    std::vector<DtbCurve> m_curves;
signals:
    
public slots:
    
};

#endif // DTBPLOT_H
