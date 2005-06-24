// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/Tables/Table.cxx,v 1.6 2005/06/23 01:20:01 jrb Exp $

#include "rdbModel/Tables/Table.h"
#include "rdbModel/Tables/Column.h"
#include "rdbModel/Tables/Index.h"
#include "rdbModel/Tables/Assertion.h"
#include "rdbModel/Db/Connection.h"
#include "rdbModel/RdbException.h"
#include "rdbModel/Tables/InsertNew.h"
#include "facilities/Util.h"
#include "facilities/Timestamp.h"


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
    if (c->getSourceType() == Column::FROMendUser) {
      m_nEndUser++;
      // if not nullable and no default, user must supply it
      if (!(c->nullAllowed()) && ((c->getDefault()).size() == 0)) {
        m_userCols.push_back(c);
      }
    }
    else if (c->getSourceType() == Column::FROMprogram) {
      m_programCols.push_back(c);
    }
  }

  int Table::smartInsert(Row& row, bool test) {

    if (!m_connect) {
      throw RdbException("Table::smartInsert Need matching connection");
    }
    row.rowSort();

    // Fill in columns in m_programCols list
    for (unsigned i = 0; i < m_programCols.size(); i++) {
      fillProgramCol(m_programCols[i], row, true);
    }

    // Check that all required columns are there and not null
    for (unsigned i = 0; i < m_userCols.size(); i++) {
      FieldVal* f = row.find(m_userCols[i]->getName());
      if ((!f) || (f->m_null)) {
        std::string 
          msg("Table::smartInsert Row to be inserted missing req'd field ");
        msg = msg + m_userCols[i]->getName();
        throw RdbException(msg);
      }
    }
    // Check against consistency conditions specified by <insertNew>
    const Assertion* cond = m_iNew->getInternal();

    bool satisfied = cond->verify(row, row);

    if (!satisfied) return -1;   // or maybe throw exception here?

    // Check against official conditions specified by <insertNew>
    const Assertion* condO = m_iNew->getOfficial();
    Row empty;

    satisfied = condO->verify(empty, row);

    // If not official, do insert and exit
    if (!satisfied) {
      std::vector<std::string> colNames;
      std::vector<std::string> colValues;
      std::vector<std::string> nullCols;

      row.regroup(colNames, colValues, nullCols);
      bool ok = m_connect->insertRow(m_name, colNames, colValues,
                                     0, &nullCols);
      return (ok) ?  0 : -1;
    }
    // Collect list (by key value) of rows potentially needing changes
    // Make specified changes (update)
    // Insert and exit
    
    // NYI
    return -1;
  }

  bool Table::fillProgramCol(Column* col, Row& row, bool newRow) {
    std::string val;

    switch (col->getContentsType() ) {
    case Column::CONTENTSserviceName: 
      val="rdbModel";
      break;
    case Column::CONTENTSusername: {
#ifdef    __GNUG__ 
      val = std::string("$(USER)");
#else
      val = std::string("$(USERNAME)");
#endif

      int nsub = facilities::Util::expandEnvVar(&val);
      if (nsub != 1) {
        val="no username";
      }
    }

    case Column::CONTENTSinsertTime:
      if (!newRow) return false;   // otherwise, same as updateTime case
    case Column::CONTENTSupdateTime: {
      facilities::Timestamp curTime;
      val = curTime.getString();
    }
    default:
      return false;    // unrecognized type
    }
    // look for this field in Row input. If there, overwrite value
    FieldVal* f = row.find(col->getName());
    if (f) {
      f->m_val = val;
      f->m_null = false;
    } 
    else {
      FieldVal g(col->getName(), val, false);
      row.addField(g);
      row.rowSort();
    }
    return true;
  }

}
