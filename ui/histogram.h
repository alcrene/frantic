#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <vector>
#include <forward_list>
#include <algorithm>
#include <assert.h>
#include <QString>
#include <QGridLayout>

#include <qwt6/qwt_plot_histogram.h>
//#include <qwt6/qwt_interval.h>
#include <qwt6/qwt_series_data.h>

/* TODO:
 *      - Could be nice to allow variable bin sizes
 *      - Set borderflags in intervalSample ?
 */

class Histogram : public QwtPlotHistogram
{

public:
    explicit Histogram(const QString& title = QString::null):QwtPlotHistogram(title){}

    void setSamples(const std::vector<double> &rawData, const int nbins, const double discardThreshold=0);
    
private:
    void fillBin(QwtIntervalSample& bin, std::forward_list<const double*>& dataPointers);

signals:
    
public slots:
    
};

#endif // HISTOGRAM_H
