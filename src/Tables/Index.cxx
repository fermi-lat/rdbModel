// $Header:  $

#include "rdbModel/Tables/Index.h"

namespace rdbModel {

  Visitor::VisitorState  Index::accept(Visitor *) {
    return v->visitIndex(this);
  }

  const std::vector<std::string>& Index::getColumnNames() {
    return m_indexCols;
  }


}
