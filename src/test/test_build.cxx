// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/test/test_build.cxx,v 1.11 2004/06/21 23:18:50 jrb Exp $
// Test program for rdbModel primitive buiding blocks

#include <iostream>
#include <string>
#include "rdbModel/Rdb.h"
#include "rdbModel/RdbException.h"
#include "rdbModel/Management/Manager.h"
#include "rdbModel/Management/XercesBuilder.h"
#include "rdbModel/Db/MysqlConnection.h"
#include "rdbModel/Db/MysqlResults.h"
#include "rdbModel/Tables/Column.h"
#include "rdbModel/Tables/Datatype.h"
#include "rdbModel/Tables/Assertion.h"
#include "facilities/Util.h"

// #define TEST_INSERT

int doInsert(rdbModel::Connection* con);
int doUpdate(rdbModel::Connection*, int serial);
int main(int, char**) {
  std::string infile("$(RDBMODELROOT)/xml/calibTest.xml");

  rdbModel::Manager* man = rdbModel::Manager::getManager();

  man->setBuilder(new rdbModel::XercesBuilder);
  man->setInputSource(infile);

  // good errcode is 0
  int errcode = man->build();

  if (errcode) {
    std::cerr << "Build failed with error code " << errcode << std::endl;
    return errcode;
  }
  rdbModel::Rdb* rdb = man->getRdb();

  std::string colMin;
  std::string colMax;
  
  rdbModel::Column* serCol = rdb->getColumn("metadata_v2r1", "ser_no");
  if (serCol) {
    rdbModel::Datatype* serType = serCol->getDatatype();
    
    if (serType->getInterval(colMin, colMax) ) {
      std::cout << "Min and max for ser_no are " << colMin 
                << ", " << colMax << std::endl;
    }  else {
      std::cout << "ser_no has no min, max " << std::endl;
    }
  }
  else std::cout << "no such column as 'ser_no' " << std::endl;

  rdbModel::Column* vstartCol = rdb->getColumn("metadata_v2r1", "vstart");
  if (vstartCol) {
    rdbModel::Datatype* vstartType = vstartCol->getDatatype();

    if (vstartType->getInterval(colMin, colMax) ) {
      std::cout << "Min and max for vstart are " << colMin 
                << ", " << colMax << std::endl;
    }  else {
      std::cout << "vstart has no min, max " << std::endl;
    }
  }
  else std::cout << "no such column as 'vstart' " << std::endl;

  // mostly don't want to run code doing an insert.  For times
  // when we do, must connect as user with INSERT priv.
#ifdef TEST_INSERT
  std::string connectfileT("$(RDBMODELROOT)/xml/connect/mysqlTester.xml");
#else
  std::string connectfileT("$(RDBMODELROOT)/xml/connect/mysqlSlacT.xml");
#endif
  
  // Connect to real database
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
  std::string rq[2];
  rq[0] ="select * from metadata_v2r1";
  rq[1] ="select garbage from metadata_v2r1";
  for (int i = 0; i < 2; i++) {
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
      std::cerr << "Code " << ex.getCode() << std::endl;
    }
  }


  // Now try to close connection, then reopen.
  con->close();

  if (!(con->open(connectfile)) ) {
    std::cerr << "Unable to connect to MySQL database" << std::endl;
    return -1;
  }
  con->close();

  // Now open with alternate connection file
  if (!(con->open(connectfileT)) ) {
    std::cerr << "Unable to connect to MySQL database" << std::endl;
    return -1;
  }

    // Following will do an insert.  To keep from cluttering up the
    // database, mostly don't execute
    //  
#ifdef TEST_INSERT
    int serial = doInsert(con);
    if (serial) {
      std::cout << "Hallelujah!  Inserted new row, serial# " 
                << serial  << std::endl;

      // Now try update
      int nUpdates = doUpdate(con, serial);

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
  match = con->matchSchema(rdb);

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

  return 0;
}

int doInsert(rdbModel::Connection* con) {
  
    std::vector<std::string> cols;
    std::vector<std::string> vals;
    std::vector<std::string> nullCols;
    cols.push_back("instrument");
    vals.push_back("LAT");

    cols.push_back("calib_type");
    vals.push_back("Test_Gen");

    cols.push_back("flavor");
    vals.push_back("zin");

    cols.push_back("data_fmt");
    vals.push_back("nonsense");

    cols.push_back("vstart");
    vals.push_back("2003-02-01");

    cols.push_back("vend");
    vals.push_back("2020-02-01");

    cols.push_back("data_size");
    vals.push_back("0");

    cols.push_back("locale");
    vals.push_back("phobos");

    cols.push_back("completion");
    vals.push_back("ABORT");

    cols.push_back("creator");
    vals.push_back("test_build");

    cols.push_back("uid");
    vals.push_back("jrb");

    cols.push_back("data_ident");
    vals.push_back("$(mycalibs)/test/moreJunk.xml");

    // null this one out
    //    cols.push_back("input_desc");
    // vals.push_back("imagination");

    cols.push_back("notes");
    vals.push_back("Absurd test item, setting input_desc to NULL");

    nullCols.push_back("input_desc");
    //    nullCols.push_back("vend");

    int  serial = 0;
  
    con->insertRow("metadata_v2r1", cols, vals, &serial, &nullCols);
    return serial;
}

int doUpdate(rdbModel::Connection* con, int serial) {
  using rdbModel::Assertion;
  using facilities::Util;

  // Set up WHERE clause, always the same
  std::string serialStr;
  Util::itoa(serial, serialStr);
  Assertion::Operator* serEquals = 
    new Assertion::Operator(rdbModel::OPTYPEequal, "ser_no",
                            serialStr, false, true);

  Assertion* whereSer = new Assertion(Assertion::WHENwhere, serEquals);

  // First call an update without any null columns; change notes field
  // and set data_size to something.
  std::vector<std::string> colNames, values, nullCols;
  colNames.push_back(std::string("notes"));
  colNames.push_back(std::string("data_size"));

  values.push_back(std::string("1st update: set data_size to non-zero value"));
  values.push_back(std::string("883"));

  std::string table("metadata_v2r1");
  unsigned nChange = con->update(table, colNames, values, whereSer);

  // Now null out data_size
  nullCols.push_back("data_size");
  colNames.clear();
  colNames.push_back(std::string("notes"));
  values.clear();
  values.push_back(std::string("2nd update: data_size set to NULL"));
  nChange += con->update(table, colNames, values, whereSer, &nullCols);

  return nChange;
}

