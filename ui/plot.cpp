#include "plot.h"

namespace frantic {

  Plot::Plot(QWidget *parent) :
      QwtPlot(parent)
  {
  }

  /* Remove all items from the plot
   * WARNING: the items are not only detached, but their corresponding resources are also deleted.
   */
  void Plot::clear()
  {
    QwtPlotItemList itemList(this->itemList());
    QVector<QwtPlotItem*> itemPtrs;
    itemPtrs.reserve(itemList.size());

    foreach (QwtPlotItem* item, itemList) {
      itemPtrs.push_back(item);
    }
    foreach (QwtPlotItem* item, itemPtrs) {
      item->detach();
      delete item;
    }
  }

  /* \todo: Implement function that detaches items and returns them as a list, instead of deleting the resources ?
   */

}
