// $Header:  $
#ifndef RDBMODEL_COLUMN_H
#define RDBMODEL_COLUMN_H
#include <vector>
#include <string>
#include "rdbModel/Management/Visitor.h"

namespace rdbModel{

  class Table;
  class Datatype;

  class XercesBuilder;


  /** 
   * rdbModel representation of a(n SQL-like) table description
   */
  class Column {
    class ColumnSource;      // embedded class, described below

  public:
    Column() : m_type(0), m_source(0) /* , m_primary(false)*/ {};
    ~Column();

    Datatype* getDatatype() const;
    ColumnSource* getSource() const;
    const std::string& getName() const {return m_name; };
    const std::string& getComment() const {return m_comment;};

    Visitor::VisitorState accept(Visitor* v);
    Visitor::VisitorState acceptNotRec(Visitor* v);

  private:
    friend XercesBuilder; // needs access to add.. methods below

    Table* m_myTable

    std::string m_name;
    std::string m_comment;
    Datatype*  m_type;
    ColumnSource* m_source;   // maybe make this a nested class?
    //    bool m_primary;         // primary key.  Or do we just do this
    //                               with a Key object?
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
