// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/Tables/Table.cxx,v 1.5 2005/06/19 20:39:20 jrb Exp $

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

  // Columns are stored in order; binary cuts should do better than
  // just dumb compare one by one.
  Column* Table::getColumnByName(const std::string& name) const {
    unsigned nCol = m_sortedCols.size();
    unsigned minI = 0;
    unsigned maxI = nCol;

    unsigned guess = maxI/2;
    unsigned oldGuess = nCol;

    int cmp = name.compare(m_sortedCols[guess]->getName());

    while (cmp != 0)  {
      if (guess == oldGuess) return 0;        // not found

      if (cmp < 0 ) {    // thing we tried is > colName, so decrease maxI
        maxI = guess;
      }  else {   // thing we tried is > colName, so increase minI
        minI =  guess;
      }
      oldGuess = guess;
      guess = (minI + maxI)/2;
      cmp = name.compare(m_sortedCols[guess]->getName());
    }
    return m_sortedCols[guess];

  }

  Index* Table::getIndexByName(const std::string& name) const {
    unsigned nIx = m_indices.size();
    for (unsigned iIx = 0; iIx < nIx; iIx++) {
      Index* index = m_indices[iIx];
      if (index->getName() == name) return index;
    }
    return 0;
  }

  Assertion* Table::getAssertionByName(const std::string& name) const {
    unsigned nAssert = m_asserts.size();
    for (unsigned i = 0; i < nAssert; i++) {
      Assertion* a = m_asserts[i];
      if (a->getName() == name) return a;
    }
    return 0;

  }

  // Traversal order is self, columns, indices, asserts
  Visitor::VisitorState Table::accept(Visitor* v) {
    
    Visitor::VisitorState state = v->visitTable(this);
    if (state == Visitor::VBRANCHDONE) return Visitor::VCONTINUE;
    if (state != Visitor::VCONTINUE)       return state;

    unsigned n = m_cols.size();

    for (unsigned i = 0; i < n; i++) {
      state = m_cols[i]->accept(v);
      if (state != Visitor::VCONTINUE) return state;
    }

    n = m_indices.size();
    for (unsigned i = 0; i < n; i++) {
      state = m_indices[i]->accept(v);
      if (state != Visitor::VCONTINUE) return state;
    }

    n = m_asserts.size();
    for (unsigned i = 0; i < n; i++) {
      state = m_asserts[i]->accept(v);
      if (state != Visitor::VCONTINUE) return state;
    }
    return state;
  }

  void  Table::sortColumns() {
    if (m_sorted) return;
    m_sortedCols = m_cols;

    ColCompare  cmpObject;

    // Keep around original set so that rdbGUI will display them in
    // this order
    sort (m_sortedCols.begin(), m_sortedCols.end(), cmpObject); 
    m_sorted = true;

  }

  void Table::addColumn(Column* c) {
    m_cols.push_back(c);
    if (c->getSourceType() == Column::FROMendUser) m_nEndUser++;
  }

  /*
  Column* Table::getQuick(const std::string& colName) const {
    unsigned nCol = m_cols.size();
    unsigned minI = 0;
    unsigned maxI = nCol;

    unsigned guess = maxI/2;
    unsigned oldGuess = nCol;

    int cmp = colName.compare(m_cols[guess]->getName());

    while (cmp != 0)  {
      if (guess == oldGuess) return 0;        // not found

      if (cmp < 0 ) {    // thing we tried is > colName, so decrease maxI
        maxI = guess;
      }  else {   // thing we tried is > colName, so increase minI
        minI =  guess;
      }
      oldGuess = guess;
      guess = (minI + maxI)/2;
      cmp = colName.compare(m_cols[guess]->getName());
    }
    return m_cols[guess];

  }
  */

}
