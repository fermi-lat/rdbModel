// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/rdbModel/Tables/Table.h,v 1.2 2004/03/04 01:07:11 jrb Exp $
#ifndef RDBMODEL_TABLE_H
#define RDBMODEL_TABLE_H
#include <vector>
#include <string>
#include "rdbModel/Management/Visitor.h"

namespace rdbModel{

  class Column;
  class Index;
  class Assertion;

  class XercesBuilder;


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
