// $Header:  $
#ifndef RDBMODEL_RDBEXCEPTION_H
#define RDBMODEL_RDBEXCEPTION_H
#include <exception>

namespace rdbModel {

  class RdbException : std::exception {
  public:
    RdbException(const std::string& extraInfo = "") : std::exception(),
      m_name("RdbException"), m_extra(extraInfo) {}
    virtual ~RdbException() throw() {}
    virtual std::string getMsg() {
      std::string msg = m_name + ": " + m_extra;
      return msg;}
    virtual const char* what() {
      return m_extra.c_str();
    }
  protected: 
    std::string m_name;
  private:
    std::string m_extra;
  };

}
#endif
