// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/Tables/Column.cxx,v 1.5 2004/04/07 23:06:49 jrb Exp $

#include "rdbModel/Tables/Column.h"
#include "rdbModel/Tables/Datatype.h"

namespace rdbModel {

  Column::~Column() {
    delete m_type;
  }

  Enum* Column::getEnum() const {return m_type->getEnum();}

  bool Column::okValue(const std::string& val, bool set) const {
    // auto increment and datetime values are established by rdbms
    if (set) {

      if ( (m_from == FROMautoIncrement) || 
           (m_from == FROMnow)) return false;
    }

    return m_type->okValue(val);
  }

  bool Column::isCompatible(const Column* otherCol) const {
    return m_type->isCompatible(otherCol->m_type);
  }

  bool Column::isAutoIncrement() const {
    return (m_from == FROMautoIncrement);
  }

  Visitor::VisitorState Column::accept(Visitor* v) {

    Visitor::VisitorState state = v->visitColumn(this);
    if (state == Visitor::VBRANCHDONE) return Visitor::VCONTINUE;
    return state;
  }

}
