// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/rdbModel/Tables/Table.h,v 1.3 2004/03/06 01:13:10 jrb Exp $
#ifndef RDBMODEL_RESULTHANDLE_H
#define RDBMODEL_RESULTHANDLE_H
#include <vector>
#include <string>


namespace rdbModel{

  /** 
      Pure virtual class representing results of a query.  Each concrete 
      implementation of Connection will have an associated concrete
      implementation of ResultHandle
  */
  class ResultHandle {

    ResultHandle() {};
    virtual ~ResultHandle() {};

    /// Return number of rows in results
    int getNRows() const = 0;

    /// Return specified row in results as a string
    bool getRowString(std::string& row, unsigned int iRow=0) const = 0;

    /**  Return vector of rows
         @param rows     to hold returned data
         @param iRow     starting row
         @param maxRow   maximum number of rows requested.  0 means "all"
         @param clear    if true, clear @a rows before storing new data
         @return         status
    */
    bool getRowStrings(std::vector<std::string>& rows, unsigned int iRow=0,
                       unsigned int maxRow=0, bool clear=true) const = 0;

  };
}
#endif
