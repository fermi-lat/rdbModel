// $Header:   $

#include "rdbModel/Tables/Column.h"
#include "rdbModel/Tables/Datatype.h"

namespace rdbModel {

  Column::~Column {
    delete m_type;
    delete m_source;
  }

  bool Column::okValue(const std::string& val, bool set) const {
    // auto increment and datetime values are established by rdbms
    if (set) {
      if ((m_from == FROMautoIncrement) || (m_from == FROMnow)) return false;
    }

    return m_type->okValue(val);
  }

  Visitor::VisitorState Column::accept(Visitor* v) {

    return = v->visitColumn(this);
  }

}
