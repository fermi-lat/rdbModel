// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/rdbModel/Tables/Assertion.h,v 1.4 2004/03/24 02:05:08 jrb Exp $
#ifndef RDBMODEL_ASSERTION_H
#define RDBMODEL_ASSERTION_H
#include <vector>
#include <string>
#include "rdbModel/Management/Visitor.h"

namespace rdbModel{

  class XercesBuilder;
  class Table;


  /** 
      Assertions are used in at least two ways:
      1. As part of a table description.  The assertion describes a condition
         which should be evaluated upon a particular event, such as when
         a new element is to be inserted.  Such assertions stick around
         for the life of the application instance.  If the assertion is
         checked often, a pre-compiled version (dependent on the type
         of connection) can save some time.  
      2. As a WHERE clause in a client-institued UPDATE or SELECT. These
         are only around long enough to do the UPDATE or SELECT.

      The bulk of the information comprising an assertion is kept in a
      tree whose nodes are "operations".  An operation may be either 
      a comparison ( =, <=, etc. and also "is null") or an operation which
      has child operations:  OR, AND, NOT, for all, there exists, hence
      a node is a leaf node iff it's a comparison.
 
  */
  class Assertion {
    class Operator;         // nested class declared below

  public:
    // when does this assertion get applied?
    enum WHEN {
      WHENglobalCheck = 1,
      WHENchangeRow,
      WHENwhere     // as a WHERE clause, used and then discarded
    };
    Assertion(Table* myTable=0) : m_op(0), m_myTable(myTable) 
    { m_sqlEquivalent.clear();};
    ~Assertion();
    WHEN getWhen() const {return m_when;}
    Visitor::VisitorState accept(Visitor* v);

    Operator* getOperator() const {return m_op;}
    /// 
    const std::string& getPrecompiled() const {return m_compiled;}
    
    //    Visitor::VisitorState acceptNotRec(Visitor* v);


  private:
    friend class rdbModel::XercesBuilder;

    WHEN m_when;
    Operator* m_op;
    Table* m_myTable;

    /// Let's hope that, independent of connection type, std::string is
    /// a reasonable choice for "compiled" form of the assertion
    std::string m_compiled;  

    enum OPTYPE {
      OPTYPEor = 1,
      OPTYPEand,
      OPTYPEexists,
      OPTYPEforAll,
      OPTYPEnot,
      OPTYPEisNull,
      OPTYPEequal,           // first of 2-operand compare ops
      OPTYPEnotEqual,
      OPTYPElessThan,
      OPTYPEgreaterThan,
      OPTYPElessOrEqual,
      OPTYPEgreaterOrEqual
    };
    const int OPTYPElast = OPTYPgreaterOrEqual;

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

      /// True if operator is "isNull" or any of the usual arithmetic
      /// comparisons
      bool isCompareOp() const {return (m_opType >= OPTYPEisNull);}

      /// Throw exception if Operator is not a comparison operator
      const std::string[2]& getCompareArgs() const;

      /// Throw exception if Operator is not a comparison operator
      const bool[2]& getLiteralness() const;

      /// Throw exception if Operator is a comparison operator
      const std::vector<Operator* >& getChildren() const;

      OPTYPE getOpType() const {return m_opType;}
      
    private:
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

