// $Header:  $
#ifdef  WIN32
#include <windows.h>
#endif

#include "rdbModel/Db/MysqlConnection.h"
#include "rdbModel/Table/Assertion.h"
#include "rdbModel/Db/MysqlResults.h"

#include "mysql.h"

namespace rdbModel {

  MysqlConnection::MysqlConnection() : m_mysql(0) {
    m_mysql = new MYSQL;

  }

  MysqlConnection::~MysqlConnection() {

  }

  bool MysqlConnection::open(const std::string& host, 
                             const std::string& user,
                             const std::string& password,
                             const std::string& dbName) {

    mysql_init(m_mysql);
    MYSQL *connected = mysql_real_connect(m_mysql, host.c_str(), user.c_str(),
                                          password.c_str(), dbName.c_str(),
                                          0, NULL, 0);

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
                                  const StringVector& values) {
    std::string ins;
  }


  bool MysqlConnection::updateUnique(const std::string& tableName, 
                                     const StringVector& colNames, 
                                     const StringVector& values,
                                     const std::string& keyValue) {
  }

  int MysqlConnection::update(const std::string& tableName, 
                              const StringVector& colNames, 
                              const StringVector& values,
                              const Assertion* where=0) {
  }

  ResultHandle* MysqlConnection::select(const std::string& tableName,
                                        const StringVector& getCols,
                                        const Assertion* where=0,
                                        const std::string& orderBy="",
                                        int   rowLimit=0) {

  }

  std::string MysqlConnection::compileAssertion(const Asssertion* a) {


  }

} // end namespace rdbModel
