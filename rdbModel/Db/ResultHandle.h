// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/rdbModel/Db/ResultHandle.h,v 1.1 2004/03/24 02:01:31 jrb Exp $
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
  public:
    ResultHandle() {};
    virtual ~ResultHandle() {};

    /// Return number of rows in results
    virtual unsigned int getNRows() const = 0;

    /**  
         Get array of field values for ith row of result set
    */
    virtual bool getRow(std::vector<std::string>& fields, unsigned int i = 0,
                        bool clear = true) =0;

    /*
    // Return specified row in results as a string
    virtual bool getRowString(std::string& row, unsigned int iRow=0) const = 0;

      Return vector of rows
         @param rows     to hold returned data
         @param iRow     starting row
         @param maxRow   maximum number of rows requested.  0 means "all"
         @param clear    if true, clear @a rows before storing new data
         @return         status
    
    virtual bool getRowStrings(std::vector<std::string>& rows, 
                               unsigned int iRow=0, unsigned int maxRow=0, 
                               bool clear=true) const = 0;
    */
  };
}
#endif
