// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/rdbModel/Db/MysqlResults.h,v 1.1 2004/03/24 02:01:31 jrb Exp $
#ifndef RDBMODEL_MYSQLRESULTS_H
#define RDBMODEL_MYSQLRESULTS_H

#include "rdbModel/Db/ResultHandle.h"

namespace rdbModel{

  class MYSQL_RES;
  /** 
      Concrete implementation of ResultHandle, to accompany MysqlConnection.
  */
  class MysqlResults : virtual public ResultHandle {
    friend class rdbModel::MysqlConnection;

  public:
    virtual ~MysqlResults();

    /// Return number of rows in results
    int getNRows() const;

    /// Return specified row in results as a string
    bool getRowString(std::string& row, unsigned int iRow=0);

    /**  Return vector of rows
         @param rows     to hold returned data
         @param iRow     starting row
         @param maxRow   maximum number of rows requested.  0 means "all"
         @param clear    if true, clear @a rows before storing new data
         @return         status
    */
    bool getRowStrings(std::vector<std::string>& rows, unsigned int iRow=0,
                       unsigned int maxRow=0, bool clear=true) const;

  private:
    // Only MysqlConnection calls constructor
    MysqlResults(MYSQL_RES* results = 0); 

    MYSQL_RES* m_myres;
  };
}
#endif
