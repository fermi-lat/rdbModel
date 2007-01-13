// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/test/InitRdb.h,v 1.3 2006/10/12 22:44:49 jrb Exp $
// Public include for class to initialize rdb-type database
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include "rdbModel/Tables/Column.h"

namespace rdbModel {
  class Rdb;
  class Builder;
  class Table;

  class InitRdb {
  public:
    InitRdb(bool dbg = false) : m_rdb(0), m_build(0), m_dbg(dbg), m_doc(0) { }
    ~InitRdb();
    int  buildModel(std::string& dfile);

    /// Connect and check that db and description are compatible
    int dbconnect(const char* host, int port, const std::string& dbname, 
                  const std::string& group);

    /// Parse init file and do inserts as it directs 
    int init(std::string& ifile);
  private:
    int handleTable(XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* tbl);

    /// If all goes well, handleRow does an insert
    int handleRow(XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* row, 
                  Table* tbl, const std::vector<FieldVal>& forAllFields,
                  unsigned* newKey=0);
    int handleRef(XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* refElt,
                  std::string& col, std::string& val);
    Rdb* m_rdb;
    Builder* m_build;
    bool     m_dbg;
    /// If m_add is true, keep going after certain kinds of insert failures
    bool     m_add;   
    XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* m_doc;

  };
}
