// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/rdbModel/Rdb.h,v 1.6 2004/07/02 14:21:41 riccardo Exp $
#ifndef RDBMODEL_RDB_H
#define RDBMODEL_RDB_H
#include <vector>
#include <string>
// #include "detModel/Sections/Section.h"

#include "rdbModel/Management/Visitor.h"

namespace rdbModel {
  
  class Table;
  class Column;
  class Index;
  //  class Assertion;
  class XercesBuilder;
  class Connection;

    // Values for database fields may be specified in various ways
  enum FIELDTYPE {
    FIELDTYPElit = 0,  // literal value
    FIELDTYPEold,      // col name, refers to (value of) col in existing row
    FIELDTYPEtoBe,     // col name, refers to (value of) col in proposed row
    FIELDTYPEask,       // supply value at the time of the operation
    FIELDTYPElitDef,   // literal but may be overridden
    FIELDTYPEoldDef,   // comes from existing row, may be overridden
    FIELDTYPEtoBeDef  // comes from proposed row, may be overridden
  };

  /**
   * This is the main container of all the rdb description.
   * The Manager is responsible for its creation and destruction.
   * Provide both a query interface (provide various functions 
   * to look up components by name) and also accept visitors.
   *
   * Adapted from detModel::Gdd class, written by R. Giannitrapani
   * and D. Favretto
   * @author J. Bogart
   */
  class Rdb {    
  public:
    /** This is the destructor; it should be called only by the manager
     * destructor. It starts the destruction of all the objects created
     * by the builder
     */
    virtual ~Rdb();


    unsigned getMajorVersion(){return m_majorVersion;};
    unsigned getMinorVersion(){return m_minorVersion;};

    const std::string& getCVSid(){return m_CVSid;};

    const std::string& getDbName(){return m_dbName;};
    // Not sure we want to expose this:
    // This method gives back a pointer to the tables vector
    //    std::vector <Table*>  getTables(){return m_tables;};  

    Table* getTable(const std::string& name) const;
    Column* getColumn(const std::string& tableName, 
                      const std::string& colName) const;

    Index* getIndex(const std::string& tableName, 
                      const std::string& indexName) const;

    unsigned int getNTable() const {return m_tables.size();}
    // There is no good way to look up assertions; they don't
    // have names.  For now assertions may only refer to a single
    // table, so access will be via Table object.

    // Do we want an unset as well?  Or just call this with arg == 0 ?
    void setConnection(Connection* connection) {m_connection = connection;}


    /// This is the recursive accept for the visitor pattern
    unsigned int  accept(Visitor* v);
    // This is the non recursive accept for the visitor pattern
    // Not yet sure we need it
    //    Visitor::VisitorState acceptNotRec(Visitor* v);

    // This method builds a global volumes map for all the sections 
    //    void buildVolumeMap();
    // This method builds the global constants maps for all the constants
    //void buildConstantsMap();

  private:
    // Must make each concrete implementation of Builder a friend since
    // derived classes don't inherit friendliness.
    friend class rdbModel::XercesBuilder;

    //    void setDTDversion(std::string pdtd){m_DTDversion = pdtd;};
    void setCVSid(std::string pcvs){m_CVSid = pcvs;};

    void addTable(Table* t){m_tables.push_back(t);};

    // Maybe have private table map to support look-up by name?
    // Or maybe don't bother.  The target application will have
    // a very small number of tables, so a linear search is
    // perfectly ok.

    std::vector<Table* > m_tables;  
  
    /// SQL database name (e.g., "calib") 
    std::string m_dbName;
    /// The Schema version from the input xml description
    unsigned m_majorVersion;
    unsigned m_minorVersion;
    //    std::string m_DTDversion;
    /// The CVSid from the input xml description
    std::string m_CVSid;
    Connection* m_connection;

  };
}

#endif //RDB_H

