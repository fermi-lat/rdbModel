// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/Db/MysqlConnection.cxx,v 1.53 2007/11/12 19:54:48 jrb Exp $
#ifdef  WIN32
#include <windows.h>
#endif

#include "rdbModel/Db/MysqlConnection.h"
#include "rdbModel/Rdb.h"
#include "rdbModel/Tables/Table.h"
#include "rdbModel/Tables/Assertion.h"
#include "rdbModel/Tables/Column.h"
#include "rdbModel/Tables/Datatype.h"
#include "rdbModel/Db/MysqlResults.h"
#include "rdbModel/RdbException.h"
#include "facilities/Util.h"

#include "xmlBase/XmlParser.h"
#include "xmlBase/Dom.h"

#include "mysql/mysql.h"
#include "mysql/errmsg.h"
#include <iostream>
#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include "facilities/Util.h"
namespace {

  // Size specification is of form (m) or (m,d)  If no size specification 
  // return 0; else return value of m.
  int extractSize(const std::string& sqlString) {
    unsigned leftLoc = sqlString.find("(");
    if (leftLoc == std::string::npos) return 0;
    leftLoc++;           // now is at start of m
    unsigned rightLoc = sqlString.find(",");
    if (rightLoc == std::string::npos) {
      rightLoc = sqlString.find(")");
    }
    std::string numString = 
      sqlString.substr(leftLoc, rightLoc - leftLoc);
    return facilities::Util::stringToInt(numString);
  }

  void addArg(bool literal, const std::string arg, std::string& sqlString) {
    if (literal) sqlString += '"';
    sqlString += arg;
    if (literal) sqlString += '"';
    return;
  }

  bool compareEnumList(const std::vector<std::string>& choices, 
                       std::string sqlType) {
    // Number has to be the same.  
    unsigned locComma = sqlType.find(",");
    unsigned nComma = 0;
    while (locComma != std::string::npos) {
      nComma++;
      locComma = sqlType.find(",", locComma+1);
    }
    unsigned nChoice = choices.size();
    if (nChoice != (nComma + 1)) return false;
    for (unsigned iChoice = 0; iChoice < nChoice; iChoice++) {
      unsigned loc = sqlType.find(choices[iChoice]);
      if (loc == std::string::npos) return false;
    }
    return true;
  }
  std::string getMysqlError(MYSQL* my, unsigned* myErrno) {
    if ((*myErrno = mysql_errno(my)) ) {
      const char* errstring = mysql_error(my);
      std::string report("MySQL error code "), code;
      facilities::Util::utoa(*myErrno, code);
      report += code + std::string(": ") + std::string(errstring);
      return report;
    }
    return std::string("");
  }
  unsigned 
  int realQuery(MYSQL* my, std::string const& qstring) {
    unsigned long sz = qstring.size();
    const char* str = qstring.c_str();
    return mysql_real_query(my, str, sz);
  }
  std::string mysqlEscape(MYSQL* my, std::string const& s)
  {
    char * asciiz = new char[s.size()*2+1];
    unsigned sz = mysql_real_escape_string(my, asciiz, s.c_str(), s.size());
    std::string result(asciiz, sz);
    delete [] asciiz;
    return result;
  }
}
    
namespace rdbModel {
  bool   MysqlConnection::m_compileInit = false;

  MysqlConnection::MysqlConnection(std::ostream* out,
                                   std::ostream* errOut) :
    Connection(out, errOut),
    m_mysql(0), m_connected(0), m_host(""), m_port(-1),
    m_user(""), m_pw(""), m_dbName(""),
    m_visitorType(VISITORundefined), m_rdb(0), m_tempRes(0),
    m_writeDisabled(false), m_maxRetry(2), m_avgWait(10000)
  {
    // Seed random number generator with a random thing
    srand(time(0));
  }

  bool MysqlConnection::close() {
    if (m_tempRes) {
      mysql_free_result(m_tempRes);
      m_tempRes = 0;
    }
    mysql_close(m_mysql);
    m_mysql = 0;
    m_connected = false;
    return true;
  }

  MysqlConnection::~MysqlConnection() {
    close();
    delete m_mysql;
    return;
  }

  bool MysqlConnection::open(const std::string& host, 
                             const std::string& user,
                             const std::string& password,
                             const std::string& dbName) {
    return open(host, user.c_str(), password.c_str(), dbName.c_str());
  }


  bool MysqlConnection::init() {
    m_mysql = new MYSQL;
    if (!m_mysql) return false;
    mysql_init(m_mysql);
    return true;
  }
  
  bool MysqlConnection::setOption(DBOPTIONS option, const char* value)
  {
    if (!m_mysql) return false;
    mysql_option myOpt;
    if (option == DBreadDefaultFile) myOpt = MYSQL_READ_DEFAULT_FILE;
    else if (option == DBreadDefaultGroup) myOpt = MYSQL_READ_DEFAULT_GROUP;
    else return false;
    int ret =  mysql_options(m_mysql, myOpt, value);
    return (ret == 0);
  }

