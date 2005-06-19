// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/Tables/Assertion.cxx,v 1.11 2005/02/15 22:43:41 jrb Exp $
#include "rdbModel/Rdb.h"
#include "rdbModel/Tables/Assertion.h"
#include "rdbModel/Tables/Table.h"
#include "rdbModel/Tables/Column.h"
#include "rdbModel/RdbException.h"

namespace rdbModel {

  Assertion::Operator::~Operator() {
    if (!m_keepChildren) {
      while (m_operands.size() ) {
        Operator* op = m_operands.back();
        m_operands.pop_back();
        delete op;
      }
    }
  }

  Assertion::~Assertion() {
    if (!m_keepOp) delete m_op;
  }

  // Constructor for comparisons
  // It's also used for isNull, but in that case rightArg (and rightLiteral)
  // are ignored.  They get stuffed into object, then never used.
  Assertion::Operator::Operator(OPTYPE type, const std::string& leftArg, 
                                const std::string& rightArg, 
                                FIELDTYPE leftLiteral, 
                                FIELDTYPE rightLiteral) 
    :  m_opType(type), m_keepChildren(false), m_toBe(false), m_old(false) {

    m_tableName.clear();
    m_operands.clear();
    if (!isCompareOp()) {
      m_opType = OPTYPEundefined;
      return;
    }

    m_compareArgs[0] = leftArg;
    m_compareArgs[1] = rightArg;
    m_compareType[0] = leftLiteral;
    m_compareType[1] = rightLiteral;
    m_toBe = ((leftLiteral == FIELDTYPEtoBe) || 
              (rightLiteral == FIELDTYPEtoBe));
    m_old = ((leftLiteral == FIELDTYPEold) || 
             (rightLiteral == FIELDTYPEold));
  }

  /// Constructor for EXISTS
  Assertion::Operator::Operator(OPTYPE type, const std::string& tableName,
                                Operator* child) : m_opType(type),
                                                   m_tableName(tableName),
                                                   m_keepChildren(false)
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
                                const std::vector<Operator*>& children,
                                bool keepChildren)  :
    m_opType(type), m_keepChildren(keepChildren) {

    m_toBe = false;
    m_old = false;
    if ((type == OPTYPEor) || (type == OPTYPEand) || (type = OPTYPEnot)) {
      m_tableName.clear();
      unsigned int nChild = children.size();
      if (!nChild) {
        m_opType = OPTYPEundefined;
        return;
      }
      for (unsigned int iChild = 0; iChild < nChild; iChild++) {
        m_toBe |= (children[iChild]->m_toBe);
        m_old |= (children[iChild]->m_old);
        m_operands.push_back(children[iChild]);
      }
    }
    else m_opType = OPTYPEundefined;
  }

  // This only makes sense for conjunction-style operators AND, OR
  bool Assertion::Operator::appendChild(Operator* child) {
    m_toBe |= child->m_toBe;
    m_old |= child->m_old;
    if  ((m_opType == OPTYPEor) || (m_opType == OPTYPEand) ) {
      m_operands.push_back(child);
      return true;
    }
    else if ((m_opType == OPTYPEnot) && (m_operands.size() == 0) ) {
      m_operands.push_back(child);
      return true;
    }
    throw RdbException("Assertion::Operator::appendChild: wrong parent operator type");
    return false;
  }


  bool Assertion::Operator::validCompareOp(Table* myTable) const {
    if (m_compareType[0] != FIELDTYPElit) {
      Column* col0 = myTable->getColumnByName(m_compareArgs[0]);
      if (m_compareType[1] != FIELDTYPElit) {
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
  //  const bool* Assertion::Operator::getLiteralness() const {
  const FIELDTYPE* Assertion::Operator::getCompareType() const {
    if (!isCompareOp()) 
      throw RdbException("Assertion::Operator::getLiteralness: wrong type");
    return &m_compareType[0];
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
    Visitor::VisitorState state = v->visitAssertion(this);
    if (state == Visitor::VBRANCHDONE) return Visitor::VCONTINUE;
    return state;
  }

}
