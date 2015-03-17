#include "uiparameter.h"

namespace frantic {

  template <>
  int QString_to_value<int>(const QString& string) {
    return string.toInt();
  }
  template <>
  long QString_to_value<long>(const QString& string) {
    return string.toLong();
  }
  template <>
  unsigned long QString_to_value<unsigned long>(const QString& string) {
    return string.toULong();
  }
  template <>
  double QString_to_value<double>(const QString& string) {
    return string.toDouble();
  }
  template <>
  std::string QString_to_value<std::string>(const QString& string) {
    return string.toStdString();
  }

}
