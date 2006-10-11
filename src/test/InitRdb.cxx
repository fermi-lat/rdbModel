// $Header: $
// Class to initialize rdbModel-type database from init file
// satisfying initRdbms.xsd schema, invoked from main initRdb.

#include <iostream>
#include <fstream>
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
#include "InitRdb.h"


namespace rdbModel {
  int InitRdb::buildModel(std::string& dfile) {
    Builder* b = new XercesBuilder;
    m_rdb = new Rdb;
    int errcode = m_rdb->build(dfile, b);

    if (errcode) {
      std::cerr << "Build failed with error code " << errcode << std::endl;
      return errcode;
    }
    return 0;

  }

  int InitRdb::dbconnect(const char* host, int port, const std::string& dbname,
                         const std::string& group) {

    // if m_rdb = 0, complain?

    std::ostream* infoOut = 0;   // goes to std::cout
    rdbModel::MysqlConnection* conn = 0;
    if (!m_dbg) {
      infoOut = new std::ofstream("/dev/null");
    }
    conn = new rdbModel::MysqlConnection(infoOut);
    if (!conn) return false;
    bool ok = conn->init();
    if (!ok) {
      delete conn;
      return 1;
    }
    std::string cnf("$(HOME)/.my.cnf");
    facilities::Util::expandEnvVar(&cnf);
    ok = conn->setOption(rdbModel::DBreadDefaultFile, cnf.c_str());
    //if (ok) ok = conn->setOption(rdbModel::DBreadDefaultGroup, "MOOT_WRITE");
    if (ok) ok = conn->setOption(rdbModel::DBreadDefaultGroup, group.c_str());
    if (ok) ok =  conn->open(host, port, NULL, NULL, dbname.c_str());
    if (!ok) return -2;   // no connection
    m_rdb->setConnection(conn);

    rdbModel::MATCH match = m_rdb->getConnection()->matchSchema(m_rdb, false);

    switch (match) {
    case rdbModel::MATCHequivalent:
    case rdbModel::MATCHcompatible:
      if (m_dbg) {
        std::cout << "XML schema and MySQL database are compatible" 
                  << std::endl;
        std::cout.flush();
      }
      return 0; // RETOk;
    case rdbModel::MATCHfail:
      std::cout.flush();
      std::cerr << "XML schema and MySQL database are NOT compatible" 
                << std::endl;
      std::cerr.flush();
      return -1; // RETBadCnfFile;
    case rdbModel::MATCHnoConnection:
      std::cout.flush();
      std::cerr << "Connection failed while attempting match" << std::endl;
      std::cerr.flush();
      return -2; // RETNoConnect;
    }
    std::cout.flush();
    std::cerr.flush();
    return -3; // RETBadValue;
  }

  InitRdb::~InitRdb() {
    if (m_rdb) delete m_rdb;
  }
    
}
