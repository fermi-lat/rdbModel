// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/test/test_build.cxx,v 1.24 2005/11/04 21:45:31 jrb Exp $
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

// When TEST_INSERT is defined, use connection which can write
#define TEST_INSERT

int doInsert(rdbModel::Rdb* con);
int doUpdate(rdbModel::Rdb*, int serial);

int main(int, char**) {
  using rdbModel::FieldVal;

  std::string infile("$(RDBMODELROOT)/xml/calib_test.xml");

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
  std::string connectfileT("$(RDBMODELROOT)/xml/connect/mysqlTester.xml");
#else
  // This is glastreader, calib_test
  std::string connectfileT("$(RDBMODELROOT)/xml/connect/mysqlSlacT.xml");
#endif
  


  // Connect to production database, read only
  rdbModel::MysqlConnection* con = new rdbModel::MysqlConnection();

  std::string connectfile("$(RDBMODELROOT)/xml/connect/mysqlSlac.xml");

  if (!(con->open(connectfile)) ) {
    std::cerr << "Unable to connect to MySQL database" << std::endl;
    return -1;
  }


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

  /*


// Following will do an insert if disable = set to false.  
// To keep from cluttering up the
// database, mostly don't execute
//  
# ifdef TEST_INSERT

  bool disable = true;
  con->disableModify(disable);     // so don't really change db
  int serial = doInsert(rdb);
  if (serial) {
    std::cout << "Hallelujah!  Inserted new row, serial# " 
              << serial  << std::endl;
  } else if (disable) {  // pick random serial# to check out disabled update
    serial = 17;
  }
  if (serial) {

    int nUpdates = doUpdate(rdb, serial);

    if (nUpdates) {
      std::cout << "Did " << nUpdates << " on row " << serial
                << std::endl;
    }
    else std::cout << "Failed to update row " << serial << std::endl;
  }
  else {
    std::cout << "Bah, humbug.  Insert failed. " << std::endl;
  }

#else


  // Check that we can really do something with this connection

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
    break;
    //    return -2;
  case rdbModel::MATCHnoConnection:
    std::cout << "Connection failed while attempting match" << std::endl;
    return -1;
  }

  if (match == rdbModel::MATCHfail) { // try again without dbname match
    match = con->matchSchema(rdb, false);

    switch (match) {
    case rdbModel::MATCHequivalent:
      std::cout << "XML schema and MySQL database are equivalent!" 
                << std::endl;
      break;
    case rdbModel::MATCHcompatible:
      std::cout << "XML schema and MySQL database are compatible" << std::endl;
      break;
    case rdbModel::MATCHfail:
      std::cout << "XML schema and MySQL database are NOT compatible" 
                << std::endl;
      //    return -2;
    case rdbModel::MATCHnoConnection:
      std::cout << "Connection failed while attempting match" << std::endl;
      return -1;
    }

  }
#endif
  */
  return 0;
}

// int doInsert(rdbModel::Connection* con) {
int doInsert(rdbModel::Rdb* rdb) {
  
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

  int  serial = 0;
  
  Row row(fields);

  rdb->insertRow("metadata_v2r1", row, &serial);
    
  return serial;
}

// int doUpdate(rdbModel::Connection* con, int serial) {
int doUpdate(rdbModel::Rdb* rdb, int serial) {
  using rdbModel::Assertion;
  using rdbModel::Column;
  using facilities::Util;
  using rdbModel::FieldVal;
  using rdbModel::Row;

  // Set up WHERE clause, always the same
  std::string serialStr;
  Util::itoa(serial, serialStr);
  Assertion::Operator* serEquals = 
    new Assertion::Operator(rdbModel::OPTYPEequal, "ser_no",
                            serialStr, rdbModel::FIELDTYPEold, 
                            rdbModel::FIELDTYPElit);

  Assertion* whereSer = new Assertion(serEquals);
  //  Assertion* whereSer = new Assertion(Assertion::WHENwhere, serEquals);

  // First call an update without any null columns; change notes field
  // and set data_size to something.
  /*  std::vector<std::string> colNames, values, nullCols; */
  std::vector<FieldVal> fields;
  
  fields.push_back(FieldVal("notes", "1st update: set data_size to non-zero value"));
  fields.push_back(FieldVal("data_size", "883"));
  
  Row row(fields);

  std::string table("metadata_v2r1");

  unsigned nChange = rdb->updateRows(table, row, whereSer);

  // Now null out data_size
  fields.clear();
  fields.push_back(FieldVal("data_size", "", true));
  fields.push_back(FieldVal("notes", "2nd update: data_size set to NULL"));
  Row row2(fields);

  nChange += rdb->updateRows(table, row2, whereSer);
  return nChange;
}
