// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/test/test_build.cxx,v 1.24 2005/11/04 21:45:31 jrb Exp $
// Program to initialize rdbModel-type database from init file
// satisfying initRdbms.xsd schema

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


/*
   Inputs:  Sufficient info to make db connection
            (host, port, dbname.  Expect to get user, password from .my.cnf)
            xml file describing the db, satisfying rdbmsDescrip.xsd;
             check for compatibility
            xml file describing how to initialize, satisfying rdbmsInit.xsd
 */
void outputHelp();

int main(int narg, char** args) {
  using rdbModel::FieldVal;

  if (narg < 4) {
    outputHelp();
    return 0;

  }
  // store args
  std::string host("glastDB.slac.stanford.edu");
  int port = 0;
  std::string dbname("");
  std::string dfile("");
  std::string ifile("");

  for (int iarg = 1; iarg < narg; iarg++) {
    std::string tmp(args[iarg]);
    unsigned eq = tmp.find('=');
    if ((eq > tmp.size() || eq < 4)) {
      std::cout << "Improperly formed argument: " << tmp << std::endl;
      outputHelp();
      return 0;
    }

    std::string parm = std::string(tmp, 2, eq-2);
    std::string val = std::string(tmp, eq+1);
    std::cout << "Parsed argument " << iarg << "; found value '" << val
              << "' for named parameter "  << parm << std::endl;

    if (parm == std::string("host")) {
      host = val;
    }
    else if (parm == std::string("port")) {
      port = facilities::Util::stringToInt(val);
    }
    else if (parm == std::string("db")) {
      dbname = val;
    }
    else if (parm == std::string("desc")) {
      dfile = val;
    }
    else if (parm == std::string("init")) {
      ifile = val;
    }
  }

  if (dbname.size() * dfile.size() * ifile.size() == 0) {
    std::cout << "Required argument was not supplied " << std::endl << std::endl;
    outputHelp();
  }
  return 0;
  /*
  rdbModel::Builder* b = new rdbModel::XercesBuilder;
  rdbModel::Rdb* rdb = new rdbModel::Rdb;
  int errcode = rdb->build(infile, b);

  if (errcode) {
    std::cerr << "Build failed with error code " << errcode << std::endl;
    return errcode;
  }

  */
  // stuff
}

/*
int doInsert(rdbModel::Rdb* rdb) {
  
  using rdbModel::FieldVal;
  using rdbModel::Row;

  // or maybe this belongs in another class
  return serial;
}
*/
void outputHelp() {
  std::cout << 
    "initRdb --db=<dbname> --desc=<fname1> --init=<fname2> --host=<host> --port=<port>" << std::endl << std::endl;;
  std::cout << "  Arguments may be in any order." << std::endl;
  std::cout << "  host defaults to glastDB.slac.stanford.edu" << std::endl;
  std::cout << "  port defaults to 0" << std::endl << std::endl;
  std::cout << "  dbname is name of MySQL database to be initialized," 
            << std::endl << "  e.g. mood_test" << std::endl << std::endl;
  std::cout << 
    "  fname1 is file path for xml file describing db structure " << std::endl
            << "  according to schema rdbmsDescrip.xsd" 
            << std::endl << std::endl;
  std::cout << 
    "  fname2 is file path for xml file describing initialization" << std::endl
            <<"  according to schema rdbmsInit.xsd" << std::endl;
}
