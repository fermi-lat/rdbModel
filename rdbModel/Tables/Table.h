// $Header:  $
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

    Column* getColumnByName(const std::string& name) const;
    Index* getIndexByName(const std::string& name) const;

    Visitor::VisitorState accept(Visitor* v);
    Visitor::VisitorState acceptNotRec(Visitor* v);

  private:
    friend XercesBuilder; // needs access to add.. methods below

    std::vector<Column* > m_cols;
    std::vector<Assertion* > m_asserts;
    std::vector<Index* > m_indexs;

    void addColumn(const Column* c) m_cols.push_back(c);
    void addAssert(const Assertion* a) m_cols.push_back(a);
    void addIndex(const Index* i) m_cols.push_back(k);

    std::string m_name;
    std::string m_version;
    std::string m_comment;

  };

}
#endif
