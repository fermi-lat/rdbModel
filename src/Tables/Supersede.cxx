// $Header:   $

#include "rdbModel/Tables/Set.h"
#include "rdbModel/Tables/Supersede.h"
#include "rdbModel/RdbException.h"

namespace rdbModel {

  Supersede::Supersede(Table* table, Assertion* onlyIf) : 
    m_myTable(table), m_onlyIf(onlyIf) {
    m_setOld.clear();
    m_setNew.clear();
  }

  void Supersede::addSet(Set* s) {
    FIELDTYPE vtype = s->getDestType();
    switch(vtype) {
    case FIELDTYPEold: 
    case FIELDTYPEoldDef:    // this shouldn't occur
      m_setOld.push_back(s); break;
    case FIELDTYPEtoBe: 
    case FIELDTYPEtoBeDef:    // nor this
      m_setNew.push_back(s); break;
    default: throw RdbException("Bad value type for <set> destination");
    }
    return;
  }

  Supersede::~Supersede() {
    while (m_setOld.size() ) {
      Set* s  = m_setOld.back();
      m_setOld.pop_back();
      delete s;
    }
    while (m_setNew.size() ) {
      Set* s  = m_setNew.back();
      m_setNew.pop_back();
      delete s;
    }
    //    delete m_onlyIf;    probably already handled by ~Table
  }
  // For now assume visitSupersede will handle everything. Don't need
  // separate visitSet method.
  Visitor::VisitorState Supersede::accept(Visitor* v) {
    Visitor::VisitorState state = v->visitSupersede(this);
    if (state == Visitor::VBRANCHDONE) return Visitor::VCONTINUE;
    return state;
  }
}
