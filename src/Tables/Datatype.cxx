// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/Tables/Datatype.cxx,v 1.1 2004/03/05 01:38:52 jrb Exp $

#include "rdbModel/Tables/Datatype.h"
#include "facilities/Util.h"

namespace {

  bool initDone = false;
  const int N_SUPPORTED_TYPES = Datatype::TYPEchar + 1;
  std::string typenames[N_SUPPORTED_TYPES];
  void init() {
    if (!initDone) {
      typenames[TYPEenum] = std::string("enum");
      typenames[TYPEdatetime] = std::string("datetime");
      typenames[TYPEtimestamp] = std::string("timestamp");
      typenames[TYPEint] = std::string("int");
      typenames[TYPEmediumint] = std::string("mediumint");
      typenames[TYPEsmallint] = std::string("smallint");
      typenames[TYPEreal] = std::string("real");
      typenames[TYPEdouble] = std::string("double");
      typenames[TYPEvarchar] = std::string("varchar");
      typenames[TYPEchar] = std::string("char");
      initDone = true;
    }
  }
  int findType(std::string aType) {
    for (unsigned i = 0; i < N_SUPPORTED_TYPES; i++) {
      if (aType == typenames[i]) return i;
    }
    return -1;
  }
}

namespace rdbModel {

  int Datatype::setType(std::string name) {
    m_type = findType(name);
    if (m_type >=0 ) {
      m_typename = name;
    }

    // Set up initial restrictions for integer-like types.  These
    // values come from the MySQL doc.  See e.g. 
    // http://www.mysql.com/documentation/mysql/bychapter/manual_Column_types.html#Numeric_types
    switch (m_type) {
      case TYPEint: {
        m_maxInt = 2147483647;
        m_minInt = -2147483648;
        m_isInt = true;
        break;
      }
      case TYPEmediumint: {
        m_maxInt = 8388607;
        m_minInt = -8388608;
        m_isInt = true;
        break;
      }
      case TYPEsmallint: {
        m_maxInt = 32767;
        m_minInt = -32768;
        m_isInt = true;
        break;
      }
    default:
      break;
    }
    return m_type;
  }

  // Makes sense only for numeric types or for datetime
  bool Datatype::setInterval(const std::string& min, const std::string& max) {
    bool ret = false;
    if ((m_type == TYPEtimestamp) || (m_type == TYPEenum) || 
        (m_type == TYPEchar) || (m_type == TYPEvarchar)) {
      std::cerr << "From rdbModel::Datatype::setInterval " << std::endl;
      std::cerr << "Cannot set interval restriction for type " <<
        typenames[m_type} << std::endl;
      return false;
    }

    m_restrict = RESTRICTinterval;
    m_min = min;
    m_max = max;

    // In case data is some integer type, convert min and max to integers,
    // store.  Return false if anything about them is unreasonable.
    if (m_isInt) {
      try {
        int minInt = facilities::Util::stringToInt(min);
        int maxInt = facilities::Util::stringToInt(max);
        if (minInt > newType->m_min) newType->m_min = minInt;
        if (maxInt < newType->m_max) newType->m_max = maxInt;
      }
      catch (facilities::WrongType ex) {
        std::cerr << "Error detected in XercesBuilder::buildDatatype " 
                  << std::endl;
        std::cerr << ex.getMsg() << std::endl;
        return false;
      }
      ret = (m_min < m_max);
    }
    // Don't expect to encounter interval restriction for floating point,
    // so don't bother to cache values, but at least check for validity
    if ((m_type == TYPEdouble) || (m_type == TYPEreal)) {
      try {
        double minFloat = facilities::Util::stringToDouble(min);
        double maxFloat = facilities::Util::stringToDouble(max);
      }
      catch (facilities::WrongType ex) {
        std::cerr << "Error detected in XercesBuilder::buildDatatype " 
                  << std::endl;
        std::cerr << ex.getMsg() << std::endl;
        return false;
      }
      ret =  (minFloat < maxFloat);
    }
    if (m_type == TYPEdatetime) {
      try {
        facilities::timestamp minTime(min);
        facilities::timestamp maxTime(max);
        ret =  (minTime < maxTime);
      }
      catch (facilities::BadTimeInput ex) {
        std::cerr << "From rdbModel::Datatype::setInterval" << std::endl;
        std::cerr << ex.complaint << std::endl;
        return false;
      }
    }
    return ret;
  }

  bool Datatype::okValue(const std::string& val) {
    using facilities::Util;

    switch (m_type) {
    case TYPEreal:
    case TYPEdouble: {
      double doubleVal;
      try {
        doubleVal = Util::stringToDouble(val);
      }
      catch (facilities::WrongType ex) {
        return false;
      }
      if (m_restrict == RESTRICTnonneg) return (doubleVal  >= 0.0 );
      else if (m_restrict == RESTRICTpos) return (doubleVal > 0.0);
      else if (m_restrict == RESTRICTinterval) {
        double min = Util::stringToDouble(m_min);
        double max = Util::stringToDouble(m_max);
        return ((min <= doubleVal) && (doubleVal <= max));
      }
      return true;

    }

    case TYPEint:
    case TYPEmediumint:
    case TYPEsmallint: {
      int intVal;

      try {
        intVal = Util::stringToInt(val);
      }
      catch (facilities::WrongType ex) {
        return false;
      }
      return ((intVal >= m_minInt) && (intVal <= m_maxInt));
    }
    case TYPEvarchar:
    case TYPEchar:
      if (m_restrict == RESTRICTnone) return true;
      if (!m_restrict->m_enum->m_required) return true;
    case TYPEenum: 
      unsigned nChoice = m_restrict->m_enum->m_choices.size();
      for (unsigned i = 0; i < nChoice; i++) {
        if (val == m_restrict->m_enum->m_choices[i]) return true;
      }
      return false;
    }
    case TYPEdatetime:
    case TYPEtimestamp: {
      try {
        facilities::Timestamp aTime(val);
        if (m_restrict == RESTRICTinterval) {
          facilities::Timestamp min(m_min);
          facilities::Timestamp max(m_min);
          return ((min <= aTime) && (aTime <= max));
        }
        return true;
      }
      catch (facilities::BadTimeInput ex) {
        std::cerr << "From rdbModel::Datatype::okValue" << std::endl;
        std::cerr << ex.complaint << std::endl;
        return false;
      }

    }
    default:
      return false;
  }

}
