// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/Rdb.cxx,v 1.2 2004/03/06 01:13:29 jrb Exp $ 
#include "rdbModel/Rdb.h"
#include "rdbModel/Tables/Table.h"

namespace rdbModel {

  Rdb::~Rdb() {
    while (m_tables.size() ) {
      Table* table = m_tables.back();
      m_tables.pop_back();
      delete table;
    }
  }

  Table* Rdb::getTable(const std::string& name) const {
    unsigned nTable = m_tables.size();
    for (unsigned iTable = 0; iTable < nTable; iTable++) {
      Table* table = m_tables[iTable];
      if (table->getName() == name) return table;
    }
    return 0;
  }

  Column* Rdb::getColumn(const std::string& tableName, 
                         const std::string& colName) const {
    Table* table = getTable(tableName);
    if (!table) return 0;

    return table->getColumnByName(colName);

  }

  Index* Rdb::getIndex(const std::string& tableName, 
                       const std::string& indexName) const {
    Table* table = getTable(tableName);
    if (!table) return 0;

    return table->getIndexByName(indexName);

  }

  unsigned int Rdb::accept(Visitor* v) {
    Visitor::VisitorState state = v->visitRdb(this);
    if (state != Visitor::VCONTINUE) return state;

    unsigned nTable = m_tables.size();

    for (unsigned i = 0; i < nTable; i++) {
      state = m_tables[i]->accept(v);
      if (state != Visitor::VCONTINUE) return state;
    }
    return state;
  }

}
    