  bool MysqlConnection::open(const std::string& host, 
                             const char* user,
                             const char* password,
                             const char* dbName) {
                             //     , unsigned int       port) {
    if (dbName == 0) {
      (*m_err) << 
        "rdbModel::MysqlConnection::open : null db name not allowed!" <<
        std::endl;
      m_err->flush();
      return false;
    } 

    if (!m_mysql) {
      bool ok = init();
      if (!ok) return false;
    }
    // 'host' argument is of the form hostname[:port]
    //  That is, port section is optional.  If not supplied, use
    // default port.
    std::string hostOnly;
    int port = 0;
    unsigned int colonLoc = host.find(":");
    if (colonLoc == std::string::npos) {
      hostOnly = host;
    }
    else {
      hostOnly = host.substr(0, colonLoc);
      std::string portString = host.substr(colonLoc+1);
      try {
        port = facilities::Util::stringToInt(portString);
      }
      catch (facilities::WrongType ex) {
        (*m_err) << "From MysqlConnection::connect.  Bad port: "
                 << ex.getMsg() << std::endl;
        m_err->flush();
        return false;
      }

    }

    return open(hostOnly.c_str(), port, user, password, dbName);
  }
  
  bool MysqlConnection::open(const char* host, 
                             int         port,
                             const char* user,
                             const char* password,
                             const char* dbName) {
  
    if (!m_mysql) {
      bool ok = init();
      if (!ok) return false;
    }

    MYSQL *connected = mysql_real_connect(m_mysql, host,
                                          user,
                                          password, dbName,
                                          port, NULL, 0);

    if (connected != 0) {  // Everything is fine.  Put out an info message
      (*m_out) << "Successfully connected to MySQL host " 
               << ((host != 0) ? host : "from init file" )
               << ", database " << dbName << std::endl;
      m_out->flush();
      m_connected = true;
      if (host) m_host = std::string(host);
      else m_host = std::string("");
      m_port = port;
      if (user) m_user = std::string(user);
      else m_user = std::string("");
      if (password) m_pw = std::string(password);
      else m_pw = std::string("");
      m_dbName = dbName;
    }
    else {
      (*m_err) <<  "Failed to connect to MySQL host " << host <<
        "with error " << mysql_error(m_mysql) << std::endl;
      m_err->flush();
      m_connected = false;
    }
    return m_connected;
  }

  bool MysqlConnection::open(const std::string& parms) {
    using XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument;
    using XERCES_CPP_NAMESPACE_QUALIFIER DOMElement;
    xmlBase::XmlParser parser;
    DOMDocument* doc = parser.parse(parms.c_str(), "mysqlConnection");
    if (doc == 0) {
      (*m_err) << "parse of connection parameters failed" << std::endl;
      m_err->flush();
      return false;
    }
    DOMElement*  conn = doc->getDocumentElement();
    
    std::string host = xmlBase::Dom::getAttribute(conn, "host");
    std::string user = xmlBase::Dom::getAttribute(conn, "user");
    std::string password = xmlBase::Dom::getAttribute(conn, "password");
    std::string dbname = xmlBase::Dom::getAttribute(conn, "dbname");

    if (password.size() == 0 ) { // prompt for password?
      (*m_out) << "interactive login NYI " << std::endl;
      m_out->flush();
      return false;
    }

    return this->open(host, user, password, dbname);
  }



  MATCH MysqlConnection::matchSchema(Rdb *rdb, bool matchDbName) {
    if (!m_connected) return MATCHnoConnection;

    m_matchDbName = matchDbName;

    // Check global characteristics; 
    // Could do this via Manager; seems a bit artificial, bypass for now
    m_visitorType = VISITORmatch;
    m_matchReturn = MATCHequivalent;
    unsigned int ret = rdb->accept(this);

    if ((ret == Visitor::VERROR) || (ret == Visitor::VERRORABORT)) {
      return MATCHfail;
    }
    else return m_matchReturn;
  }

    // For each table
    //         compare # of columns
    //         compare datatype description, other attributes of column
    //         compare indices


