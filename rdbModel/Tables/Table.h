// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/rdbModel/Tables/Table.h,v 1.9 2005/06/23 18:57:45 jrb Exp $
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
    Table() : m_sorted(false), m_nEndUser(0), m_iNew(0), m_sup(0), m_connect(0)
    {m_sortedCols.clear(); m_programCols.clear(); m_userCols.clear();}
    ~Table();


    const std::string& getName() const { return m_name;}
    Column* getColumnByName(const std::string& name) const;
    Index* getIndexByName(const std::string& name) const;
    Assertion* getAssertionByName(const std::string& name) const;

    /**
       smartInsert is smart in the following respects:
          o Makes some checks to see if row is self-consistent
          o Fills in all fields which are to be supplied by "service"
          o If row satisfies conditions for being "official"
            - queries table to see if any pre-existing rows need 
              adjustment; if so, they are adjusted
          o Finally inserts the row 

       If @a test is true, just output description of what would be
       done without actually doing it.
       Note @a row may be modified by the function.
    */
    int smartInsert(Row& row, bool test=false);

    // Do we need these for anything?  
    InsertNew* getInsertNew() const {return m_iNew;}
    Supersede* getSupersede() const {return m_sup;}

    Visitor::VisitorState accept(Visitor* v);
    //     Visitor::VisitorState acceptNotRec(Visitor* v);

    void  sortColumns();
    /*
    bool evaluateAssertion(Assertion* pA, std::vector<FieldValPair>* toBe,
                           std::vector<FieldValPair>* old=0);
    */
  private:
    friend class rdbModel::XercesBuilder; // needs access to add.. methods
    friend class rdbModel::Rdb;           // to set connection
    std::vector<Column* > m_cols;
    std::vector<Column* > m_sortedCols;
    std::vector<Assertion* > m_asserts;
    std::vector<Index* > m_indices;

    /// Subset of columns which are responsibility of Program (us) to fill
    std::vector<Column* > m_programCols;

    /// Subset of columns which user *must* supply
    std::vector<Column* > m_userCols;

    void addColumn(Column* c);

    void addAssert(Assertion* a) {m_asserts.push_back(a);}
    void addIndex(Index* i) {m_indices.push_back(i); }

    bool fillProgramCol(Column* col, Row& row, bool newRow);

    std::string m_name;
    std::string m_version;
    std::string m_comment;

    bool m_sorted;  // set to true once columns have been sorted by name
    unsigned m_nEndUser;  // #columns whose value must be supplied by user

    InsertNew* m_iNew;
    Supersede* m_sup;

    Connection* m_connect;
  };

}
#endif
