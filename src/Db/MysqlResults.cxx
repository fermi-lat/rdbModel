// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/Db/MysqlResults.cxx,v 1.8 2005/07/11 23:49:40 jrb Exp $
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
    unsigned long const* lengths = mysql_fetch_lengths(m_myres);

    unsigned nFields = mysql_num_fields(m_myres);

    if (clear) fields.clear();

    for (unsigned int iField = 0; iField < nFields; iField++) {
      if (myRow[iField])
	fields.push_back(std::string(myRow[iField],
				     lengths[iField]));
      else
	fields.push_back("");
    }

    return true;
  }

  bool MysqlResults::getRowPtrs(std::vector<std::string*>& fields, 
                                unsigned int i, bool clear) {

    mysql_data_seek(m_myres, i);
    MYSQL_ROW myRow = mysql_fetch_row(m_myres);
    unsigned long const* lengths = mysql_fetch_lengths(m_myres);

    unsigned nFields = mysql_num_fields(m_myres);


    if (clear) fields.clear();

    for (unsigned int iField = 0; iField < nFields; iField++) {
      if (myRow[iField])
	fields.push_back(new std::string(myRow[iField],
					 lengths[iField]));
      else
	fields.push_back(0);
    }

    return true;

  }
  // May also want to do 
  //  MYSQL_FIELD* fields = mysql_fetch_fields(m_myres);
  // and make these available to client, especially for select * queries
}
