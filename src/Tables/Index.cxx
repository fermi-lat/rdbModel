// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/Tables/Index.cxx,v 1.3 2004/04/02 03:05:05 jrb Exp $

#include "rdbModel/Tables/Index.h"

namespace rdbModel {

  Visitor::VisitorState  Index::accept(Visitor * v) {
    Visitor::VisitorState state = v->visitIndex(this);
    if (state == Visitor::VBRANCHDONE) return Visitor::VCONTINUE;
    return state;
  }

  const std::vector<std::string>& Index::getColumnNames() {
    return m_indexCols;
  }


}
