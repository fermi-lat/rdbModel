// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/rdbModel/Tables/Assertion.h,v 1.2 2004/03/04 01:07:11 jrb Exp $
#ifndef RDBMODEL_ASSERTION_H
#define RDBMODEL_ASSERTION_H
#include <vector>
#include <string>
#include "rdbModel/Management/Visitor.h"

namespace rdbModel{

  class XercesBuilder;
  class Table;

  class Assertion {
    class Operator;         // nested class declared below

  public:
    // when does this assertion get applied?
    enum WHEN {
      WHENglobalCheck = 1,
      WHENchangeRow
    };
    Assertion(Table* myTable=0) : m_op(0), m_myTable(myTable) 
    { m_sqlEquivalent.clear();};
    ~Assertion();
    WHEN getWhen() const {return m_when;}
    Visitor::VisitorState accept(Visitor* v);

    const std::string& getSql() const {return m_sqlEquivalent;}
    
    //    Visitor::VisitorState acceptNotRec(Visitor* v);


  private:
    friend class rdbModel::XercesBuilder;

    WHEN m_when;
    Operator* m_op;
    Table* m_myTable;

    // parse op tree in constructor; save equivalent SQL expression
    void makeSql();
    std::string m_sqlEquivalent;  

    enum OPTYPE {
      OPTYPEor = 1,
      OPTYPEand,
      OPTYPEisNull,
      OPTYPEexists,
      OPTYPEnot,
      OPTYPEequal,           // first of compare ops
      OPTYPEnotEqual,
      OPTYPElessThan,
      OPTYPElessOrEqual
    };

    // Compare operators take two arguments.  One must be a column
    // name, the other may be a column name or a constant
    // Other operators take 1 or more other operators as arguments
    class Operator {
    public:
      Operator() {};
      ~Operator();

      /// Check whether columns or column and literal to be compared
      /// have compatible types
      bool validCompareOp(Table* table) const;

      OPTYPE m_opType;

      // Following two lines apply only to compare operators
      // In order to format properly in an SQL query, need to
      // keep track of whether compare arg is literal or column name.
      std::string m_compareArgs[2];
      bool m_literal[2];     

      // Following is used only for non-compare operators
      std::vector<Operator* > m_operands;  // #allowed depends on opType
    };
      
  };
}
#endif

