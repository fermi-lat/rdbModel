// $Header:  $
#ifndef RDBMODEL_DATATYPE_H
#define RDBMODEL_DATATYPE_H
#include <vector>
#include <string>
#include "rdbModel/Management/Visitor.h"

namespace rdbModel{

  class XercesBuilder;

  class Datatype {
  public:
    // Include MySQL-supported types we might conceivably use
    enum TYPES {
      TYPEenum = 1,
      TYPEdatetime,
      TYPEtimestamp,
      TYPEint,
      TYPEmediumint,
      TYPEsmallint,
      TYPEreal,
      TYPEdouble,
      TYPEvarchar,
      TYPEchar
    };
    enum RESTRICT {
      RESTRICTnone = 0,
      RESTRICTnonneg,   // value must be non-negative
      RESTRICTpos,      // value must be strictly positive
      RESTRICTinterval, // value (numeric or datetime) must be in interval    
      RESTRICTenum,     // restrict to, or suggest, enumerated set
      RESTRICTfile      // value must have valid file path syntax
    };

    Datatype() : m_restrict(0), m_enum(0)  {}
    ~Datatype() {if (m_enum) delete m_enum;}
    /// Check that supplied string has proper syntax for our type and
    /// is in accord with restriction, if any
    bool okValue(const std::string& val);

    Visitor::VisitorState accept(Visitor* v);
    Visitor::VisitorState acceptNotRec(Visitor* v);

  private:    // embedded class since Enum restriction is a bit more complex
    class Enum {
    public: 
      std::vector<std::string&> m_choices;
      // sometimes column *must* have one of the enumerated values; 
      // other times they're just suggestions
      bool m_required;  
    };

  private:
    friend XercesBuilder;
    std::string m_typename;

    TYPES m_type;    // Do we want this or is string rep. good enough?
    RESTRICT m_restrict;
    // which, if any, of the following have interesting contents depends
    // on value of m_restrict
    Enum* m_enum;
    std::string m_min;   // form of these depends on value of m_type
    std::string m_max;   

  };
}
#endif