  std::string MysqlConnection::formatField(Datatype const* dtype,
					   std::string const& value) const
  {
    /* Essentially make sure that the string has the correct size. The
       other verifications (syntax, enum-correctness, ...) are made by
       mysql itself */
    switch (dtype->getType())
      {
#define CASE_TYPELEN(tname,szmax) \
      case Datatype::TYPE ## tname: \
        { \
	  if (value.size() >= szmax)  \
	    throw RdbException("formatField: contents too large for " \
		  	       #tname " field"); \
        }

	CASE_TYPELEN(tinyblob, 1<<8) break;
	CASE_TYPELEN(blob, 1<<16) break;
	CASE_TYPELEN(mediumblob, 1<<24) break;
	// Nothing for longblobs: value.size() is unsigned int, which is 32b
	// even on 64b hosts

	CASE_TYPELEN(tinytext, 1<<8) break;
	CASE_TYPELEN(text, 1<<16) break;
	CASE_TYPELEN(mediumtext, 1<<24) break;
	// Nothing for longtexts: value.size() is unsigned int, which is 32b
	// even on 64b hosts

      default:
	break;
      }

    return mysqlEscape(m_mysql, value);
  }


  bool MysqlConnection::insertRow(const std::string& tableName, 
                                  const StringVector& colNames, 
                                  const StringVector& values,
                                  int* auto_value,
                                  const StringVector* nullCols,
                                  unsigned int* u_auto_value) {
    std::string ins;
    if (auto_value) *auto_value = 0;

    // check that sizes of vectors match
    unsigned  nCol = colNames.size();    
    if (!nCol || (nCol != values.size()  ) ) {
      (*m_err) << " MysqlConnection::insertRow: vector lengths incompatible"
                << std::endl;
      m_err->flush();
      return false;
    }

    // caller should already have checked for validity and should
    // have supplied all necessary columns

    ins += "insert into " + tableName;
    ins += " set " + colNames[0] + "='" + values[0] + "' ";
    for (unsigned iCol = 1; iCol < nCol; iCol++) {
      ins += ", " + colNames[iCol] + "='" + values[iCol] + "' ";
    }
    if (nullCols) {
      if (nullCols->size() > 0) {
        unsigned nNull = nullCols->size();
        for (unsigned iNull = 0; iNull < nNull; iNull++) {
          ins += ", " + (*nullCols)[iNull] + "= NULL ";
        }
      }
    }

    (*m_out) << std::endl << "# INSERT string is:" << std::endl;
    (*m_out) << ins << std::endl;
    m_out->flush();

    if (m_writeDisabled) {
      (*m_out) << "write to Db previously disabled; INSERT not sent"
               << std::endl;
      m_out->flush();
      return true;
    }

    // For the time being, no retries for code which writes to db
    int mysqlRet = realQuery(m_mysql, ins);

    if (mysqlRet) {
      unsigned errcode;
      (*m_err) << "MySQL error during INSERT"  << std::endl;
      (*m_err) << getMysqlError(m_mysql, &errcode) << std::endl;
      m_err->flush();
      return false;
    }
    if ((auto_value) || (u_auto_value)) {
      my_ulonglong id = mysql_insert_id(m_mysql);
      if (auto_value) *auto_value = id;
      if (u_auto_value) *u_auto_value = id;
    }
    return true;
  }

  unsigned int MysqlConnection::update(const std::string& tableName, 
                                       const StringVector& colNames, 
                                       const StringVector& values,
                                       const Assertion* where,
                                       const StringVector* nullCols) {
    
    std::string whereString("");
    if (where) {
      whereString += " WHERE ";
      bool ret = compileAssertion(where, whereString);
      if (!ret) return 0;
    }
    return update(tableName, colNames, values, whereString, nullCols);
  }

  unsigned int MysqlConnection::update(const std::string& tableName, 
                                       const StringVector& colNames, 
                                       const StringVector& values,
                                       const std::string& where,
                                       const StringVector* nullCols) {

    unsigned int nCol = colNames.size();
    if (nCol != values.size()) {
      (*m_err) << "rdbModel::mysqlConnection::update: ";
      (*m_err) << "Incompatible vector arguments " << std::endl;
      m_err->flush();
      return 0;
    }
    std::string sqlString = "UPDATE " + tableName + " SET ";
    sqlString += colNames[0] + " = '" + values[0] + "'";
    for (unsigned int iCol = 1; iCol < nCol; iCol++) {
      sqlString += "," + colNames[iCol] + " = '" + values[iCol] + "'";
    }
    if (nullCols) {
      unsigned nNull = nullCols->size();
      for (unsigned iNull = 0; iNull < nNull; iNull++) {
        sqlString += ", " + (*nullCols)[iNull] + "= NULL ";
      }
    }

    sqlString += where;

    (*m_out) << std::endl << "#  UPDATE to be issued:" << std::endl;
    (*m_out) << sqlString << std::endl;
    m_out->flush();
    if (m_writeDisabled) {
      (*m_out) << "write to Db previously disabled; UPDATE not sent"
               << std::endl;
      m_out->flush();
      return 0;
    }
    // No retries for code which modifies db
    int mysqlRet = realQuery(m_mysql, sqlString);

    if (mysqlRet) {
      unsigned errcode;
      (*m_err) << "rdbModel::MysqlConnection::update: ";
      (*m_err) << "MySQL error during UPDATE" << std::endl;
      (*m_err) << getMysqlError(m_mysql, &errcode) << std::endl;
      m_err->flush();
      return 0;
    }
    my_ulonglong nModLong = mysql_affected_rows(m_mysql);
    // Not much chance that we'll change more rows than will fit in just long
    unsigned nMod = nModLong;
    return nMod;
  }

  // This should now just do the compileAssertion on the 'where'
  // argument and call the other version.
  ResultHandle* MysqlConnection::select(const std::string& tableName,
                                        const StringVector& getCols,
                                        const StringVector& orderCols,
                                        const Assertion* where,
                                        int   rowLimit,
                                        int   rowOffset) {
    std::string sqlWhere;

    if (where != 0) {
      sqlWhere = std::string(" WHERE ");
      bool ret = compileAssertion(where, sqlWhere);
      if (!ret) return 0;
    }
    else sqlWhere.clear();

    return select(tableName, getCols, orderCols, sqlWhere, rowLimit,
                  rowOffset);
  }

    /**
      Alternate form of select, where condition is just a string.
      String should either be empty or should start with WHERE
    */
  ResultHandle* MysqlConnection::select(const std::string& tableName,
                                        const StringVector& getCols,
                                        const StringVector& orderCols,
                                        const std::string& where,
                                        int   rowLimit,
                                        int   rowOffset) {
    std::string sqlString = "SELECT ";
    unsigned nGet = getCols.size();
    unsigned nOrder = orderCols.size();

    sqlString += getCols[0];
    for (unsigned iGet = 1; iGet < nGet; iGet++) {
      sqlString += ",";
      sqlString += getCols[iGet];
    }
    sqlString +=  " FROM " + tableName +  " ";
    if (where.size() >  0) {
      sqlString += where + " ";
    }
    if (nOrder > 0 ) {
      sqlString += " ORDER BY " + orderCols[0]; 
      for (unsigned iOrder = 1; iOrder < nOrder; iOrder++) {
        sqlString += ",";
        sqlString += orderCols[iOrder];
      }
    }
    if (rowLimit > 0) {
      // SQL format is LIMIT offset,limit      or
      //               LIMIT limit             or
      //               LIMIT limit   OFFSET offset  [don't use this one]
      sqlString += " LIMIT ";
      std::string limitStr;
      if (rowOffset > 0) {
        facilities::Util::itoa(rowOffset, limitStr);
        sqlString += limitStr + ",";
      }
      limitStr.clear();
      facilities::Util::itoa(rowLimit, limitStr);
      sqlString += limitStr;
    }
    (*m_out) << std::endl << "# About to issue SELECT:" << std::endl;
    (*m_out) << sqlString << std::endl;
    m_out->flush();
    
    int mysqlRet = realQueryRetry(sqlString);
    if (mysqlRet) {
      unsigned errcode;
      std::string msg = 
        "rdbModel::MysqlConnection::select. ";
      msg += getMysqlError(m_mysql, &errcode);
      (*m_err) << std::endl << msg << std::endl;
      m_err->flush();
      throw RdbException(msg, mysqlRet);
      return 0;
    }

    MYSQL_RES *myres = mysql_store_result(m_mysql);
    MysqlResults* results = new MysqlResults(myres);
    return results;
  }

  ResultHandle* MysqlConnection::select(const std::string& tableName,
                                        const StringVector& getCols,
                                        const StringVector& orderCols,
                                        SELECTOPTIONS flags,
                                        const std::string& where,
                                        int   rowLimit) {
    std::string sqlString = "SELECT ";
    unsigned nGet = getCols.size();
    unsigned nOrder = orderCols.size();

    sqlString += getCols[0];
    for (unsigned iGet = 1; iGet < nGet; iGet++) {
      sqlString += ",";
      sqlString += getCols[iGet];
    }
    sqlString +=  " FROM " + tableName +  " ";
    if (where.size() >  0) {
      sqlString += where + " ";
    }
    if (nOrder > 0 ) {
      sqlString += " ORDER BY " + orderCols[0]; 
      for (unsigned iOrder = 1; iOrder < nOrder; iOrder++) {
        sqlString += ",";
        sqlString += orderCols[iOrder];
      }
      if (flags & SELECTdesc) {
        sqlString += " DESC ";
      }
    }
    if (rowLimit > 0) {
      sqlString += " LIMIT ";
      std::string limitStr;
      limitStr.clear();
      facilities::Util::itoa(rowLimit, limitStr);
      sqlString += limitStr;
    }

    if (flags & SELECTforUpdate) {
      sqlString += " FOR UPDATE ";
    }
    else if (flags & SELECTshareLock) {
      sqlString += " LOCK IN SHARE MODE ";
    }

    (*m_out) << std::endl << "# About to issue SELECT:" << std::endl;
    (*m_out) << sqlString << std::endl;
    m_out->flush();
    
    int mysqlRet;
    if (flags & (SELECTforUpdate | SELECTshareLock)) { // no retry
      mysqlRet = realQuery(m_mysql, sqlString);
    }
    else mysqlRet = realQueryRetry(sqlString);
    if (mysqlRet) {
      unsigned errcode;
      std::string msg = 
        "rdbModel::MysqlConnection::select. ";
      msg += getMysqlError(m_mysql, &errcode);
      (*m_err) << std::endl << msg << std::endl;
      m_err->flush();
      throw RdbException(msg, mysqlRet);
      return 0;
    }

    MYSQL_RES *myres = mysql_store_result(m_mysql);
    MysqlResults* results = new MysqlResults(myres);
    return results;

  }



  ResultHandle* MysqlConnection::dbRequest(const std::string& request) {

    (*m_out) << std::endl << "# About to issue SQL request:" << std::endl;
    (*m_out) << request << std::endl;
    m_out->flush();
    
    // no retries for totally arbitrary request
    int mysqlRet = realQuery(m_mysql, request);
    if (mysqlRet) {
      unsigned errcode;
      std::string msg = 
        "rdbModel::MysqlConnection::dbRequest. ";
      msg += getMysqlError(m_mysql, &errcode);
      (*m_err) << std::endl << msg << std::endl;
      m_err->flush();
      throw RdbException(msg, mysqlRet);
      return 0;
    }

    MYSQL_RES *myres = mysql_store_result(m_mysql);
    if (!myres) {
      // Was it supposed to return data?
      if (mysql_field_count(m_mysql) == 0) { // no data expected
        return 0;
      }
      else {
        std::string msg =
          "rdbModel::MysqlConnection::dbRequest: expected data; none returned";
        (*m_err) << std::endl << msg << std::endl;
        m_err->flush();
        throw RdbException(msg);
        return 0;
      }
    }
    return new MysqlResults(myres);
  }

  bool MysqlConnection::compileAssertion(const Assertion* a, 
                                         std::string& sqlString) const {
    if (!m_compileInit) {
      compileInit();
      m_compileInit = true;
    }
    try {
      return compileOperator(a->getOperator(), sqlString);
    }
    catch (RdbException ex) {
      (*m_err) << std::endl << ex.getMsg() << std::endl;
      m_err->flush();
      return false;
    }
  }

  std::string opSymbols[OPTYPElast];

  void MysqlConnection::compileInit() {
    opSymbols[OPTYPEor] = " OR ";
    opSymbols[OPTYPEand] = " AND ";
    opSymbols[OPTYPEnot] = " NOT ";
    opSymbols[OPTYPEexists] = "EXISTS ";
    opSymbols[OPTYPEisNull] = " IS NULL";
    opSymbols[OPTYPEequal] = "=";
    opSymbols[OPTYPEnotEqual] = "<>";
    opSymbols[OPTYPElessThan] = "<";
    opSymbols[OPTYPEgreaterThan] = ">";
    opSymbols[OPTYPElessOrEqual] = "<=";
    opSymbols[OPTYPEgreaterOrEqual] = ">=";
    return;
  }



  /*  
     Need significant changes here to deal with which="toBe" case
     In that case, the output isn't going to be SQL; in fact, it's
     not clear what it should be, exactly!
  */

  /** Result is appended to caller-supplied string
   Convention is to use "   "  around literal values
   Note no verification is done here; that operator is in fact a comparison
   or isNull operator.  This is called internally only and that check will
   have been done before invoking this routine.
  */
  bool MysqlConnection::compileComparison(Assertion::Operator* op, 
                                          std::string& sqlString) {
    if (op->getToBe()) return false;  // can't compile

    OPTYPE opType = op->getOpType();
    if (opType == OPTYPEisNull) {
      sqlString +="(";
      sqlString += op->getCompareArgs()[0];
      sqlString += opSymbols[opType];
      sqlString += ")";
      return true;
    }
    sqlString += "(";

    bool literal0 = (op->getCompareArgTypes()[0] == FIELDTYPElit);
    bool literal1 = (op->getCompareArgTypes()[1] == FIELDTYPElit);

    addArg(literal0, op->getCompareArgs()[0], sqlString);
    sqlString += opSymbols[opType];
    addArg(literal1, op->getCompareArgs()[1], sqlString);
    sqlString += ")";

    return true;
  }

  bool MysqlConnection::compileOperator(Assertion::Operator* op, 
                                        std::string &sqlString) {
    if (op->isCompareOp() ) return compileComparison(op, sqlString);
    if (op->getToBe()) return false;  // can't compile in this case
    bool ret = true;

    const std::vector<Assertion::Operator*>& children = op->getChildren();
    unsigned nChild = children.size();

    // For single-child operators NOT,  exists, operator symbol
    // goes 1st, then operand
    if (nChild <= 1) { // operator goes first
      sqlString += opSymbols[op->getOpType()];

      // more special handling for EXISTS
      if (op->getOpType() == OPTYPEexists) {
        sqlString += "(SELECT * FROM " + op->getTableName();
        if (!nChild) {     // done
          sqlString += ")";
          return ret;
        }
        // else EXISTS child is object of a WHERE clause 
        sqlString += " WHERE(";
      }
      ret = compileOperator(children[0], sqlString);
      if (!ret) {
        std::string msg = 
          "rdbModel::MysqlConnection::compileOperator failed for operator "
          + opSymbols[op->getOpType()];
        throw RdbException(msg);
      }

      // Have an extra closing ")" for EXISTS with WHERE clause
      if (op->getOpType() == OPTYPEexists)       sqlString += ")";

      return ret;
    }

    // Otherwise put operator symbols between adjacent children.

    // First open parentheses
    sqlString += "(";

    std::string symbol = opSymbols[op->getOpType()];

    ret = compileOperator(children[0], sqlString);
    if (!ret) {
      std::string msg = 
        "rdbModel::MysqlConnection::compileOperator failed for operator "
        + symbol;
      throw RdbException(msg);
    }
    for (unsigned int iChild = 1; iChild < nChild; iChild++) {
      sqlString += symbol;
      ret = compileOperator(children[iChild], sqlString);
      if (!ret) {
        std::string msg = 
          "rdbModel::MysqlConnection::compileOperator failed for operator "
          + symbol;
        throw RdbException(msg);
      }
    }
    // Finally close paren.
    sqlString += ")";
    return ret;
  }

  // Satisfy Visitor interface.  For now the only visitor is the
  // one to check whether remote and local db descriptions match or
  // are at least compatible enough to be used.
  Visitor::VisitorState MysqlConnection::visitRdb(Rdb *rdb) {
    
    if (m_matchDbName) {
      if (m_dbName != rdb->getDbName()) {
        m_matchReturn = MATCHfail;
        return Visitor::VDONE;
      }
    }
    
    unsigned int nLocal = rdb->getNTable();

    // Null pointer for 2nd argument means "list all tables"
    
    MYSQL_RES* res = mysql_list_tables(m_mysql, 0);
    if (!res) {
      m_matchReturn = MATCHfail;
      (*m_err) << std::endl << "Match failed.  Could not list db tables" 
                  << std::endl;
      return Visitor::VERRORABORT;
    }
    unsigned int nRemote = mysql_num_rows(res);
    mysql_free_result(res);

    if (nRemote < nLocal) {
      m_matchReturn = MATCHfail;
      (*m_err) << std::endl << "Match failed.  Real db missing tables" 
                  << std::endl;
      m_err->flush();
      return Visitor::VDONE;
    }
    else if (nRemote > nLocal) m_matchReturn = MATCHcompatible;

    // Tell Rdb about this 
    rdb->setConnection(this);

    return Visitor::VCONTINUE;
  }

  Visitor::VisitorState MysqlConnection::visitTable(Table* table) {
    const std::string& tName = table->getName();

    // Result set will have all fields for the table
    if (m_tempRes) {
      mysql_free_result(m_tempRes);
      m_tempRes = 0;
    }
    m_primColName.clear();

    std::string query = "SHOW COLUMNS FROM " + tName;

    (*m_out) << std::endl << "# About to issue SHOW COLUMNS request :" 
             << std::endl;
    (*m_out) << query << std::endl;
    m_out->flush();
    
    int ret = realQueryRetry(query);
    if (ret) {
      unsigned errcode;
      m_matchReturn = MATCHfail;
      (*m_err) << std::endl << "SHOW COLUMNS request failed " << std::endl;
      (*m_err) << getMysqlError(m_mysql, &errcode) << std::endl;
      m_err->flush();
      return Visitor::VERRORABORT;
    }
      
    m_tempRes = mysql_store_result(m_mysql);
    if (!m_tempRes) {
      (*m_err) << std::endl << "Could not access SHOW COLUMNS results" 
                  << std::endl;
      m_err->flush();
      m_matchReturn = MATCHfail;
      return Visitor::VERRORABORT;
    }
    // Result set is a table with fields "Field"(the name) "Type" "Null"(yes
    // or no) "Key" "Default", "Extra"  
    // Make it easier for accept(Column* ) to find relevant information
    unsigned int nRow = mysql_num_rows(m_tempRes);
    m_colIx.clear();
    for (unsigned iRow = 0; iRow < nRow; iRow++) {
      MYSQL_ROW colDescrip = mysql_fetch_row(m_tempRes);
      std::string name = std::string(colDescrip[0]);
      m_colIx[name] = iRow;
    }
    return Visitor::VCONTINUE;

  }

  Visitor::VisitorState MysqlConnection::visitColumn(Column* col) {
    std::string myName = col->getName();
    if (m_colIx.find(myName) == m_colIx.end()) {
      (*m_err) << "Could not find column " << myName << " in MySQL db "
                  << std::endl;
      m_err->flush();
      m_matchReturn = MATCHfail;
      return Visitor::VERRORABORT;
    }
    unsigned int ix = m_colIx[myName];
    mysql_data_seek(m_tempRes, ix);
    MYSQL_ROW colDescrip = mysql_fetch_row(m_tempRes);

    // Type
    std::string sqlDtype = std::string(colDescrip[1]);
    Datatype* dtype = col->getDatatype();
    if (!checkDType(dtype, sqlDtype)) {
      m_matchReturn = MATCHfail;
      (*m_err) << "Failed dtype match of col " << myName << std::endl;
      m_err->flush();
      return Visitor::VERRORABORT;
    }
    
    // Null
    bool nullable = (std::string(colDescrip[2]) == std::string("YES"));
    if (nullable != col->nullAllowed()) {
      m_matchReturn = MATCHfail;
      (*m_err) << "Failed null/not null match of col " << myName 
                  << std::endl;
      m_err->flush();
      return Visitor::VERRORABORT;
    }
    // Key (PRI for primary, MUL if first in a multiple-field key
    // or if nullable unique key, UNI if not nullable, unique.  
    // Save primary key and unique key info, if any
    if (std::string(colDescrip[3]) == std::string("PRI")) {
      m_primColName = myName;
      col->m_isUniqueKey = true;
    } else if  (std::string(colDescrip[3]) == std::string("UNI")) {
      col->m_isUniqueKey = true;
    }

    // Field 4 is default
    // Extra (may say auto_increment)
    bool autoInc = 
      (std::string(colDescrip[5]) == std::string("auto_increment"));
    if (autoInc != col->isAutoIncrement()) {
      m_matchReturn = MATCHfail;
      (*m_err) << "Failed isAutoIncrement match of col " 
               << myName << std::endl;
      m_err->flush();
      return Visitor::VERRORABORT;
    }
    return Visitor::VCONTINUE;
  }

  bool MysqlConnection::checkDType(Datatype* dtype, 
                                   const std::string& sqlType) {
    std::string base;
    int sqlSize;
    if (dtype->getType() != Datatype::TYPEenum) {
      sqlSize = extractSize(sqlType);
    }

    // Cases  char, varchar, enum and datetime are handled entirely within
    // the switch statement, but most do the bulk of the work in
    // common, after the switch.
    switch (dtype->getType()) {
    case Datatype::TYPEenum: {
      base = "enum";
      if (sqlType.find(base) != 0) {
        m_matchReturn = MATCHfail;
        break;        //        return false;
      }
      Enum* ourEnum = dtype->getEnum();
      // Finally compare local list of choices to those listed in sqlType
      // Local list is a vector; in sqlType they're quoted, comma separated
      return compareEnumList(ourEnum->getChoices(), sqlType);
    }
    case Datatype::TYPEvarchar: {
      base = "varchar";
      if (sqlType.find(base) != 0) {
        m_matchReturn = MATCHfail;
        break;  //        return false;
      }
      // size in db must be at least as large as size in Col.
      if (sqlSize < dtype->getOutputSize()) {
        m_matchReturn = MATCHfail;
        break; //        return false;
      }
      else if (sqlSize > dtype->getOutputSize()) {
        m_matchReturn = MATCHcompatible;
      }
      return true;
    }
    case Datatype::TYPEchar: {
      base = "char";
      if (sqlType.find(base) != 0) {
        m_matchReturn = MATCHfail;
        break;  //        return false;
      }
      //  For char datatype unspecified size is equivalent to size=1
      if (!sqlSize) sqlSize = 1;
      // size in db must be at least as large as size in Col.
      if (sqlSize < dtype->getOutputSize()) {
        m_matchReturn = MATCHfail;
        break; // return false;
      }
      else if (sqlSize > dtype->getOutputSize()) {
        m_matchReturn = MATCHcompatible;
      }
      return true;
    }
      //

    case Datatype::TYPEblob: {
      base = "blob";
      break;
    }
    case Datatype::TYPEtinyblob: {
      base = "tinyblob";
      break;
    }
    case Datatype::TYPEmediumblob: {
      base = "mediumblob";
      break;
    }
    case Datatype::TYPElongblob: {
      base = "longblob";
      break;
    }
    case Datatype::TYPEtext: {
      base = "text";
      break;
    }
    case Datatype::TYPEtinytext: {
      base = "tinytext";
      break;
    }
    case Datatype::TYPEmediumtext: {
      base = "mediumtext";
      break;
    }
    case Datatype::TYPElongtext: {
      base = "longtext";
      break;
    }

      //
    case Datatype::TYPEdatetime: {
      base = "datetime";
      if (sqlType != "datetime") {
        m_matchReturn = MATCHfail;
        break;  // return false;
      }
      return true;
    }

    case Datatype::TYPEtimestamp: {
      base = "timestamp";
      break;
    }
    case Datatype::TYPEint: 
    case Datatype::TYPEintUnsigned:      {
      base = "int";
      break;
    }
    case Datatype::TYPEmediumint: 
    case Datatype::TYPEmediumintUnsigned:  {
      base = "mediumint";
      break;
    }
    case Datatype::TYPEsmallint: 
    case Datatype::TYPEsmallintUnsigned:   {
      base = "smallint";
      break;
    }
    case Datatype::TYPEtinyint: 
    case Datatype::TYPEtinyintUnsigned:   {
      base = "tinyint";
      break;
    }
    case Datatype::TYPEfloat: {
      base = "float";
      break;
    }

    case Datatype::TYPEdouble: {
      base = "double";
      break;
    }
    default: {  // Indicates bad xml file input.  Applications
                //should have exited already
      base = "unknown";
      m_matchReturn = MATCHfail;
      break; // return false;
    }
    }     // end switch
    if (m_matchReturn == MATCHfail) {
      (*m_err) << std::endl << "Datatype match for base " 
                  << base <<  "failed " << std::endl;
      m_err->flush();
      return false;
    }
    if (sqlType.find(base) != 0) {
      m_matchReturn = MATCHfail;
      (*m_err) << std::endl << "Datatype match failed for base = " 
                 << base << std::endl;
      m_err->flush();
      return false;
    }
    // Also need to check for unsigned here
    unsigned loc = sqlType.find("unsigned");
    if ((loc < sqlType.size()) != (dtype->isUnsigned())) {
      (*m_err) << std::endl << "Datatype match failed signed/unsigned " 
                  << std::endl;
      m_err->flush();
      m_matchReturn = MATCHfail;
      return false;
    }

    // Now check size.  It's only for display, so mismatch is not failure
    if (sqlSize != dtype->getOutputSize()) {
      m_matchReturn = MATCHcompatible;
    }


    return true;
  }

  /**
     Retry up to retry count if query fails with retriable error
  */
  int MysqlConnection::realQueryRetry(const std::string& qstring) {
    unsigned remain = m_maxRetry;
    int mysqlRet = realQuery(m_mysql, qstring);
    while (remain) {
      --remain;
      switch (mysqlRet) {
        // retriable errors
      case CR_SERVER_GONE_ERROR:
      case CR_SERVER_LOST:
      case CR_UNKNOWN_ERROR: {
        close();        // close old connection
        // calculate sleep time, sleep
        // rand returns a value in the range 0 - MAX_RAND.  The latter
        // is OS-dependent, but must be at least 32767 = 0x7fff
        unsigned maxRnd = 0xfff;
        unsigned wait = m_avgWait;
        if (wait < maxRnd/2) wait += maxRnd/2;

        unsigned rnd = rand() & maxRnd;  // just use last 12 bits
        wait += (rnd - maxRnd/2);
        facilities::Util::gsleep(wait);
        // open new connection, retry
        const char* host =0;
        const char* user = 0;
        const char* pw = 0;
        if (m_host.size()) host = m_host.c_str();
        if (m_user.size()) user = m_user.c_str();
        if (m_pw.size()) pw = m_pw.c_str();
        bool ok = open(host, m_port, user, pw, m_dbName.c_str());
        if (!ok) continue;  // on to next retry

        mysqlRet = realQuery(m_mysql, qstring);
        break;
      }
        // For other errors or success just return
      default:
        return mysqlRet;
      }
    }
    return mysqlRet;

  }


  Visitor::VisitorState MysqlConnection::visitIndex(Index* ) {
    return Visitor::VCONTINUE;
    // might put something real here later
  }

  Visitor::VisitorState MysqlConnection::visitAssertion(Assertion*) {
    return Visitor::VCONTINUE;
  }

  Visitor::VisitorState MysqlConnection::visitInsertNew(InsertNew*) {
    return Visitor::VCONTINUE;
  }

  Visitor::VisitorState MysqlConnection::visitSupersede(Supersede*) {
    return Visitor::VCONTINUE;
  }

  Visitor::VisitorState MysqlConnection::visitQuery(Query*) {
    return Visitor::VCONTINUE;
  }

  Visitor::VisitorState MysqlConnection::visitSet(Set*) {
    return Visitor::VCONTINUE;
  }

  Visitor::VisitorState MysqlConnection::visitInterRow(InterRow*) {
    return Visitor::VCONTINUE;
  }




} // end namespace rdbModel
