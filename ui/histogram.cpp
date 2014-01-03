#include "histogram.h"

/* Bin vector data into nbins number of bins and add to the histogram using its setSamples method.
 * The optional discardThreshold is a percentage indicating the minimum frequency we want in a bin.
 * Upper and lower bins with lower frequency will be combined into overflow bins.
 * Overflow bin frequency is not allowed to exceed the threshold frequency.
 * Binwidth is adjusted to keep the number of bins (excluding threshold) equal to nbins
 */
void Histogram::binData(const std::vector<double> &rawData, const int nbins, const double discardThreshold) {

    // TODO: Use QwtSeriesStore instead of QVectors?
    QVector<QwtIntervalSample> bins;
    std::vector<const double*> dataPtrs;
    bins.reserve(nbins+2);  // +2 for two overload bins

    auto minmax = std::minmax_element(rawData.begin(), rawData.end());
    double min = *minmax.first;
    double max = *minmax.second;
    std::vector<double>::size_type nDataPts = rawData.size();
    assert(min < max);

    double binWidth = (max - min)/nbins;

    // Create a list of pointers to the data, so that we can delete elements that have been sorted without counting over everything again
    for(auto itr = rawData.begin(); itr != rawData.end(); ++itr) {
        dataPtrs.push_back(&(*itr));
    }

	QwtIntervalSample bottomOverflowBin;
	QwtIntervalSample topOverflowBin;
	// TODO: Set appropriate borderflags
	//       Make sure the isNull() calls still work
    QwtIntervalSample bottomBin(0, min, min + binWidth);
    QwtIntervalSample topBin(0, max - binWidth, max);
	//    bins.push_back(QwtIntervalSample()); // Empty lower overflow bin
    //    bins.push_back(QwtIntervalSample(0, min, min + binWidth));

	bool refillBottomBin = true;
	bool refillTopBin = true;
	// Find the min/max cutoff values where either the top and bottom bins exceed the significance threshold,
	// or where their sum with the overflow bins exceed the threshold
    while(refillBottomBin or refillTopBin) {

	  // We don't just test for .value == 0 because it's conceivable
	  // that it might be 0 but the bin still kept because the overflow too large
	  if (refillBottomBin) {
		fillBin(bottomBin, dataPtrs);
	  }
	  if (refillTopBin) {
		fillBin(topBin, dataPtrs);
	  }

	  if ((bottomOverflowBin.value + bottomBin.value)/nDataPts < discardThreshold) {
		// threshold not reached: merge with overflow bin
		// set bin to Null to indicate that it must be readjusted
		bottomOverflowBin.value += bottomBin.value;
		bottomBin.value = 0;
		bottomBin.interval.setMinValue(bottomBin.interval.maxValue());
	  }
	  if ((topOverflowBin.value + topBin.value)/nDataPts < discardThreshold) {
		topOverflowBin.value += topBin.value;
		topBin.value = 0;
		topBin.interval.setMaxValue(topBin.interval.minValue());
	  }

	  if (not (topBin.interval.isNull() or bottomBin.interval.isNull())) {
		// exit loop
		refillBottomBin = false;
		refillTopBin = false;
	  } else {
		// readjust bin limits
		binWidth = (topBin.interval.maxValue() - bottomBin.interval.minValue())/nbins;
		if (bottomBin.interval.isNull()) {
          bottomBin.interval.setMaxValue(bottomBin.interval.minValue() + binWidth);
		  refillBottomBin = true;  // technically unnecessary
		} else {
		  refillBottomBin = false;
		}
		if (topBin.interval.isNull()) {
            topBin.interval.setMinValue(topBin.interval.maxValue() - binWidth);
		  refillTopBin = true;
		} else {
		  refillTopBin = false;
		}
	  }
	} // end of while


	// fill bins vector
	bins.push_back(bottomOverflowBin);
	bins.push_back(bottomBin);
    for(int i=0; i < nbins-2; ++i) {
	  // -2 because bottom and top bins are added outside the loop
	  bins.push_back(QwtIntervalSample(0, bottomBin.interval.maxValue() + i*binWidth, 
									   bottomBin.interval.maxValue() + (i+1)*binWidth));
	  fillBin(bins.back(), dataPtrs);
	}
	bins.push_back(topBin);
	bins.push_back(topOverflowBin);

    // finally add all that data to the histogram
    QwtPlotHistogram::setSamples(bins);
}

/*  Fill a bin with the count of values it contains,
	discarding used pointers so they aren't counted again uselessly.
	Modifies both arguments.
*/
void Histogram::fillBin(QwtIntervalSample& bin, std::vector<const double*>& dataPointers) {
//  std::vector<std::vector<const double*>::iterator> itrsToDelete;

  // Count all the values that fall within this bin
  for(auto itr = dataPointers.begin(); itr != dataPointers.end(); ) {
    if (bin.interval.contains(**itr)) {
      // Increment bin value and remove from pointer list to keep from counting again
      ++bin.value;
      itr = dataPointers.erase(itr);
    } else {
      ++itr;
    }
  }

}
