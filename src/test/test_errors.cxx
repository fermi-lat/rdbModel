// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/test/test_errors.cxx,v 1.5 2008/07/21 15:17:38 glastrm Exp $
// Test program for rdbModel primitive buiding blocks

#include <iostream>
#include <string>
#include <cstdlib>
#include "rdbModel/Rdb.h"
#include "rdbModel/RdbException.h"
#include "rdbModel/Management/Manager.h"
#include "rdbModel/Management/XercesBuilder.h"
#include "rdbModel/Db/MysqlConnection.h"
#include "rdbModel/Db/MysqlResults.h"
#include "rdbModel/Tables/Table.h"
#include "rdbModel/Tables/Column.h"
#include "rdbModel/Tables/Datatype.h"
#include "rdbModel/Tables/Assertion.h"
#include "facilities/Util.h"
#include "facilities/commonUtilities.h"

// When TEST_INSERT is defined, use connection which can write
// #define TEST_INSERT

int doBadInsert(rdbModel::Rdb* con);
int doBadUpdate(rdbModel::Rdb*, int serial);
int doUpdate(rdbModel::Rdb*, int serial);

int main(int, char**) {
  using rdbModel::FieldVal;

  std::string xmlPath = facilities::commonUtilities::getXmlPath("rdbModel");
  std::string infile = xmlPath + std::string("/calib_test.xml");

  rdbModel::Builder* b = new rdbModel::XercesBuilder;
  rdbModel::Rdb* rdb = new rdbModel::Rdb;
  int errcode = rdb->build(infile, b);

  if (errcode) {
    std::cerr << "Build failed with error code " << errcode << std::endl;
    return errcode;
  }
  
  rdbModel::Table* t = rdb->getTable("metadata_v2r1");


  // mostly don't want to run code doing an insert.  For times
  // when we do, must connect as user with INSERT priv.
#ifdef TEST_INSERT
  std::string connectfileT = xmlPath + std::string("connect/mysqlTester.xml");
#else
  // This is glastreader, calib_test
  std::string connectfileT = xmlPath + std::string("connect/mysqlSlacT.xml");
#endif
  


  // Connect to production database, read only
  rdbModel::MysqlConnection* con = new rdbModel::MysqlConnection();

  std::string connectfile = xmlPath + std::string("/connect/mysqlSlac.xml");
  if (!(con->open(connectfile)) ) {
    std::cerr << "Unable to connect to MySQL database" << std::endl;
    return -1;
  }
  /*
  if (!(con->open(connectfileT)) ) {
    std::cerr << "Unable to connect to MySQL test database" << std::endl;
    return -1;
  }
  */

  rdbModel::MATCH match = con->matchSchema(rdb, false);

  switch (match) {
  case rdbModel::MATCHequivalent:
    std::cout << "XML schema and MySQL database are equivalent!" << std::endl;
    break;
  case rdbModel::MATCHcompatible:
    std::cout << "XML schema and MySQL database are compatible" << std::endl;
    break;
  case rdbModel::MATCHfail:
    std::cout << "XML schema and MySQL database are NOT compatible" 
              << std::endl;
    return -2;
  case rdbModel::MATCHnoConnection:
    std::cout << "Connection failed while attempting match" << std::endl;
    return -1;
  }


  // Make a query
  std::string rq[4];
  rq[0] ="select * from metadata_v2r1";
  rq[1] ="select calib_type from metadata_v2r1";
  rq[2] ="select garbage from metadata_v2r1";

  // Try a query with improper syntax
  rq[3]=" select ser_no from metadata_v2r1 WHERE noSuchColumn='2' ";

  for (int i = 0; i < 3; i++) {
    try {
      rdbModel::ResultHandle* res = 
        con->dbRequest(rq[i]);
      if (res) {
        std::cout << "dbRequest '" << rq[i] << "'" << std::endl; 
        std::cout << "succeeded, returned " << res->getNRows() 
                  << " rows" << std::endl;
      }
      else {
        std::cout << "dbRequest '" << rq[i] << "'" << std::endl; 
        std::cout << "succeeded, no returned data expected" << std::endl;
      }
    }
    catch (rdbModel::RdbException ex) {
      std::cerr << "dbRequest '" <<  rq[i] << "'" << std::endl; 
      std::cerr <<" failed with error: " << ex.getMsg() << std::endl;
      //      std::cerr << "Code " << ex.getCode() << std::endl;
    }
  }

  // Make a bad query with con->select
  std::vector<std::string> getCols;
  std::vector<std::string> orderCols;
  std::string where(" WHERE ser_no >=<  7");
  getCols.push_back("flavor");
  try {
    rdbModel::ResultHandle* res =
      con->select("metadata_v2r1", getCols, orderCols, where);
  }
  catch (rdbModel::RdbException ex) {
    std::cerr << "select failed with error: " << ex.getMsg() << std::endl;
      //      std::cerr << "Code " << ex.getCode() << std::endl;
  }

  doUpdate(rdb, 1);     // should fail because we don't have write access
  con->close();

// Following will do an insert if disable = set to false.  
// To keep from cluttering up the
// database, mostly don't execute
//  
# ifdef TEST_INSERT

  if (!(con->open(connectfileT)) ) {
    std::cerr << "Unable to connect to MySQL test database" << std::endl;
    return -1;
  }


  match = con->matchSchema(rdb, false);

  switch (match) {
  case rdbModel::MATCHequivalent:
    std::cout << "XML schema and MySQL database are equivalent!" << std::endl;
    break;
  case rdbModel::MATCHcompatible:
    std::cout << "XML schema and MySQL database are compatible" << std::endl;
    break;
  case rdbModel::MATCHfail:
    std::cout << "XML schema and MySQL database are NOT compatible" 
              << std::endl;
    return -2;
  case rdbModel::MATCHnoConnection:
    std::cout << "Connection failed while attempting match" << std::endl;
    return -1;
  }

  doBadInsert(rdb);  
  doBadUpdate(rdb, 23);  
# endif
  return 0;
}

