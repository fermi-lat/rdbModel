// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/rdbModel/Db/MysqlConnection.h,v 1.1 2004/03/24 02:01:31 jrb Exp $
#ifndef RDBMODEL_MYSQLCONNECTION_H
#define RDBMODEL_MYSQLCONNECTION_H

#include "rdbModel/Db/Connection.h"

namespace rdbModel{

  class MysqlResults;
  class Assertion;
  /** 
      Class to handle connection to a MySQL database

        - initiate connection
        - make queries, including making returned data from queries available
        - issue SQL statements such as INSERT and UPDATE which have no
          returned data other than status
        - close down connection.
      Someday it might also have a method to create a database

      Initial design will just use host, password, userid passed in.
      Will be up to caller to insure that the userid has the right
      privilages for the operations caller intends to do.
  */
  class MysqlConnection {
  public:
    /** Open a connection 
        Allowed operations will depend on userid, etc., specified 
        return true if successful
    */
    MysqlConnection();
    virtual ~MysqlConnection();
    virtual bool open(const std::string& host, const std::string& userid,
                      const std::string& password,
                      const std::string& dbName,
                      unsigned int       port=0);
    /** Close the current open connection , if any.  Return true if there
     was a connection to close and it was closed successfully */
    virtual bool close();

    /** Typical derived class will form a syntactically correct 
        INSERT statement from the input arguments and issue it to
        the dbms. Return true if row was inserted successfully
        If @a auto_value is non-zero and the table has an auto-increment
        column, its value will be returned.

        Might also want to add a routine for INSERT ... SELECT
    */

    virtual bool insertRow(const std::string& tableName, 
                           const StringVector& colNames, 
                           const StringVector& values,
                           int* auto_value=0);


    /**
      Generic UPDATE. Return value is number of rows changed.
    */
    virtual unsigned int update(const std::string& tableName, 
                                const StringVector& colNames, 
                                const StringVector& values,
                                const Assertion* where=0);

    /**
      Support only for relatively simple SELECT, involving just
      one table.  
      @param tableName
      @param getCols   vector of columns to be retrieved
      @param where     ptr to an Assertion object
      @return If the SELECT succeeds, a pointer to an object which 
       manages the returned data; else 0.  Caller is responsible for
       deleting the ResultHandle object.
    */
    virtual ResultHandle* select(const std::string& tableName,
                                 const StringVector& getCols,
                                 const StringVector& orderCols,
                                 const Assertion* where=0,
                                 int   rowLimit=0);

    /**
      compile method for assertions.  Use it internally, but also make
      it publicly available so assertions belonging to a table
      can save the compiled version.
    */
    bool compileAssertion(const Asssertion* a, std::string& sqlString) const;
  private:
    MYSQL* m_mysql;
    bool   m_connected;
    bool   m_compileInit;
  };

}  // end namespace rdbModel
#endif
