// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/rdbModel/Tables/Column.h,v 1.12 2004/08/24 00:04:21 jrb Exp $
#ifndef RDBMODEL_COLUMN_H
#define RDBMODEL_COLUMN_H
#include <vector>
#include <string>
#include "rdbModel/Management/Visitor.h"

namespace rdbModel {

  class Table;
  class Datatype;
  class Enum;

  class XercesBuilder;


  /** 
   * rdbModel representation of a(n SQL-like) table description
   */
  class Column {
    //    class ColumnSource;      // embedded class, described below

  public:
    /// Source of value.  Note timestamp with value current time should
    /// be indicated by contents value CONTENTSupdateTime or (if only
    /// upon insert) CONTENTS enterTime
    enum FROM {
      FROMdefault = 1,          // enduser can override default, however
      FROMautoIncrement,
      FROMnow,                  // datatype must be timestamp - deprecated
      FROMprogram,
      FROMendUser
    };

    /// Hints to program in case FROM field is FROMprogram
    enum CONTENTS {
      CONTENTSunspecified = 0,
      CONTENTSserviceName = 1,
      CONTENTSusername    = 2,
      CONTENTSinsertTime   = 3,
      CONTENTSupdateTime = 4
    };

    Column(Table* myTable=0) : m_myTable(myTable), m_type(0), 
                               m_isPrimaryKey(false) {
      m_contents = CONTENTSunspecified;
      m_default = std::string("");};
    // Column(Table* myTable=0) : m_myTable(myTable), m_type(0), m_source(0) {};
    ~Column();


    const std::string& getName() const {return m_name; };
    const std::string& getComment() const {return m_comment;};

    const std::string& getDefault() const {return m_default;}

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

    /// Returns true if column may take on value NULL
    bool nullAllowed() const { return m_null;}
   
    bool stickyInsert() const { return m_stickyInsert;}

    bool isPrimaryKey() const {return m_isPrimaryKey;}

    bool isAutoIncrement() const;  

    FROM getSourceType() const {return m_from;}
    CONTENTS getContentsType() const {return m_contents;}
                               
    Visitor::VisitorState accept(Visitor* v);

  private:
    friend class rdbModel::XercesBuilder; // needs access to add.. methods 

    Table* m_myTable;

    std::string m_name;
    std::string m_comment;
    Datatype*  m_type;
      
    std::string m_default;
    FROM m_from;
    CONTENTS m_contents;

    /// Can this field have the value NULL?
    bool m_null;
    ///  For multi-insert, does this column normally keep same value
    /// for all the inserts?
    bool m_stickyInsert;  
    
    /// Is this column a primary key?
    bool m_isPrimaryKey;

  };

}
#endif
