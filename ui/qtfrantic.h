#ifndef QTFRANTIC_H
#define QTFRANTIC_H

#include <QColor>
#include <QTime>
#include <QCoreApplication>

#include <random>

/* Short functions and classes of general use for QtFRANTIC */

namespace frantic {

  // inlines required because in header file; maybe could just move everything to .cpp instead ?

  inline QColor randomColor() {
    static std::minstd_rand r_engine;
    static std::uniform_real_distribution<> rand_rgb(0, 255);
    return QColor(rand_rgb(r_engine), rand_rgb(r_engine), rand_rgb(r_engine));
  }

  /* Delay 'amount'-seconds; default is 1 second.
   * This function does not freeze GUI.
   * http://stackoverflow.com/a/11487434
   */
  inline void delay(int amount = 1) {
    QTime dieTime= QTime::currentTime().addSecs(amount);
    while( QTime::currentTime() < dieTime )
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
  }
}

#endif // QTFRANTIC_H