// int doInsert(rdbModel::Connection* con) {
int doBadInsert(rdbModel::Rdb* rdb) {
  
  using rdbModel::FieldVal;
  using rdbModel::Row;

  std::vector<FieldVal> fields;
  fields.reserve(15);

  fields.push_back(FieldVal("instrument", "LAT"));
  fields.push_back(FieldVal("calib_type","Test_Gen"));
  fields.push_back(FieldVal("flavor","berry"));
  fields.push_back(FieldVal("data_fmt","nonsense"));
  fields.push_back(FieldVal("vstart","2003-02-01"));
  fields.push_back(FieldVal("data_size","0"));
  fields.push_back(FieldVal("locale","phobos"));
  fields.push_back(FieldVal("completion","ABORT"));
  fields.push_back(FieldVal("data_ident","$(mycalibs)/test/moreJunk.xml"));
  fields.push_back(FieldVal("notes", 
                            "Absurd test item, setting input_desc to NULL"));
  fields.push_back(FieldVal("input_desc","", true));
  fields.push_back(FieldVal("noSuchField","", true));

  int  serial = 0;
  
  Row row(fields);

  rdb->insertRow("metadata_v2r1", row, &serial);

  unsigned errcode = rdb->getConnection()->getLastError();
  std::cerr << "From doBadInsert.  Last error code was " << errcode 
            << std::endl;
  if (rdb->duplicateError() ) {
    std::cerr << "Last error was duplicate insert " << std::endl;
  }
  else {
    std::cerr << "Last error was NOT duplicate insert " << std::endl;
  }
    
  return serial;
}

// int doUpdate(rdbModel::Connection* con, int serial) {
int doBadUpdate(rdbModel::Rdb* rdb, int serial) {
  using rdbModel::Column;
  using facilities::Util;
  using rdbModel::FieldVal;
  using rdbModel::Row;

  // Set up bad WHERE clause
  std::string serialStr;
  Util::itoa(serial, serialStr);
  std::string where("ser_no = '~");
  where += serialStr + std::string("'");

  std::vector<FieldVal> fields;
  
  fields.push_back(FieldVal("notes", "1st update: set data_size to non-zero value"));
  fields.push_back(FieldVal("data_size", "883"));
  
  Row row(fields);

  std::string table("metadata_v2r1");

  unsigned nChange = rdb->updateRows(table, row, where);

  unsigned errcode = rdb->getConnection()->getLastError();
  std::cerr << "From doBadUpdate.  Last error code was " << errcode 
            << std::endl;

  if (rdb->duplicateError() ) {
    std::cerr << "Last error was duplicate insert " << std::endl;
  }
  else {
    std::cerr << "Last error was NOT duplicate insert " << std::endl;
  }
  return (int) nChange;
}

int doUpdate(rdbModel::Rdb* rdb, int serial) {
  using rdbModel::Column;
  using facilities::Util;
  using rdbModel::FieldVal;
  using rdbModel::Row;

  // WHERE clause
  std::string serialStr;
  Util::itoa(serial, serialStr);
  std::string where(" where ser_no = '");
  where += serialStr + std::string("'");

  std::vector<FieldVal> fields;
  
  fields.push_back(FieldVal("notes", "Update: set data_size to non-zero value"));
  fields.push_back(FieldVal("data_size", "883"));
  
  Row row(fields);

  std::string table("metadata_v2r1");

  unsigned nChange = rdb->updateRows(table, row, where);
  return (int) nChange;

}
