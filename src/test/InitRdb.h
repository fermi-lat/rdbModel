// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/test/InitRdb.h,v 1.1 2006/10/11 00:17:32 jrb Exp $
// Public include for class to initialize rdb-type database
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMElement.hpp>

namespace rdbModel {
  class Rdb;
  class Builder;

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
    Rdb* m_rdb;
    Builder* m_build;
    bool     m_dbg;
    XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* m_doc;

  };
}
