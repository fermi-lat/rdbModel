// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/rdbModel/Tables/Table.h,v 1.5 2004/04/27 00:05:32 jrb Exp $
#ifndef RDBMODEL_TABLE_H
#define RDBMODEL_TABLE_H
#include <vector>
#include <string>
#include "rdbModel/Management/Visitor.h"

namespace rdbModel {

  class Column;
  class Index;
  class Assertion;

  class XercesBuilder;
  class Connection;


  /** 
   * rdbModel representation of a(n SQL-like) table description
   */
  class Table {
  public:
    Table() {};
    ~Table();

    const std::string& getName() const { return m_name;}
    Column* getColumnByName(const std::string& name) const;
    Index* getIndexByName(const std::string& name) const;
    /// Verify that the input can be used to form an appropriate INSERT 
    /// statement for this row; if so and if we have a connection,
    /// ask the connection to do the insert and return status (an int,
    /// meaning to be determined.  Should include values for illegal
    /// input, missing connection, etc. )
    //  hasn't been implemented
    //    int  insertRow(const std::vector<std::string>& colNames,
    //                   const std::vector<std::string>& colValues);

    
    Visitor::VisitorState accept(Visitor* v);
    //     Visitor::VisitorState acceptNotRec(Visitor* v);

  private:
    friend class rdbModel::XercesBuilder; // needs access to add.. methods

    std::vector<Column* > m_cols;
    std::vector<Assertion* > m_asserts;
    std::vector<Index* > m_indices;

    void addColumn(Column* c) {m_cols.push_back(c);}
    void addAssert(Assertion* a) {m_asserts.push_back(a);}
    void addIndex(Index* i) {m_indices.push_back(i); }

    std::string m_name;
    std::string m_version;
    std::string m_comment;


  };

}
#endif
