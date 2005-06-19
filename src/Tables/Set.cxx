// $Header: $

#include "rdbModel/Tables/Set.h"

// more includes??

namespace rdbModel {

  Visitor::VisitorState Set::accept(Visitor* v) {    
    Visitor::VisitorState state = v->visitSet(this);
    if (state == Visitor::VBRANCHDONE) return Visitor::VCONTINUE;
    return state;
  }
}

