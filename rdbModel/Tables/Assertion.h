// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/rdbModel/Tables/Assertion.h,v 1.11 2005/06/19 20:39:19 jrb Exp $
#ifndef RDBMODEL_ASSERTION_H
#define RDBMODEL_ASSERTION_H
#include <vector>
#include <string>
#include "rdbModel/Rdb.h"
#include "rdbModel/Management/Visitor.h"
#include "rdbModel/Tables/Column.h"

namespace rdbModel{

  class Table;

    enum OPTYPE {
      OPTYPEundefined = 0,
      OPTYPEor = 1,
      OPTYPEand,
      OPTYPEnot,
      OPTYPEexists,
      OPTYPEisNull,
      OPTYPEequal,           // first of 2-operand compare ops
      OPTYPEnotEqual,
      OPTYPElessThan,
      OPTYPEgreaterThan,
      OPTYPElessOrEqual,
      OPTYPEgreaterOrEqual,
      OPTYPElast
    };


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

      Once an operation has been incorporated into an Assertion or into 
      another operation, it is 'owned' by this parent.  Deleting the parent
      will cause its children to be deleted.  Hence applications building
      assertions out of operators should never delete those operators.
      
 
  */
  class Assertion {
  public:
    class Operator;         // nested class declared below

  public:
    // when does this assertion get applied?
    /*
    enum WHEN {
      WHENundefined = 0,
      WHENglobalCheck = 1,
      WHENchangeRow,
      WHENwhere     // as a WHERE clause, used and then discarded
    };
    */

    class Operator {
    public:
      Operator() : m_opType(OPTYPEundefined) {};
      ~Operator();

      /// Constructor for comparison.  If the operator is OPTTYPEisNull,
      /// rightArg and rightLiteral are ignored.
      Operator(OPTYPE type, const std::string& leftArg, 
               const std::string& rightArg, 
               FIELDTYPE leftType, FIELDTYPE rightType);

      /// Constructor for EXISTS
      Operator(OPTYPE type, const std::string& tableName, Operator* child=0);

      /// Constructor for OR, AND, NOT
      Operator(OPTYPE type, const std::vector<Operator*>& children,
               bool keepChildren = false);

      /// Add another child to a conjunction-style operator
      bool appendChild(Operator* child);

      /// Check whether columns or column and literal to be compared
      /// have compatible types
      bool validCompareOp(Table* table) const;

      /// True if operator is "isNull" or any of the usual arithmetic
      /// comparisons
      bool isCompareOp() const {return (m_opType >= OPTYPEisNull);}

      /// Throw exception if Operator is not a comparison operator
      const std::string* getCompareArgs() const;

      /// Throw exception if Operaotr is not EXISTS
      const std::string& getTableName() const;

      /// Get types of comparison args
      const FIELDTYPE* getCompareArgTypes() const;

      /// Throw exception if Operator is a comparison operator
      const std::vector<Operator* >& getChildren() const;

      OPTYPE getOpType() const {return m_opType;}

      /// True if operator or sub-operator refers to future row
      bool getToBe() const {return m_toBe;}

      /// True if operator or sub-operator refers to existing row
      bool getOld() const {return m_old;}

      /// Evaluate operator on argument Rows 
      bool verify(Row& old, Row& toBe, Table* t);

      /// Handling specific to 2-arg compare operators
      bool verifyCompare(Row& old, Row& toBe, Table* t);

      /// Handling specific to timestamp data
      bool compareTs(const std::string* vals, OPTYPE type);

      /// Handling specific to integer data
      bool compareInt(const std::string* vals, OPTYPE type);

      /// Handling specific to floating point data
      bool compareFloat(const std::string* vals, OPTYPE type);

      /// Handling specific to string data
      bool compareString(const std::string* vals, OPTYPE type);


    private:
      OPTYPE m_opType;

      /** Following two lines apply only to compare operators (includes
          isNull)

          In order to format properly in an SQL query, need to
          keep track of whether compare arg is literal, column name
          referring to current row under consideration, or column
          name referring to proposed row (in case this is part of an
          assertion about relation of existing rows to proposed row)
      */
      std::string m_compareArgs[2];
      FIELDTYPE m_compareType[2]; 

      /// Following used only for EXISTS
      std::string m_tableName;
      bool        m_keepChildren;

      /// Following is used only for non-compare operators
      std::vector<Operator* > m_operands;  // #allowed depends on opType

      /// Following is true if this operator or suboperator has an arg.
      /// column name referring to a "toBe" row
      bool  m_toBe; 

      /// Following is true if this operator or suboperator has an arg.
      /// column name referring to a  row already in the table
      bool  m_old;
    };

    /**  
         Normally, operator associated with the assertion will be deleted
         when the assertion itself is deleted, but this won't happen if
         keepOp is set to true.
     */
    //    Assertion(WHEN when = WHENundefined, Operator* op = 0, 
    //              Table* myTable=0, bool keepOp=false) : 
    //      m_when(when), m_op(op), m_myTable(myTable), m_keepOp(keepOp)
    Assertion(Operator* op = 0, Table* myTable=0, bool keepOp=false) : 
      m_op(op), m_myTable(myTable), m_keepOp(keepOp)
    { m_compiled.clear(); m_name.clear();};

    ~Assertion();
    //    WHEN getWhen() const {return m_when;}
    Visitor::VisitorState accept(Visitor* v);

    Operator* getOperator() const {return m_op;}
    /// 
    const std::string& getPrecompiled() const {return m_compiled;}
    
    /// True if associated operator or descendant refers to future row
    /// (in which case can't call MySql::compileAssertion)
    bool getToBe() const {return m_op->getToBe();}

    /// True if associated operator or descendant refers to existing row
    bool getOld() const {return m_op->getOld();}

    const std::string& getName() const {return m_name;}
    void setName(const std::string& name) {m_name = name;}

    /// Does assertion (which may refer to one or both of an old row and
    /// a proposed row) hold for these arguments?  
    /// Caller is responsible for fetching old row fields out of dbs if
    /// necessary
    bool verify(Row& old, Row& toBe);

  private:
    //    WHEN m_when;
    Operator* m_op;
    Table* m_myTable;
    bool   m_keepOp;
    std::string m_name;

    /// Let's hope that, independent of connection type, std::string is
    /// a reasonable choice for "compiled" form of the assertion
    std::string m_compiled;  

  };
}
#endif

