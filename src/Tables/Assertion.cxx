// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/Tables/Assertion.cxx,v 1.6 2004/03/30 23:58:12 jrb Exp $
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

  // Constructor for comparisons
  Assertion::Operator::Operator(OPTYPE type, const std::string& leftArg, 
                                const std::string& rightArg, bool leftLiteral, 
                                bool rightLiteral) :  m_opType(type)  {
    m_tableName.clear();
    m_operands.clear();
    if (!isCompareOp()) {
      m_opType = OPTYPEundefined;
      return;
    }

    m_compareArgs[0] = leftArg;
    m_compareArgs[1] = rightArg;
    m_literal[0] = leftLiteral;
    m_literal[1] = rightLiteral;
  }

  /// Constructor for EXISTS
  Assertion::Operator::Operator(OPTYPE type, const std::string& tableName,
                                Operator* child) : m_opType(type),
                                                   m_tableName(tableName)
  {
    if (type != OPTYPEexists) {
      m_opType = OPTYPEundefined;
      return;
    }
    m_operands.clear();
    m_operands.push_back(child);
  }

  /// Constructor for OR, AND, NOT
  Assertion::Operator::Operator(OPTYPE type, 
                                const std::vector<Operator*>& children)  :
    m_opType(type) {
    
    if ((type == OPTYPEor) || (type == OPTYPEand) || (type = OPTYPEnot)) {
      m_tableName.clear();
      unsigned int nChild = children.size();
      if (!nChild) {
        m_opType = OPTYPEundefined;
        return;
      }
      for (unsigned int iChild = 0; iChild < nChild; iChild++) {
        m_operands.push_back(children[iChild]);
      }
    }
    else m_opType = OPTYPEundefined;
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

  /// Throw exception if Operator is not EXISTS
  const std::string& Assertion::Operator::getTableName() const {
    if (m_opType != OPTYPEexists) 
      throw RdbException("Assertion::Operator::getTableNmae: wrong type");
    return m_tableName;
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
