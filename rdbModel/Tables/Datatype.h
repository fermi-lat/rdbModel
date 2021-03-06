// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/rdbModel/Tables/Datatype.h,v 1.14 2006/12/07 18:03:41 jrb Exp $
#ifndef RDBMODEL_DATATYPE_H
#define RDBMODEL_DATATYPE_H
#include <vector>
#include <string>
#include "rdbModel/Management/Visitor.h"

namespace rdbModel{

  class XercesBuilder;

  /// Extra little class for datatypes with values (preferred or
  /// required) coming from an enumerated list
  class Enum {
  public: 
    const std::vector<std::string>& getChoices() const {
      return m_choices;}
    bool choicesRequired() const {return m_required;}

  private:
    friend class rdbModel::XercesBuilder;
    std::vector<std::string> m_choices;
    // sometimes column *must* have one of the enumerated values; 
    // other times they're just suggestions
    bool m_required;  
  };

  class Datatype {
  public:
    // Include MySQL-supported types we might conceivably use
    enum TYPES {
      TYPEnotFound = -1,
      TYPEenum = 0,
      TYPEdatetime,
      TYPEtimestamp,
      TYPEint,
      TYPEmediumint,
      TYPEsmallint,
      TYPEtinyint,
      TYPEfloat,
      TYPEdouble,
      TYPEvarchar,
      TYPEchar,
      TYPEintUnsigned,
      TYPEmediumintUnsigned,
      TYPEsmallintUnsigned,
      TYPEtinyintUnsigned,
      TYPEtinyblob,
      TYPEblob,
      TYPEmediumblob,
      TYPElongblob,
      TYPEtinytext,
      TYPEtext,
      TYPEmediumtext,
      TYPElongtext
    };
    enum RESTRICT {
      RESTRICTnone = 0,
      RESTRICTnonneg,   // value must be non-negative
      RESTRICTpos,      // value must be strictly positive
      RESTRICTinterval, // value (numeric or datetime) must be in interval    
      RESTRICTenum,     // restrict to, or suggest, enumerated set
      RESTRICTfile      // value must have valid file path syntax
    };

    Datatype() : m_restrict(RESTRICTnone), m_enum(0), m_isInt(false),
          m_isUnsigned(false)  {}
    ~Datatype() {if (m_enum) delete m_enum;}
    /// Check that supplied string has proper syntax for our type and
    /// is in accord with restriction, if any
    bool okValue(const std::string& val) const;
    bool isCompatible(const Datatype* other) const;
    bool isUnsigned() {return m_isUnsigned;}
    bool isText() const {return ((m_type == TYPEtinytext) ||
                                 (m_type == TYPEtext) ||
                                 (m_type == TYPEmediumtext) ||
                                 (m_type == TYPElongtext) ); }
                           
                           
    bool isBlob() const {return ((m_type == TYPEtinyblob) ||
                                 (m_type == TYPEblob) ||
                                 (m_type == TYPEmediumblob) ||
                                 (m_type == TYPElongblob) ); }
                           
    TYPES getType() const {return m_type;}
    int   getOutputSize() const {return m_outputSize;}

    /// Return pointer to Enum object owned by datatype (if none, return
    /// null pointer).
    Enum* getEnum() const {return m_enum;}
    RESTRICT getRestrict() const {return m_restrict;}

    /** User-supplied strings min and max will be set to min and max values,
        if any, for the Datatype object.  
        @ret  true if there is a min & max, false otherwise
    */
    bool getInterval(std::string& min, std::string& max);


  private:
    friend class rdbModel::XercesBuilder;

    // Bring all internal data specific to type up to date.  
    // Return type discovered, or -1 if unrecognized
    int setType(std::string name, bool isUnsigned=false);

    // In case restriction type is RESTRICTinterval, bring internal data
    // up to date.  Return true if strings represent valid min and
    // max for column datatype
    bool setInterval(const std::string& min, const std::string& max);

    std::string m_typename;

    int m_outputSize;

    TYPES m_type;    // Do we want this or is string rep. good enough?
    RESTRICT m_restrict;
    // which, if any, of the following have interesting contents depends
    // on value of m_restrict
    Enum* m_enum;
    std::string m_min;   // form of these depends on value of m_type
    std::string m_max;   
    bool m_isInt;
    bool m_isUnsigned;
    int m_minInt;         // applies only to integer signed types
    int m_maxInt; 
    unsigned int m_minIntUnsigned;  // applies only to integer unsigned
    unsigned int m_maxIntUnsigned;

  };


}
#endif

