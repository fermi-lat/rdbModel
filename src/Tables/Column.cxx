// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/Tables/Column.cxx,v 1.1 2004/03/05 01:38:52 jrb Exp $

#include "rdbModel/Tables/Column.h"
#include "rdbModel/Tables/Datatype.h"

namespace rdbModel {

  Column::~Column() {
    delete m_type;
    delete m_source;
  }

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

  Visitor::VisitorState Column::accept(Visitor* v) {

    return v->visitColumn(this);
  }

}
