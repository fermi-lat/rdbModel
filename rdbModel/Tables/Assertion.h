// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/rdbModel/Tables/Assertion.h,v 1.1.1.1 2004/03/03 01:57:04 jrb Exp $
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
    Assertion();
    ~Assertion();
    Visitor::VisitorState accept(Visitor* v);
    //    Visitor::VisitorState acceptNotRec(Visitor* v);

    bool execute();

  private:
    friend XercesBuilder;

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
      Operator();
      ~Operator();

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

