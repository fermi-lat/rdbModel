// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/Tables/Assertion.cxx,v 1.2 2004/03/07 08:21:26 jrb Exp $
#include "rdbModel/Tables/Assertion.h"
#include "rdbModel/Tables/Table.h"
#include "rdbModel/Tables/Column.h"

namespace rdbModel {

  Assertion::Operator::~Operator() {
    while (m_operands.size() ) {
      Operator* op = m_operands.back();
      m_operands.pop_back();
      delete op;
    }
  }

  Assertion::~Assertion() {
    delete m_op;
  }

  bool Assertion::Operator::validCompareOp(Table* myTable) const {
    if (!m_literal[0]) {
      Column* col0 = myTable->getColumnByName(m_compareArgs[0]);
      if (!m_literal[1]) {
        Column* col1 = myTable->getColumnByName(m_compareArgs[1]);
        return col1->isCompatible(col0);
      }
      else {  // one column, one literal
        return col0->okValue(m_compareArgs[1], false);
      }
    }
    else { // 1st arg is a literal; second arg must be column
      Column* col1 = myTable->getColumnByName(m_compareArgs[1]);
      return col1->okValue(m_compareArgs[0], false);        
    }
  }
  

  Visitor::VisitorState  Assertion::accept(Visitor* v) {
    return v->visitAssertion(this);
  }

}
