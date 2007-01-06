// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/Db/MysqlConnection.cxx,v 1.51 2006/12/16 00:44:08 decot Exp $
#ifdef  WIN32
#include <windows.h>
#endif

#include "rdbModel/Db/Connection.h"
#include "rdbModel/Rdb.h"
#include "rdbModel/Tables/Table.h"
#include "rdbModel/Tables/Assertion.h"
#include "rdbModel/Tables/Column.h"
#include "rdbModel/Tables/Datatype.h"
#include "rdbModel/Db/MysqlResults.h"
#include "rdbModel/RdbException.h"

#include <iostream>
#include <cerrno>
#include "facilities/Util.h"


namespace rdbModel {

  Connection::Connection(std::ostream* out, std::ostream* errOut)
     : m_out(out), m_err(errOut)
  {
    if (m_out == 0) m_out = &std::cout;
    if (m_err == 0) m_err = &std::cerr;
  }
};
