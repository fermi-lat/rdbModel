// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/rdbModel/Management/XercesBuilder.h,v 1.3 2004/03/07 08:20:16 jrb Exp $
#ifndef RDBMODEL_XERCESBUILDER_H
#define RDBMODEL_XERCESBUILDER_H

#include "rdbModel/Management/Builder.h"
#include "xml/XmlParser.h"
#include <xercesc/dom/DOM_Document.hpp>
#include <xercesc/dom/DOM_Element.hpp>
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
    Table* buildTable(DOM_Element e);

    /**
     *  build a Column from its xml description and return a pointer to it
     */
    Column* buildColumn(DOM_Element e, Table* t);

    /**
     * build an Index object (corresponding to MySQL index or key) from its 
     * xml description
     */
    Index* buildIndex(DOM_Element e, bool primary, Table* t);

    Assertion* buildAssertion(DOM_Element e, Table* t);
 
    Assertion::Operator* buildOperator(DOM_Element e, Table* t);

    Datatype* buildDatatype(DOM_Element e);

    DOM_Document m_doc;
    Rdb* m_rdb;

  };
}
#endif //XERCESBUILDER_H







