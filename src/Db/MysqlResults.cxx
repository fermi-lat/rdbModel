// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/Db/MysqlResults.cxx,v 1.2 2004/03/28 08:24:27 jrb Exp $
#ifdef  WIN32
#include <windows.h>
#endif

#include "rdbModel/Db/MysqlResults.h"
#include "mysql.h"

namespace rdbModel {

  MysqlResults::MysqlResults(MYSQL_RES* results) : m_myres(results) {
  }

  MysqlResults::~MysqlResults() {
    mysql_free_result(m_myres);
  }

  unsigned int MysqlResults::getNRows() const {
    return mysql_num_rows(m_myres);
  }

  bool MysqlResults::getRow(std::vector<std::string>& fields, 
                            unsigned int i, bool clear) {
    mysql_data_seek(m_myres, i);
    MYSQL_ROW myRow = mysql_fetch_row(m_myres);

    unsigned nFields = mysql_num_fields(m_myres);

    if (clear) fields.clear();

    for (unsigned int iField = 0; iField < nFields; iField++) {
      if (myRow[iField]) fields.push_back(std::string(myRow[iField]));
      else fields.push_back("");
    }

    return true;
  }

  /*
  bool MysqlResults::getRowString(std::string& row, unsigned int iRow) const {
    mysql_data_seek(m_myres, iRow);
    MYSQL_ROW myRow = mysql_fetch_row(m_myres);

    unsigned nFields = msyql_num_fields(m_myres);
  }
  */



  /*
  bool MysqlResults::getRowStrings(std::vector<std::string>& rows, 
                              unsigned int iRow=0, unsigned int maxRow,
                              bool clear) const {
    if (clear) rows.clear();

    unsigned int nRow = getNRows();
    unsigned lastRow = nRow - 1;
    if (lastRow > (maxRow + iRow - 1) ) lastRow = maxRow + iRow - 1;

    for (unsigned int ix = iRow; ix <= lastRow; ix++) {
      std::string aRow;
      getRowString(aRow, ix);
      rows.push_back(aRow);
    }
    return true;
  }
  */
}
