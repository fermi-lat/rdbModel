// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/Db/MysqlConnection.cxx,v 1.1 2004/03/24 02:06:13 jrb Exp $
#ifdef  WIN32
#include <windows.h>
#endif

#include "rdbModel/Db/MysqlConnection.h"
#include "rdbModel/Table/Assertion.h"
#include "rdbModel/Db/MysqlResults.h"
#include "rdbModel/RdbException.h"
#include "facilities/Util.h"

#include "mysql.h"

namespace {
  using rdbModel::Assertion;
  std::string[OPTYPElast + 1] opSymbols;

  void compileInit() {
    opSymbols[OPTYPEor] = " OR ";
    opSymbols[OPTYPEand] = " AND ";
    opSymbols[OPTYPEexists] "ANY ";
    opSymbols[OPTYPEforAll] = "ALL ";
    opSymbols[OPTYPEnot] = " NOT ";
    opSymbols[OPTYPEisNull] = " IS NULL";
    opSymbols[OPTYPEequal] = "=";
    opSymbols[OPTYPEnotEqual] = "<>";
    opSymbols[OPTYPElessThan] = "<";
    opSymbols[OPTYPEgreaterThan] = ">";
    opSymbols[OPTYPElessOrEqual] = "<=";
    opSymbols[OPTYPEgreaterOrEqual] = ">=";
    return;
  }
  void addArg(bool literal, const std::string arg, std::string& sqlString) {
    if (literal) sqlString += '"';
    sqlString += arg;
    if (literal) sqlString += '"';
    return;
  }
    
  /** Result is appended to caller-supplied string
   Convention is to use "   "  around literal values
   Note no verification is done here; that operator is in fact a comparison
   or isNull operator.  This is called internally only and that check will
   have been done before invoking this routine.
  */
  bool compileComparison(Assertion::Operator* op, std::string& sqlString) {
    Assertion::OPTYPE opType = op->getOpType();
    if (op == Assertion::OPTYPEisNull) {
      sqlString +="(";
      sqlString += op->getCompareArgs()[0];
      sqlString += opSymbols[opType];
      sqlString += ")";
      return true;
    }
    sqlString += "(";
    addArg(op->getLiteralness()[0], op->getCompareArgs()[0], sqlString);
    sqlString += opSymbols[opType];
    addArg(op->getLiteralness()[1], op->getCompareArgs()[1], sqlString);
    sqlString += ")";

    return true;
  }

  bool compileOperator(Assertion::Operator* op, std::string &sqlString) {
    if (op->isComparison() ) return compileComparison(op, sqlString);
    
    bool ret;

    std::vector<Assertion::Operator*>& children = getChildren();
    unsigned nChild = children.size();

    sqlString += "(";
    // For single-child operators NOT, forAll, thereExists, operator symbol
    // goes 1st, then operand
    if (nChild == 1) { // operator goes first
      sqlString += opSymbols[op->getOpType()];
      ret = compileOperator(children[0], sqlString);
      if (!ret) {
        std::string msg = 
          "rdbModel::MysqlConnection::compileOperator failed for operator "
          + opSymbols[op->getOpType()];
        throw RdbException(msg);
      }
      sqlString += ")";
      return ret;
    }
    // Otherwise put operator symbols between adjacent children.
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
    return ret;
  }
}
    
namespace rdbModel {

  MysqlConnection::MysqlConnection() : m_mysql(0), m_connected(0),
                                       m_compileInit(false)        {
    m_mysql = new MYSQL;

  }

  MysqlConnection::~MysqlConnection() {
    mysql_close(m_mysql);
    delete m_mysql;
    m_mysql = 0;
    m_connected = false;
    return;
  }

  bool MysqlConnection::open(const std::string& host, 
                             const std::string& user,
                             const std::string& password,
                             const std::string& dbName
                             unsigned int       port) {

    mysql_init(m_mysql);
    MYSQL *connected = mysql_real_connect(m_mysql, host.c_str(), user.c_str(),
                                          password.c_str(), dbName.c_str(),
                                          port, NULL, 0);

    if (connected != 0) {  // Everything is fine.  Put out an info message
      std::cout << "Successfully connected to MySQL host" << 
        host << std::endl;
      m_connected = true;
    }
    else {
      std::cerr <<  "Failed to connect to MySQL host " << host <<
        "with error " << mysql_error(m_mysql) << std::endl;
      m_connected = false;
    }
    return m_connected;
  }

