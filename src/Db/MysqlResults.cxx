// $Header:  $
#ifdef  WIN32
#include <windows.h>
#endif

#include "rdbModel/Db/MysqlResults.h"
#include "mysql.h"

namespace rdbmodel {

  MysqlResults::MysqlResults() {

  }

  MysqlResults::~MysqlResults() {
  
  }

  MysqlResults::getNRows() const {

  }

  bool MysqlResults::getRowString(std::string& row, unsigned int iRow=0) {

  }

  bool     bool getRowStrings(std::vector<std::string>& rows, 
                              unsigned int iRow=0, unsigned int maxRow=0, 
                              bool clear=true) const {
  }

}
