// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/test/test_build.cxx,v 1.3 2004/03/09 01:42:36 jrb Exp $
// Test program for rdbModel primitive buiding blocks

#include <iostream>
#include <string>
#include "rdbModel/Rdb.h"
#include "rdbModel/Management/Manager.h"
#include "rdbModel/Management/XercesBuilder.h"
#include "rdbModel/Db/MysqlConnection.h"
#include "rdbModel/Db/MysqlResults.h"


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


  // Connect to real database
  rdbModel::MysqlConnection* con = new rdbModel::MysqlConnection();

  if (!(con->open(std::string("centaurusa.slac.stanford.edu"),
                  std::string("glastreader"),
                 std::string("glastreader"), std::string("calib") ) ) ){
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

  return 0;
}
  
  

