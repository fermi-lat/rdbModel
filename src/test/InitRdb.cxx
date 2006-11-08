// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/test/InitRdb.cxx,v 1.6 2006/10/25 22:36:50 jrb Exp $
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
#include "facilities/Util.h"
#include "InitRdb.h"
#include "xmlBase/XmlParser.h"
#include "xmlBase/Dom.h"

namespace {
  using XERCES_CPP_NAMESPACE_QUALIFIER DOMElement;
  // Check that Table has this field and that value is compatible
  // with column definition
  int handleFieldType(DOMElement* fieldElt, rdbModel::Table* tbl,
                      std::string& fieldName, std::string& fieldValue) {
    using xmlBase::Dom;
    fieldName = Dom::getAttribute(fieldElt, "col");
    rdbModel::Column* column = tbl->getColumnByName(fieldName);
    if (!column) {
      std::cerr << "Table " << tbl->getName() << " has no such column as"
                << fieldName << std::endl;
      return 1;
    }
    fieldValue = Dom::getAttribute(fieldElt, "val");
    if (!(column->okValue(fieldValue))) {
      std::cerr << "Value " << fieldValue 
                << "incompatible with column definition for column " 
                << fieldName << " in table " << tbl->getName() << std::endl;
      return 2;
    }
    return 0;
  }


}
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
    int ret;
    if (m_dbg) 
      std::cout << "Processing table named '" << name << "'" << std::endl;

    // Make sure we have a table with this name
    Table* tbl = m_rdb->getTable(name);
    if (!tbl) {
      std::cerr << "Cannot initialize table '" << name 
                << "' -- no such table in db" << std::endl;
      return 1;
    }
    // Process <forAll>s
    std::vector<DOMElement*> elts;
    Dom::getChildrenByTagName(tblElt, std::string("forAll"), elts);
    std::vector<rdbModel::FieldVal> forAllFields;
    forAllFields.reserve(elts.size());
    for (unsigned i = 0; i < elts.size(); i++) {
      std::string fieldName;
      std::string fieldValue;
      ret = handleFieldType(elts[i], tbl, fieldName, fieldValue);
      if (ret) return ret;
      forAllFields.push_back(FieldVal(fieldName, fieldValue));
    }
    // Remaining children should all be <row>
    elts.clear();
    Dom::getChildrenByTagName(tblElt, std::string("row"), elts);
    for (unsigned iRow = 0; iRow < elts.size(); iRow++) {
      unsigned newKey;
      ret = handleRow(elts[iRow], tbl, forAllFields, &newKey);
      if (ret) return ret;
    }
    return 0;
  }
  int InitRdb::handleRow(DOMElement* row, Table* tbl, 
                         const std::vector<FieldVal>& forAllFields,
                         unsigned* newKey) {
    using xmlBase::Dom;
    int ret;

    std::vector<DOMElement*> children;
    std::vector<FieldVal> fields = forAllFields;
    Dom::getChildrenByTagName(row, std::string("*"), children);
    for (unsigned i = 0; i < children.size(); i++) {
      DOMElement* child = children[i];
      std::string tagname = Dom::getTagName(child);
      if (tagname == "field") {
        std::string col, val;
        ret = handleFieldType(child, tbl, col, val);
        if (ret) return ret;
        fields.push_back(FieldVal(col, val));
      }
      else if (tagname == "ref") {
        std::string col, val;
        ret = handleRef(child, col, val);
        if (ret) return ret;
        fields.push_back(FieldVal(col, val));
      }
      else {  // should never happen
        std::cerr << "Encountered unknown tag '"  << tagname << "'" 
                  << std::endl;
        return 3;
      }
    }
    Row toInsert(fields);
    const std::string tname = tbl->getName();
    try {
      m_rdb->insertRow(tname, toInsert, 0, newKey);
    }
    catch (RdbException ex) {
      std::cerr << "Insert row for table " << tname
                << " failed with error" << std::endl
                << "***  " << ex.what()  << std::endl;
      return 4;
    }
    if (m_dbg) {
      std::cout << "Inserted row into table " << tbl->getName();
      if (newKey) std::cout  << ", assigned key " << *newKey;
      std::cout << std::endl;
    }
    return 0;
    // Should be ready to do insert 
  }
  int InitRdb::handleRef(DOMElement* refElt,
                         std::string& col, std::string& val) {
    using xmlBase::Dom;

    col = Dom::getAttribute(refElt, "col");
    
    std::string refTableName = Dom::getAttribute(refElt, "table");
    std::string refCol = Dom::getAttribute(refElt, "refCol");
    std::string refVal = Dom::getAttribute(refElt, "refVal");
    Table* refTable = m_rdb->getTable(refTableName);
    if (!refTable) {
      std::cerr << "No table '" << refTableName << "' in dbs" << std::endl;
      return 1;
    }

    // Find colname for primary key of 'table'
    std::string primCol = refTable->getPrimaryKeyCol();
    if (primCol.size() == 0) {
      std::cerr << "Table " << refTable << " has no primary key " 
               << std::endl;
      return 1;
    }
    
    std::vector<std::string> getCols;
    getCols.push_back(primCol);

    // do sql
    //   select [primary key] from [table] where
    //             [refCol] = [refVal]
    std::string where(" WHERE ");
    where += refCol + std::string("='") + refVal + std::string("'");
    ResultHandle* res;
    try {
      res = m_rdb->getConnection()->select(refTableName, getCols, getCols, where);
    }
    catch (std::exception ex) {
      std::cerr << "InitRdb::handleRef " << ex.what() << std::endl;
      std::cerr.flush();
      if (res) delete res;
      return 1;
    }
    int got = res->getNRows();
    if (got != 1) {
      if (got == 0) {
        std::cerr << "No rows found in table " << refTableName 
                  << " satisfying " << where << std::endl;
      }
      else if (got > 1) {
        std::cerr << "Too many rows found in table " << refTableName 
                  << " satisfying " << where << std::endl;
      }
      delete res;
      return 1;
    }
    std::vector<std::string>selFields;

    res->getRow(selFields);
    val = selFields[0];
    delete res;
    return 0;
  }

    
}
