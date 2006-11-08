// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/test/initRdbMain.cxx,v 1.2 2006/10/25 22:36:50 jrb Exp $
// Program to initialize rdbModel-type database from init file
// satisfying initRdbms.xsd schema

#include <iostream>
#include <string>
#include <cstdlib>
#include "facilities/Util.h"
#include "InitRdb.h"

/*
   Inputs:  Sufficient info to make db connection
            (host, port, dbname.  Expect to get user, password from .my.cnf)
            xml file describing the db, satisfying rdbmsDescrip.xsd;
             check for compatibility
            xml file describing how to initialize, satisfying rdbmsInit.xsd
 */
void outputHelp();

int main(int narg, char** args) {
  //  using rdbModel::FieldVal;

  if (narg < 4) {
    outputHelp();
    return 0;

  }
  // store args
  //  std::string host("glastDB.slac.stanford.edu");
  //  char* defHost = 0;
  //  char* host = 0;
  int port = 0;
  std::string dbname("");
  std::string dfile("");
  std::string ifile("");
  std::string hostArg("");
  bool debug = false;

  for (int iarg = 1; iarg < narg; iarg++) {
    std::string tmp(args[iarg]);
    unsigned eq = tmp.find('=');
    if ((eq > tmp.size() || eq < 4)) {
      std::cerr << "Improperly formed argument: " << tmp << std::endl;
      outputHelp();
      return 0;
    }

    std::string parm = std::string(tmp, 2, eq-2);
    std::string val = std::string(tmp, eq+1);
    std::cout << "Parsed argument " << iarg << "; found value '" << val
              << "' for named parameter "  << parm << std::endl;

    if (parm == std::string("host")) {
      hostArg = val;
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
    else if (parm == std::string("debug")) {
      if ((val == "Y") || (val == "y") || (val == "true") || (val=="1")) {
        debug = true;
      }
    }
    else {
      std::cerr << "unknown argument: " << tmp << std::endl;
      outputHelp();
      return 0;
    }
  }
  const char* host = (hostArg.size() ) ? hostArg.c_str() : 0;
  if (dbname.size() * dfile.size() * ifile.size() == 0) {
    std::cout << "Required argument was not supplied " << std::endl << std::endl;
    outputHelp();
  }

  rdbModel::InitRdb i(debug);
  int ret = i.buildModel(dfile);
  if (ret) return 0;
  // Make db connection and verify compatibility
  // Account info should be in group with name = dbname
  ret = i.dbconnect(host, port, 
                    dbname,
                    dbname);
  if (ret) return 0;
  
  // Handle init file
  ret = i.init(ifile);

  return 0;
  // 
}

void outputHelp() {
  std::cout << 
    "initRdb --db=<dbname> --desc=<fname1> --init=<fname2> --host=<host> --port=<port> --debug=<dbg>" << std::endl << std::endl;;
  std::cout << "  Arguments may be in any order." << std::endl;
  std::cout << "  host defaults to glastDB.slac.stanford.edu" << std::endl;
  std::cout << "  port defaults to 0" << std::endl;
  std::cout << "  dbg defaults to 0 (false) " << std::endl<< std::endl;
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



