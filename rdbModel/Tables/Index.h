// $Header:  $
#ifndef RDBMODEL_INDEX_H
#define RDBMODEL_INDEX_H
#include <vector>
#include <string>
#include "rdbModel/Management/Visitor.h"

namespace rdbModel{

  class Column;
  class Table;

  class XercesBuilder;


  /** 
   * rdbModel representation of a(n SQL-like) key (aka index) description
   */
  class Index {
  public:
    Index() {};
    ~Index();

    Visitor::VisitorState accept(Visitor* v);
    Visitor::VisitorState acceptNotRec(Visitor* v);

  private:
    /// Is it a primary key?
    bool m_primary;

    /// Names of  columns it's indexing
    std::vector<std::string&> m_indexCols;

    /// Point back to owning table
    Table* m_myTable;
    std::string m_name;
  };
}

#endif
