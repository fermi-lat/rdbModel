// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/Tables/Index.cxx,v 1.1 2004/03/06 01:15:25 jrb Exp $

#include "rdbModel/Tables/Index.h"

namespace rdbModel {

  Visitor::VisitorState  Index::accept(Visitor * v) {
    return v->visitIndex(this);
  }

  const std::vector<std::string>& Index::getColumnNames() {
    return m_indexCols;
  }


}
