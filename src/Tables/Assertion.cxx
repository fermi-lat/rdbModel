// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/Tables/Assertion.cxx,v 1.4 2004/03/27 01:41:38 jrb Exp $
#include "rdbModel/Tables/Assertion.h"
#include "rdbModel/Tables/Table.h"
#include "rdbModel/Tables/Column.h"
#include "rdbModel/RdbException.h"

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
  

  /// Throw exception if Operator is not a comparison operator
  const std::string* Assertion::Operator::getCompareArgs() const {
    if (!isCompareOp()) 
      throw RdbException("Assertion::Operator::getCompareArgs: wrong type");
    return &m_compareArgs[0];
  }

  /// Throw exception if Operator is not a comparison operator
  const bool* Assertion::Operator::getLiteralness() const {
    if (!isCompareOp()) 
      throw RdbException("Assertion::Operator::getLiteralness: wrong type");
    return &m_literal[0];
  }

  /// Throw exception if Operator is a comparison operator
  const std::vector<Assertion::Operator* >& 
  Assertion::Operator::getChildren() const {
    if (isCompareOp()) 
      throw RdbException("Assertion::Operator::getChildren: wrong type");
    return m_operands;
  }

  Visitor::VisitorState  Assertion::accept(Visitor* v) {
    return v->visitAssertion(this);
  }

}
