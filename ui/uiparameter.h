#ifndef UIPARAMETER_H
#define UIPARAMETER_H

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>

#include "../integrators/io.h"

namespace frantic {

  // \todo: Add constructor taking a standard Parameter (non-UI)
  //        This would allow using standard Parameters to build UI
  //        Note: differential might expect to maintain link to original Parameters
  template <typename T>
  class UIParameter : public Parameter<T>
  {
  protected:
    using super = Parameter<T>;

    QLabel* labelWidget;

  public:
    UIParameter(const std::string& key,
              const std::string display_str, const T& value, const bool modifiable=true)
      : Parameter<T>(key, display_str, value, modifiable) { }

    virtual T get () {
      return super::value;
    }
    void set(const QString& string) {return ;}  // Defined in specializations below
    QString get_QString() {return QString("");}            // Defined in specializations below

    void attach(QGridLayout* layout) {
      assert(false);  // Should never be called;
      // compilation fails with "undefined reference" if I make this a pure virtual
    }

    virtual void repaint() {
      labelWidget->repaint();
    }

  };

  template <>
  inline void UIParameter<int>::set(const QString& string) {
    super::value = string.toInt();
  }
  template <>
  inline void UIParameter<long>::set(const QString& string) {
    super::value = string.toLong();
  }
  template <>
  inline void UIParameter<unsigned long>::set(const QString& string) {
    super::value = string.toULong();
  }
  template <>
  inline void UIParameter<double>::set(const QString& string) {
    super::value = string.toDouble();
  }
  template <>
  inline void UIParameter<std::string>::set(const QString& string) {
    super::value = string.toStdString();
  }

  // \todo: Compactify this with SFINAE
  template <>
  inline QString UIParameter<int>::get_QString() {
    return QString::number(super::value);
  }
  template <>
  inline QString UIParameter<long>::get_QString() {
    return QString::number(super::value);
  }
  template <>
  inline QString UIParameter<unsigned long>::get_QString() {
    return QString::number(super::value);
  }
  template <>
  inline QString UIParameter<double>::get_QString() {
    return QString::number(super::value);
  }
  template <>
  inline QString UIParameter<std::string>::get_QString() {
    return QString::fromUtf8(super::value.c_str());
  }

  template <typename T>
  class InfoUIParameter : public UIParameter<T>
  {
  protected:
    QLabel* contentWidget;

  public:
    using super = UIParameter<T>;
    using super::labelWidget;
    using super::super::display_str;
    using super::super::key;
    using super::super::value;
    using super::super::modifiable;

    InfoUIParameter(const std::string& key, const std::string display_str,
                    const T& value, const bool modifiable=true)
      : UIParameter<T>(key, display_str, value, modifiable) {
      int a=1;
    }

    void attach(QGridLayout* layout) {
      int row = layout->rowCount();
      labelWidget = new QLabel((QString::fromUtf8(display_str.c_str())));
      contentWidget = new QLabel(super::get_QString());
      layout->addWidget(labelWidget, row, 0);
      layout->addWidget(contentWidget, row, 1);
    }

    InfoUIParameter& operator= (const T new_value) {
      value = new_value;
      contentWidget->setText(super::get_QString());
    }

    virtual void repaint() {
      contentWidget->repaint();
      super::repaint();
    }

  };


  template <typename T>
  class InputUIParameter : public UIParameter<T>
  {
  protected:
    QLineEdit* contentWidget;

  public:
    using super = UIParameter<T>;
    using super::labelWidget;
    using super::super::display_str;
    using super::super::key;
    using super::super::value;
    using super::super::modifiable;

    InputUIParameter(const std::string& key,
              const std::string display_str, const T& value, const bool modifiable=true)
      : UIParameter<T>(key, display_str, value, modifiable) { }

    virtual T get () {
      refresh();
      return super::get();
    }

    void attach(QGridLayout* layout) {
      int row = layout->rowCount();
      labelWidget = new QLabel((QString::fromUtf8(display_str.c_str())));
      contentWidget = new QLineEdit;
      contentWidget->setText(super::get_QString());
      layout->addWidget(labelWidget, row, 0);
      layout->addWidget(contentWidget, row, 1);
    }

    virtual void repaint() {
      contentWidget->repaint();
      super::repaint();
    }

    // Always call this before pulling UI values, in case user changed boxes
    void refresh() {
      UIParameter<T>::set(contentWidget->text());
    }

    InputUIParameter& operator= (const T new_value) {
      value = new_value;
      contentWidget->setText(super::get_QString());
    }

  };



  /* HACK : mostly a copy of ParameterTuple with additional functions
   * \todo: use template template parameter to subclass ParameterTuple
   *        - parent's params must be declared as UIParameterTuple
   *        - must not be duplication of param or params
   * \todo: overload get functions
   * \todo: deduce store_internally from constructor overload somehow ?
   */
  template <bool store_internally, typename ...Params>
  struct UIParameterTuple;


  template <bool store_internally, typename Param, typename ...Params>
  struct UIParameterTuple<store_internally, Param, Params...>
  {
  protected:
    // If parameter is passed as an lvalue reference, it already exists =>
    // user expects to be able to modify it directly, so we save a reference.
    // Otherwise, we create an instance of the variable, because it has to stay
    // persistent for the life of the parameter tuple
    typename std::conditional<store_internally, Param, Param&>::type param;
    UIParameterTuple<store_internally, Params...> params;

  public:

    const bool empty = false;

    // Parameters should derive from UIParameter
    UIParameterTuple(Param& param, Params& ...params)
      : param(param), params(params...) {
      assert(!store_internally);  // If parameter object exists outside of tuple's scope,
                                  // we don't want to create a duplicate
    }
    UIParameterTuple(Param&& param, Params&& ...params)
      : param(std::move(param)), params(std::move(params)...) {
      assert(store_internally);  // Ensure we don't store a reference to a temporary => Bad Things
    }

    template <typename T>
    T get(const std::string& key) {
      if (param.key == key) {
        return param.get();
      } else if (!params.empty){
        return params.get<T>(key);
      } else {
        std::cerr << "No parameter has key " << key;
        assert(false);
        return 0;
      }
    }

    template<typename T>
    void update(const std::string& key, const T new_value) {
      if (param.key == key) {
        param = new_value;
      } else if (!params.empty){
        params.update(key, new_value);
      } else {
        std::cerr << "No parameter has key " << key;
        assert(false);
      }
    }

    void print() {
      std::cout << param.display_str << " : " << param.get() << std::endl;
      if (!params.empty) {
        params.print();
      }
    }

    void attach(QGridLayout* layout) {
      param.attach(layout);
      params.attach(layout);
    }

    void refresh() {
      param.refresh();
      params.refresh();
    }
    void repaint() {
      param.repaint();
      params.repaint();
    }

  };

  template <bool store_internally>
  struct UIParameterTuple<store_internally> {
    const bool empty = true;

    template <typename T>
    T get(const std::string&) {assert(false); return T();} // Should never execute
    template <typename T>
    void update(const std::string&, const T) {assert(false);} // ditto
    void attach(QGridLayout*) {return;}
    void refresh() {return;}
    void repaint() {return;}
  };

}
#endif // UIPARAMETER_H
