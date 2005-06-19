// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/rdbModel/Tables/Assertion.h,v 1.10 2005/02/15 22:43:06 jrb Exp $
#ifndef RDBMODEL_SUPSERSEDE_H
#define RDBMODEL_SUPERSEDE_H
#include <vector>
#include <string>
#include "rdbModel/Management/Visitor.h"

namespace rdbModel{

  class Set;
  class Table;
  class Assertion;

  /** 
      Supersede operation needs to
          o change one or more column values in row being superseded
          o set values in new, superseding row.  Default is to assume
            values come from the old row; only specify those that don't
  */
  class Supersede {
  public:
    Supersede(Table* table, Assertion* onlyIf=0);

    ~Supersede();

    const std::vector<Set*>&  getSetOld() const {return m_setOld;}
    const std::vector<Set*>&  getSetNew() const {return m_setOld;}

    void addSet(Set* s);

    const Table* getTable() const {return m_myTable;}

    Visitor::VisitorState accept(Visitor* v);

    Assertion* getOnlyIf() {return m_onlyIf;}

    // Might want to get rid of this; should know assertion by constructor
    // time.
    void setOnlyIf(Assertion* onlyIf) {m_onlyIf = onlyIf;}
  private:
    Table* m_myTable;
    //    bool   m_keepOp;
    Assertion* m_onlyIf;   // conditions row must meet to make it supersedable.
    std::vector<Set*> m_setOld;
    std::vector<Set*> m_setNew;

  };
}
#endif

