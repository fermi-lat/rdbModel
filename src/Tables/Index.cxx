// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/Tables/Index.cxx,v 1.2 2004/03/07 08:21:26 jrb Exp $

#include "rdbModel/Tables/Index.h"

namespace rdbModel {

  Visitor::VisitorState  Index::accept(Visitor * v) {
    Visitor::VisitorState state = v->visitIndex(this);
    if (state == Visitor::BRANCHDONE) return Visitor::CONTINUE;
    return state;
  }

  const std::vector<std::string>& Index::getColumnNames() {
    return m_indexCols;
  }


}
