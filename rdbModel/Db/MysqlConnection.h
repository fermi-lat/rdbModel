// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/rdbModel/Db/MysqlConnection.h,v 1.5 2004/04/03 00:19:17 jrb Exp $
#ifndef RDBMODEL_MYSQLCONNECTION_H
#define RDBMODEL_MYSQLCONNECTION_H

#include "rdbModel/Db/Connection.h"
#include "rdbModel/Tables/Assertion.h"
#include "rdbModel/Management/Visitor.h"
#include <map>

typedef struct st_mysql MYSQL;
typedef struct st_mysql_res MYSQL_RES;

namespace rdbModel{

  class MysqlResults;
  class Datatype;
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
  class MysqlConnection : public virtual Connection, public Visitor {
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

    /** Parameter is normally path for an xml file descrbing the 
        connection parameters */
     virtual bool open(const std::string& parms);

    /** Close the current open connection , if any.  Return true if there
     was a connection to close and it was closed successfully */
    virtual bool close();

    /**
       Check to what degree local schema definition is compatible with
       remote db accessed via this connection
    */
    virtual MATCH matchSchema(Rdb *rdb);

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
    virtual bool 
    compileAssertion(const Assertion* a, std::string& sqlString) const;

    // Needed to satisfy Visitor interface:
    virtual Visitor::VisitorState visitRdb(Rdb*);
    virtual Visitor::VisitorState visitTable(Table*);
    virtual Visitor::VisitorState visitColumn(Column*);
    virtual Visitor::VisitorState visitIndex(Index*);
    virtual Visitor::VisitorState visitAssertion(Assertion*);


  private:
    MYSQL* m_mysql;
    bool   m_connected;

    std::string m_dbName;
    static bool   m_compileInit;
    static void compileInit();
    static bool compileComparison(Assertion::Operator* op, 
                                  std::string& sqlString);
    static bool compileOperator(Assertion::Operator* op, 
                                std::string &sqlString);

    bool checkDType(Datatype* dtype, const std::string& sqlType);

    // Following collection of data members is only of interest while 
    // visit is in progress.

    /// Someday we may want more than one kind of visitor; for example,
    /// might want one to create dbs
    enum VISITOR {
      VISITORundefined,
      VISITORmatch
    };
    VISITOR m_visitorType;

    /// Keep track of status during matching process
    MATCH   m_matchReturn;

    /// For query results while visiting.
    MYSQL_RES* m_tempRes;

    /// Index by colname; data is row number with "SHOW COLUMNS.." result set
    std::map<std::string, unsigned int> m_colIx;

    std::string m_primColName;

  };

}  // end namespace rdbModel
#endif
