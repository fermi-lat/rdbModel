// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/rdbModel/Tables/Column.h,v 1.5 2004/03/20 00:48:06 jrb Exp $
#ifndef RDBMODEL_COLUMN_H
#define RDBMODEL_COLUMN_H
#include <vector>
#include <string>
#include "rdbModel/Management/Visitor.h"

namespace rdbModel{

  class Table;
  class Datatype;
  class Enum;

  class XercesBuilder;


  /** 
   * rdbModel representation of a(n SQL-like) table description
   */
  class Column {
    class ColumnSource;      // embedded class, described below

  public:
    Column(Table* myTable=0) : m_myTable(myTable), m_type(0), m_source(0) {};
    ~Column();

    const std::string& getName() const {return m_name; };
    const std::string& getComment() const {return m_comment;};

    Datatype* getDatatype() const {return m_type;};

    /// Return pointer to Enum object associated with this column (if
    /// none, return null pointer).
    Enum* getEnum() const;

    /** See if supplied value meets constraints of column definition
     *   @arg  val    std::string representation of value to be checked
     *   @arg  set    true if value is to be written to column; false
     *                if just being used, e.g. in "where" clause 
     */
    bool okValue(const std::string& val, bool set=true) const;

    /// Return true if otherCol and this have compatible datatypes
    bool isCompatible(const Column* otherCol) const;

    Visitor::VisitorState accept(Visitor* v);
    //    Visitor::VisitorState acceptNotRec(Visitor* v);

  private:
    friend class rdbModel::XercesBuilder; // needs access to add.. methods 

    Table* m_myTable;

    std::string m_name;
    std::string m_comment;
    Datatype*  m_type;
    ColumnSource* m_source;   // maybe make this a nested class?
    //    bool m_primary;         // primary key.  Or do we just do this
    //                               with a Key object?
    ColumnSource* getSource() const {return m_source;};

  private:   
    class ColumnSource {
    public:
    // So far no need for anything other than default constructor
    // and destructor.  Might end up embedding this in Column altogether.

      enum FROM {
        FROMdefault = 1,          // enduser can override default, however
        FROMautoIncrement,
        FROMnow,                  // datatype must be timestamp
        FROMprogram,
        FROMendUser
      };
      
      std::string m_default;
      FROM m_from;
      bool m_null;
    };                     // end nested ColumnSource class

  };

}
#endif
