// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/rdbModel/Management/Visitor.h,v 1.4 2004/04/02 03:03:30 jrb Exp $
#ifndef RDBMODEL_VISITOR_H
#define RDBMODEL_VISITOR_H
#include <string>

namespace rdbModel {

  class Rdb;
  class Table;
  class Column;
  class Index;
  class Assertion;

  /**
   * This abstract class is the base for the visitors hierarchy; its
   * main purpouse is to provide an abstract interface common to all the
   * different kind of visitor; in this way the manager can hold a 
   * list of visitors that the client can start.
   * 
   * One likly visitor would be something which instantiates a suitable
   * representation, e.g. in a MySQL database, of the tables described
   * 
   * @author J. Bogart,
   * but owing heavily to Visitor class in detModel; thanks to  
   * D.Favretto and R.Giannitrapani 
   */
  class Visitor {
  public:
    enum  VisitorState {
      VCONTINUE = 0,
      VDONE,         // visitor has had its fill of visiting
      VBRANCHDONE,   // don't descend further; go to next sibling, if any
      VERROR,        // error during visit; perhaps can continue, though
      VERRORABORT    // error during visit; can't handle more
    };

  public:
    /// This method says if the visitor is recursive or not
    //    bool getRecursive(){return recursive;}
    /// This method sets if the visitor is recursive or not
    //    void setRecursive(bool prec){recursive = prec;};
    
    virtual VisitorState visitRdb(Rdb*) = 0;
    virtual VisitorState visitTable(Table*) = 0;
    virtual VisitorState visitColumn(Column*) = 0;
    virtual VisitorState visitIndex(Index*) = 0;
    virtual VisitorState visitAssertion(Assertion*) = 0;
    
  protected:
    /// If this is true, the visitor is recursive, otherwise it is not.
    //    bool recursive;
  };
}
#endif // RDBMODEL_VISITOR_H