  bool MysqlConnection::insertRow(const std::string& tableName, 
                                  const StringVector& colNames, 
                                  const StringVector& values,
                                  int* auto_value) {
    std::string ins;
    if (auto_value) *auto_value = 0;

    // check that sizes of vectors match
    unsigned  nCol = colNames.size()    
    if (!nCol || (nCol != values.size()  ) ) {
      std::cerr << " MysqlConnection::insertRow: vector lengths incompatible"
                << std::endl;
      return false;
    }

    // caller should already have checked for validity and should
    // have supplied all necessary columns

    ins += "insert into " + tableName;
    ins += "set " + colNames[0] + "='" + values[0] + "' ";
    for (unsigned iCol = 1; iCol < nCol; iCol++) {
      ins += ",set " + colNames[iCol] + "='" + values[iCol] + "' ";
    }

    int mysqlRet = mysql_query(m_mysql, ins.c_str());

    if (mysqlRet) {
      std::cerr << "MySQL error during INSERT, code " << mysqlRet << std::endl;
      false;
    }
    if (auto_value) {
      *auto_value = mysql_insert_id(m_mysql);
    }
    return true;
  }


  unsigned int MysqlConnection::update(const std::string& tableName, 
                                       const StringVector& colNames, 
                                       const StringVector& values,
                                       const Assertion* where=0) {

    unsigned int nCol = colNames.size();
    if (nCol != values.size()) {
      std::cerr << "rdbModel::mysqlConnection::update: ";
      std::cerr << "Incompatible vector arguments " << std::endl;
      return 0;
    }
    std::string sqlString = "UPDATE " + tableName + " SET ";
    sqlString += colNames[0] + "'" + values[0] + "'";
    for (unsigned int iCol = 1; iCol < nCol, iCol++) {
      sqlString += "," + colNames[iCol] + "'" + values[iCol] + "'";
    }
    if (where) {
      sqlString += " WHERE ";
      bool ret = compileAssertion(where, sqlString);
      if (!ret) return 0;
    }
    int mysqlRet = mysql_query(m_mysql, sqlString.c_str());

    if (mysqlRet) {
      std::cerr << "rdbModel::MysqlConnection::update: ";
      std::cerr << "MySQL error during UPDATE, code " <<mysqlRret << std::endl;
      return 0;
    }
    my_ulonglong nModLong = mysql_affected_rows(m_mysql);
    // Not much chance that we'll change more rows than will fit in just long
    unsigned nMod = nModLong;
    return nMod;


  }

  ResultHandle* MysqlConnection::select(const std::string& tableName,
                                        const StringVector& getCols,
                                        const StringVector& orderCols,
                                        const Assertion* where=0,
                                        const std::string& orderBy="",
                                        int   rowLimit=0) {
    std::string sqlString = "SELECT FROM " + tableName + " ";
    unsigned nGet = getCols.size();
    unsigned nOrder = orderCols.size();

    sqlString += getCols[0];
    for (unsigned iGet = 1; iGet < nGet; iGet++) {
      sqlString += ",";
      sqlString += getCols[iGet];
    }
    if (where != 0) {
      sqlString += " WHERE ";
      bool ret = compileAssertion(where, sqlString);
      if (!ret) return 0;
    }
    if (nOrder > 0 ) {
      sqlString += " ORDER BY " + orderCols[0]; 
      for (unsigned iOrder = 1; iOrder < nOrder; iOrder++) {
        sqlString += ",";
        sqlString += orderCols[iOrder];
      }
    }
    if (rowLimit > 0) {
      std::string limitStr;
      facilities::Util::itoa(rowLimit, limitStr);
      sqlString += " LIMIT " + limitStr;
    }
    int mysqlRet = mysql_query(m_mysql, sqlString.c_str());
    if (mysqlRet) {
      std::string msg = 
        "rdbModel::MysqlConnection::select: mysql_query error, code ";
      std::string codeString;
      facilities::Util::itoa(code, codeString);
      msg += codeString;
      throw RdbException(msg);
      return 0;
    }

    MYSQL_RES *myres = mysql_store_result(m_mysql);
    MysqlResults* results = new MysqlResults(myres);
    return results;
  }

  bool MysqlConnection::compileAssertion(const Asssertion* a, 
                                         std::string& sqlString) {
    if (!m_compileInit) {
      compileInit();
      m_compileInit = true;
    }
    return compileOperator(a->getOperator(), sqlString);
  }

} // end namespace rdbModel
