// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/rdbModel/Tables/Table.h,v 1.8 2005/06/23 01:20:01 jrb Exp $
#ifndef RDBMODEL_TABLE_H
#define RDBMODEL_TABLE_H
#include <vector>
#include <string>
#include "rdbModel/Management/Visitor.h"
#include "rdbModel/Tables/Column.h"

namespace rdbModel {

  //  class Column;
  class Index;
  class Assertion;

  class XercesBuilder;
  class Connection;

  class InsertNew;
  class Supersede;

  /// Function object used to sort columns by column name
  class ColCompare {
  public:
    bool operator()  (const Column* a, const Column*  b) {
      return (a->getName().compare(b->getName()) < 0);
    }
  };

  /** 
   * rdbModel representation of a(n SQL-like) table description
   */
  class Table {
  public:
    Table() : m_sorted(false), m_nEndUser(0), m_iNew(0), m_sup(0)
    {m_sortedCols.clear();}
    ~Table();


    const std::string& getName() const { return m_name;}
    Column* getColumnByName(const std::string& name) const;
    Index* getIndexByName(const std::string& name) const;
    Assertion* getAssertionByName(const std::string& name) const;

    InsertNew* getInsertNew() const {return m_iNew;}
    Supersede* getSupersede() const {return m_sup;}

    /// Verify that the input can be used to form an appropriate INSERT 
    /// statement for this row; if so and if we have a connection,
    /// ask the connection to do the insert and return status (an int,
    /// meaning to be determined.  Should include values for illegal
    /// input, missing connection, etc. )
    //  hasn't been implemented
    //    int  insertRow(const std::vector<std::string>& colNames,
    //                   const std::vector<std::string>& colValues);

    //    Column* getQuick(const std::string& colName) const;

    Visitor::VisitorState accept(Visitor* v);
    //     Visitor::VisitorState acceptNotRec(Visitor* v);

    void  sortColumns();
    /*
    bool evaluateAssertion(Assertion* pA, std::vector<FieldValPair>* toBe,
                           std::vector<FieldValPair>* old=0);
    */
  private:
    friend class rdbModel::XercesBuilder; // needs access to add.. methods

    std::vector<Column* > m_cols;
    std::vector<Column* > m_sortedCols;
    std::vector<Assertion* > m_asserts;
    std::vector<Index* > m_indices;

    void addColumn(Column* c);

    void addAssert(Assertion* a) {m_asserts.push_back(a);}
    void addIndex(Index* i) {m_indices.push_back(i); }

    std::string m_name;
    std::string m_version;
    std::string m_comment;

    bool m_sorted;  // set to true once columns have been sorted by name
    unsigned m_nEndUser;  // #columns whose value must be supplied by user

    InsertNew* m_iNew;
    Supersede* m_sup;
  };

}
#endif
