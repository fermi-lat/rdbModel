// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/rdbModel/Management/XercesBuilder.h,v 1.5 2004/11/11 21:26:48 jrb Exp $
#ifndef RDBMODEL_XERCESBUILDER_H
#define RDBMODEL_XERCESBUILDER_H

#include "rdbModel/Management/Builder.h"
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include "rdbModel/Tables/Column.h"
#include "rdbModel/Tables/Assertion.h"
namespace rdbModel{

  class Table;    // single rdbms table
  //  class Column;   // column description
  //  class Column::ColumnSource;
  class Index;    // index/key (may be primary or not)
  //  class Assertion; // represents logic in some sort of validity check
  //  class Assertion::Operator;
  class Datatype;

  //  class DMDocClient;
  /**
   * This class is a concrete Builder that use the Xerces parser and the
   * DOM functionalities to parse the xml file and build the generic model
   * hierarchy
   * @author D.Favretto & R.Giannitrapani
   */
  class XercesBuilder : public Builder {
  public:

    /**
     *  The constructor 
     */
    XercesBuilder();


    virtual ~XercesBuilder() {};
  
    /**  Invoke xml parser to produce DOM (in-memory) representation
     */
    virtual unsigned int parseInput(const std::string& inputPath);

    /**
     * This method implements the virtal method of Builder
     */
    virtual int buildRdb();

  private:
    /**
     * This method builds an individual Table object from its xml description
     * and returns a pointer to it
     */
    Table* buildTable(XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* e);

    /**
     *  build a Column from its xml description and return a pointer to it
     */
    Column* buildColumn(XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* e,Table* t);

    /**
     * build an Index object (corresponding to MySQL index or key) from its 
     * xml description
     */
    Index* buildIndex(XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* e, 
                      bool primary, Table* t);

    Assertion* buildAssertion(XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* e, 
                              Table* t);
 
    Assertion::Operator* 
    buildOperator(XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* e, Table* t);

    Datatype* buildDatatype(XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* e);

    XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* m_doc;
    Rdb* m_rdb;

  };
}
#endif //XERCESBUILDER_H







