// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/rdbModel/Db/MysqlResults.h,v 1.3 2004/03/28 08:22:51 jrb Exp $
#ifndef RDBMODEL_MYSQLRESULTS_H
#define RDBMODEL_MYSQLRESULTS_H

#include "rdbModel/Db/ResultHandle.h"

typedef struct st_mysql_res MYSQL_RES;

namespace rdbModel{

  class MysqlConnection;

  /** 
      Concrete implementation of ResultHandle, to accompany MysqlConnection.
  */
  class MysqlResults : virtual public ResultHandle {
    friend class MysqlConnection;

  public:
    virtual ~MysqlResults();

    /// Return number of rows in results
    virtual unsigned int getNRows() const;

    /**  
         Get array of field values for ith row of result set
    */
    virtual bool getRow(std::vector<std::string>& fields, unsigned int i = 0,
                        bool clear = true);


    /*
    // Return specified row in results as a string
    virtual bool getRowString(std::string& row, unsigned int iRow=0) const;

      Return vector of rows
         @param rows     to hold returned data
         @param iRow     starting row
         @param maxRow   maximum number of rows requested.  0 means "all"
         @param clear    if true, clear @a rows before storing new data
         @return         status
    
    virtual bool getRowStrings(std::vector<std::string>& rows, 
                               unsigned int iRow=0,
                               unsigned int maxRow=0, bool clear=true) const;
    */

  private:
    // Only MysqlConnection calls constructor
    MysqlResults(MYSQL_RES* results = 0); 

    MYSQL_RES* m_myres;
  };
}
#endif
