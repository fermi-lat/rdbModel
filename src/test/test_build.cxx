// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/test/test_build.cxx,v 1.7 2004/04/10 01:16:27 jrb Exp $
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

int main(int, char**) {
  std::string infile("$(RDBMODELROOT)/xml/calibMetaDb.xml");

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

  // Connect to real database
  rdbModel::MysqlConnection* con = new rdbModel::MysqlConnection();

  std::string connectfile("$(RDBMODELROOT)/xml/mysqlSlac.xml");

  /*
  if (!(con->open(std::string("centaurusa.slac.stanford.edu"),
                  std::string("glastreader"),
                 std::string("glastreader"), std::string("calib") ) ) ){
    std::cerr << "Unable to connect to MySQL database" << std::endl;
    return -1;
  }
  */
  
  if (!(con->open(connectfile)) ) {
    std::cerr << "Unable to connect to MySQL database" << std::endl;
    return -1;
  }


  rdbModel::MATCH match = con->matchSchema(rdb);

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
    return -2;
  case rdbModel::MATCHnoConnection:
    std::cout << "Connection failed while attempting match" << std::endl;
    return -1;
  }

  return 0;
}
  
  

