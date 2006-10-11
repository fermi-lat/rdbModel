// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/test/InitRdb.cxx,v 1.1 2006/10/11 00:17:32 jrb Exp $
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
#include "xmlBase/XmlParser.h"
#include "xmlBase/Dom.h"

namespace rdbModel {
  using XERCES_CPP_NAMESPACE_QUALIFIER DOMElement;
  int InitRdb::buildModel(std::string& dfile) {
    m_build = new XercesBuilder;
    m_rdb = new Rdb;
    int errcode = m_rdb->build(dfile, m_build);

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
    if (m_build) delete m_build;
  }

  // Return 0 if no errors or warnings
  int InitRdb::init(std::string& ifile) {
    //    using XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument;
    using xmlBase::Dom;
    int   ret;

    xmlBase::XmlParser parser;

    parser.doSchema(true);

    m_doc = parser.parse(ifile.c_str());

    DOMElement* top = m_doc->getDocumentElement();
    std::vector<DOMElement*> tables;
    Dom::getChildrenByTagName(top, std::string("table"), tables);
    if (m_dbg) std::cout << std::endl << tables.size() 
                         << " tables found in document " << ifile
                         << std::endl;

    for (unsigned i = 0; i < tables.size(); i++) {
      ret = handleTable(tables[i]);
      if (ret > 0) return ret;  // error
    }
    return 0;
    
  }

  int InitRdb::handleTable(DOMElement* tblElt) {
    using xmlBase::Dom;
    std::string name = Dom::getAttribute(tblElt, "name");
    if (m_dbg) 
      std::cout << "Processing table named '" << name << "'" << std::endl;

    // Make sure we have a table with this name
    Table* tbl = m_rdb->getTable(name);
    if (!tbl) {
      std::cerr << "Cannot initialize table '" << name 
                << "' -- no such table in db" << std::endl;
      return 1;
    }
    return 0;
  }
}
