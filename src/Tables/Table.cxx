// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/Tables/Table.cxx,v 1.2 2004/03/06 01:15:25 jrb Exp $

#include "rdbModel/Tables/Table.h"
#include "rdbModel/Tables/Column.h"
#include "rdbModel/Tables/Index.h"
#include "rdbModel/Tables/Assertion.h"

namespace rdbModel {
  Table::~Table() {
    while (m_cols.size() ) {
      Column* c = m_cols.back();
      m_cols.pop_back();
      delete c;
    }

    while (m_asserts.size() ) {
      Assertion* a = m_asserts.back();
      m_asserts.pop_back();
      delete a;
    }

    while (m_indices.size() ) {
      Index* i = m_indices.back();
      m_indices.pop_back();
      delete i;
    }
  }


  // For the time being the number of columns is not huge; no particular
  // reason to do anything fancy here
  Column* Table::getColumnByName(const std::string& name) const {
    unsigned nCol = m_cols.size();
    for (unsigned iCol = 0; iCol < nCol; iCol++) {
      Column* col = m_cols[iCol];
      if (col->getName() == name) return col;
    }
    return 0;

  }

  Index* Table::getIndexByName(const std::string& name) const {
    unsigned nIx = m_indices.size();
    for (unsigned iIx = 0; iIx < nIx; iIx++) {
      Index* index = m_indices[iIx];
      if (index->getName() == name) return index;
    }
    return 0;

  }

  // Traversal order is self, columns, indices, asserts
  Visitor::VisitorState Table::accept(Visitor* v) {
    
    Visitor::VisitorState state = v->visitTable(this);
    if (state == Visitor::BRANCHDONE) return Visitor::CONTINUE;
    if (state != Visitor::CONTINUE)       return state;

    unsigned n = m_cols.size();

    for (unsigned i = 0; i < n; i++) {
      state = m_cols[i]->accept(v);
      if (state != Visitor::CONTINUE) return state;
    }

    n = m_indices.size();
    for (unsigned i = 0; i < n; i++) {
      state = m_indices[i]->accept(v);
      if (state != Visitor::CONTINUE) return state;
    }

    n = m_asserts.size();
    for (unsigned i = 0; i < n; i++) {
      state = m_asserts[i]->accept(v);
      if (state != Visitor::CONTINUE) return state;
    }
    return state;
  }


}
