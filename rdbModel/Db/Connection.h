// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/rdbModel/Db/Connection.h,v 1.1 2004/03/24 02:01:31 jrb Exp $
#ifndef RDBMODEL_CONNECTION_H
#define RDBMODEL_CONNECTION_H
#include <vector>
#include <string>


namespace rdbModel{

  class ResultsHandle;
  class Assertion;
  /** 
      Class to handle connection to an SQL database, or something very like
      it.  It should be able to
        - initiate connection
        - make queries, including making returned data from queries available
        - issue SQL statements such as INSERT and UPDATE which have no
          returned data other than status
        - close down connection.
      Someday it might also have a method to create a database

      Maybe make it pure virtual?  And make a MySQL implementation
      derived from it.  

      Initial design will just use host, password, userid passed in.
      Will be up to caller to insure that the userid has the right
      privilages for the operations caller intends to do.
  */
  typedef std::vector<std::string>  StringVector;

  class Connection {
    /** Open a connection 
        Allowed operations will depend on userid, etc., specified 
        return true if successful
    */
    Connection() {};
    virtual ~Connection() {};
    virtual bool open(const std::string& host, const std::string& userid,
                      const std::string& password,
                      const std::string& dbName,
                      unsigned int       port) = 0;
    /** Close the current open connection , if any.  Return true if there
     was a connection to close and it was closed successfully */
    virtual bool close() = 0;

    /** Typical derived class will form a syntactically correct 
        INSERT statement from the input arguments and issue it to
        the dbms. Return true if row was inserted successfully

        Might also want to add a routine for INSERT ... SELECT
    */

    virtual bool insertRow(const std::string& tableName, 
                           const StringVector& colNames, 
                           const StringVector& values) = 0;

    /*
       So far anticipated uses of UPDATE would just modify a single row
       identified by ser_no (or, more generally, primary key), so
       make a method for this case.  Can call the more general
       version below.

       @return true if no errors were encountered; else false.  If the 
       update requested was error-free but entailed no actual change, 
       returns ??
     */
    /*
      At this level, don't have access to column name for primary 
      key, if any.  This has to be done by caller
    virtual bool updateUnique(const std::string& tableName, 
                              const StringVector& colNames, 
                              const StringVector& values,
                              const std::string& keyValue) = 0;
    */

    /**
      Generic UPDATE. Return value is number of rows changed.
    */
    virtual unsigned int update(const std::string& tableName, 
                                const StringVector& colNames, 
                                const StringVector& values,
                                const Assertion* where=0) = 0;

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
                                 int   rowLimit=0)=0;

    /**
      compile method for assertions.  Use it internally, but also make
      it publicly available so assertions belonging to a table
      can save the compiled version.
    */
    bool compileAssertion(const Asssertion* a, std::string& sqlString)=0;

  };

}  // end namespace rdbModel
#endif
