// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/Tables/Column.cxx,v 1.3 2004/03/28 08:25:22 jrb Exp $

#include "rdbModel/Tables/Column.h"
#include "rdbModel/Tables/Datatype.h"

namespace rdbModel {

  Column::~Column() {
    delete m_type;
    delete m_source;
  }

  Enum* Column::getEnum() const {return m_type->getEnum();}

  bool Column::okValue(const std::string& val, bool set) const {
    // auto increment and datetime values are established by rdbms
    if (set) {
      if ((m_source->m_from == ColumnSource::FROMautoIncrement) || 
          (m_source->m_from == ColumnSource::FROMnow)) return false;
    }

    return m_type->okValue(val);
  }

  bool Column::isCompatible(const Column* otherCol) const {
    return m_type->isCompatible(otherCol->m_type);
  }

  bool Column::isAutoIncrement() const {
    return (getSource()->m_from == ColumnSource::FROMautoIncrement);
  }

  Visitor::VisitorState Column::accept(Visitor* v) {

    Visitor::VisitorState state = v->visitColumn(this);
    if (state == Visitor::BRANCHDONE) return Visitor::CONTINUE;
    return state;
  }

}
