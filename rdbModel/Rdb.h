// $Header:  $
#ifndef RDBMODEL_RDB_H
#define RDBMODEL_RDB_H
#include <vector>
#include <string>
// #include "detModel/Sections/Section.h"

namespace rdbModel{
  

  class Table;
  class Column;
  class Index;
  //  class Assertion;
  class Visitor;
  class XercesBuilder;

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

    /// This method gives back the DTDversion
    std::string getDTDversion(){return DTDversion;};
    /// This method gives back the CVSid  
    std::string getCVSid(){return CVSid;};
    /// This method sets the DTDversion  
    void setDTDversion(std::string pdtd){DTDversion = pdtd;};
    /// This method sets the CVSid  
    void setCVSid(std::string pcvs){CVSid = pcvs;};

    // Not sure we want to expose this:
    // This method gives back a pointer to the tables vector
    //    std::vector <Table*>  getTables(){return m_tables;};  

    Table* getTable(const std::string& name) const;
    Column* getColumn(const std::string& tableName, 
                      const std::string colName&) const;

    Index* getIndex(const std::string& tableName, 
                      const std::string indexName&) const;

    // There is no good way to look up assertions; they don't
    // have names.  For now assertions may only refer to a single
    // table, so access will be via Table object.


    /// This is the recursive accept for the visitor pattern
    void accept(Visitor* v);
    /// This is the non recursive accept for the visitor pattern
    void acceptNotRec(Visitor* v);

    // This method builds a global volumes map for all the sections 
    //    void buildVolumeMap();
    // This method builds the global constants maps for all the constants
    //void buildConstantsMap();

  private:
    // Must make each concrete implementation of Builder a friend since
    // derived classes don't inherit friendliness.
    friend XercesBuilder;

    void addTable(Table* t){tables.push_back(t);};

    // Maybe have private table map to support look-up by name?
    // Or maybe don't bother.  The target application will have
    // a very small number of tables, so a linear search is
    // perfectly ok.

    std::vector<Table* > m_tables;  
  
    /// SQL database name (e.g., "calib")
    std::string m_dbsname;
    /// The DTDversion from the input xml description
    std::string m_DTDversion;
    /// The CVSid from the input xml description
    std::string m_CVSid;
  };
}

#endif //RDB_H

