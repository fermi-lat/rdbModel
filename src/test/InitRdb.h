// $Header:   $
// Public include for class to initialize rdb-type database

namespace rdbModel {
  class Rdb;

  class InitRdb {
  public:
    InitRdb(bool dbg = false) : m_rdb(0), m_dbg(dbg) { }
    ~InitRdb();
    int  buildModel(std::string& dfile);

    /// Connect and check that db and description are compatible
    int dbconnect(const char* host, int port, const std::string& dbname, 
                  const std::string& group);
  private:
    rdbModel::Rdb* m_rdb;
    bool           m_dbg;
  };
}
