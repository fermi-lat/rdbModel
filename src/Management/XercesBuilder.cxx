// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/Management/XercesBuilder.cxx,v 1.19 2005/01/05 01:23:07 jrb Exp $
#include "rdbModel/Management/XercesBuilder.h"
#include "rdbModel/Management/Manager.h"
#include "rdbModel/Tables/Table.h"
#include "rdbModel/Tables/Column.h"
#include "rdbModel/Tables/Assertion.h"
#include "rdbModel/Tables/Datatype.h"
#include "rdbModel/Tables/Index.h"
#include "xmlBase/XmlParser.h"
#include "xmlBase/Dom.h"
#include <iostream>
    
namespace rdbModel {
  using XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument;
  using XERCES_CPP_NAMESPACE_QUALIFIER DOMElement;

  XercesBuilder::XercesBuilder() : Builder(), m_doc(0), m_rdb(0) {
  }

  unsigned int XercesBuilder::parseInput(const std::string& filename) {
    xmlBase::XmlParser parser;

    m_doc = parser.parse(filename.c_str(), "rdbms");

    return (m_doc == 0) ? 0xffffffff : 0;
  }

  int XercesBuilder::buildRdb() {
    Manager* man = Manager::getManager();

    if (m_doc == 0 ) return 0;
    m_rdb = man->getRdb();
    DOMElement* docElt = m_doc->getDocumentElement();

    
    //  save attribute information associated with outermost (rdbms) element.
    m_rdb->m_dbName = xmlBase::Dom::getAttribute(docElt, "dbs");

    m_rdb->m_DTDversion = xmlBase::Dom::getAttribute(docElt, "DTDversion");

    m_rdb->m_CVSid = xmlBase::Dom::getAttribute(docElt, "CVSid");

    // Get vector of table elements.  
    std::vector<DOMElement*> tables;
    xmlBase::Dom::getChildrenByTagName(docElt, "table", tables);
    unsigned int nTable = tables.size();
    unsigned int processed = 0;

    for (unsigned int iTable = 0; iTable < nTable; iTable++) {
      Table* newTable = buildTable(tables[iTable]);
      if (newTable) {
        m_rdb->addTable(newTable);
        processed++;
      }
    }
    return nTable - processed;
  }

  Table* XercesBuilder::buildTable(DOMElement* tableElt) {

    Table* newTable = new Table;
    newTable->m_name = xmlBase::Dom::getAttribute(tableElt, "name");
    newTable->m_version = xmlBase::Dom::getAttribute(tableElt, "version");
    newTable->m_comment = xmlBase::Dom::getAttribute(tableElt, "comment");

    std::vector<DOMElement* > children;
    xmlBase::Dom::getChildrenByTagName(tableElt, "col", children);
    unsigned int nChild = children.size();

    // Delegate handling of columns associated with this table
    for (unsigned int iCol = 0; iCol < nChild; iCol++) {
      Column* newCol = buildColumn(children[iCol], newTable);

      if (newCol) {
        newTable->addColumn(newCol);
      }
    }

    // Look for primary key element, if any
    DOMElement* primaryKey = 
      xmlBase::Dom::findFirstChildByName(tableElt, "primary");
    if (primaryKey != 0) {
      Index* newIndex = buildIndex(primaryKey, true, newTable);
      if (newIndex) {
        newIndex->m_myTable = newTable;
        newTable->addIndex(newIndex);
      }
    }

    // Handle any other indices
    xmlBase::Dom::getChildrenByTagName(tableElt, "index", children);
    nChild = children.size();

    for (unsigned int iIndex = 0; iIndex < nChild; iIndex++) {
      Index* newIndex = buildIndex(children[iIndex], false, newTable);
      if (newIndex) {
        newTable->addIndex(newIndex);
      }
    }
    
    // Check that there is at most one primary key??

    // Handle assertion elements
    xmlBase::Dom::getChildrenByTagName(tableElt, "assert", children);
    nChild = children.size();

    for (unsigned int iAssert = 0; iAssert < nChild; iAssert++) {
      Assertion* newAssert = buildAssertion(children[iAssert], newTable);
      if (newAssert) {
        newTable->addAssert(newAssert);
      }
    }
    return newTable;
  }  

  Column* XercesBuilder::buildColumn(DOMElement* e, Table* myTable) {
    Column* newCol = new Column(myTable);
    //    m_default.clear();
    newCol->m_name = xmlBase::Dom::getAttribute(e, "name");
    DOMElement* com = xmlBase::Dom::findFirstChildByName(e, "comment");
    newCol->m_comment = xmlBase::Dom::getTextContent(com);

    DOMElement* src = xmlBase::Dom::findFirstChildByName(e, "src");

    newCol->m_null = (xmlBase::Dom::getAttribute(src, "null") == "true");

    DOMElement* child = xmlBase::Dom::getFirstChildElement(src);
    if (xmlBase::Dom::checkTagName(child, "default")) {
      newCol->m_from = Column::FROMdefault;
      newCol->m_default = xmlBase::Dom::getAttribute(child, "value");
    }
    else if (xmlBase::Dom::checkTagName(child, "from")) {
      std::string agent = xmlBase::Dom::getAttribute(child, "agent");
      if (agent == "auto_increment") {
        newCol->m_from = Column::FROMautoIncrement;
      }
      else if (agent == "now") {
        newCol->m_from = Column::FROMnow;
      }
      else if (agent == "enduser") {
        newCol->m_from = Column::FROMendUser;
      }
      else if (agent == "service") {
        newCol->m_from = Column::FROMprogram;
        std::string contents = xmlBase::Dom::getAttribute(child, "contents");
        if (contents == "service_name") {
          newCol->m_contents = Column::CONTENTSserviceName;
        }
        else if (contents == "username") {
          newCol->m_contents = Column::CONTENTSusername;
        }
        else if (contents == "enter_time") {
          newCol->m_contents = Column::CONTENTSenterTime;
        }
        // otherwise just stick with default value of CONTENTSunspecified
      }
      // shouldn't be anything else
    } 

    DOMElement* dtype = xmlBase::Dom::findFirstChildByName(e, "type");
    newCol->m_type = buildDatatype(dtype);
    
    return newCol;
  }

  Datatype* XercesBuilder::buildDatatype(DOMElement* e) {
    Datatype* newType = new Datatype;
    newType->setType(xmlBase::Dom::getAttribute(e, "typename"));

    if (xmlBase::Dom::hasAttribute(e, "size")) {
      try {
        newType->m_outputSize = xmlBase::Dom::getIntAttribute(e, "size");
      }
      catch (xmlBase::DomException ex) {
        std::cerr << "Error in rdb database description file" << std::endl;
        std::cerr << ex.getMsg() << std::endl;
        std::cerr << "Ignoring column size specification " << std::endl;
        newType->m_outputSize = -1;        // treat as unspecified
      }
    }
    else newType->m_outputSize = -1;
    if ((newType->m_outputSize == -1) &&
        (newType->getType() == Datatype::TYPEchar) ) newType->m_outputSize = 1;
    if ((newType->m_outputSize == -1) &&
        (newType->getType() == Datatype::TYPEvarchar) ) { // not allowed
      std::cerr << "Error in rdb database description file: " << std::endl;
      std::cerr << "Missing size spec. for varchar field " << std::endl;
      delete newType;
      newType = 0;
      return newType;
    }


    DOMElement* restrict = xmlBase::Dom::getFirstChildElement(e);

    if (restrict != 0) {
      DOMElement* rtype = xmlBase::Dom::getFirstChildElement(restrict);
      std::string tagname = xmlBase::Dom::getTagName(rtype);
      if ((newType->m_type == Datatype::TYPEenum) &&
          (tagname != std::string("enum") ) ) {
        std::cerr << "From rdbMode::XercesBuilder::buildDatatype" << std::endl;
        std::cerr << "Bad enum type. Missing value list " << std::endl;
        delete newType;
        newType = 0;
        return newType;
      }

      if (tagname == std::string("nonnegative")) {
        newType->m_restrict = Datatype::RESTRICTnonneg;
        if (newType->m_isInt) newType->m_minInt = 0;
      }
      else if (tagname == std::string("positive")) {
        newType->m_restrict = Datatype::RESTRICTpos;
        if (newType->m_isInt) newType->m_minInt = 1;
      }
      else if (tagname == std::string("interval")) {
        newType->setInterval(xmlBase::Dom::getAttribute(rtype, "min"),
                             xmlBase::Dom::getAttribute(rtype, "max"));
      }
      else if (tagname == std::string("file")) {
        newType->m_restrict = Datatype::RESTRICTfile;
      }
      else if (tagname == std::string("enum")) {
        newType->m_restrict = Datatype::RESTRICTenum;
        Enum* newEnum  = new Enum();
        newEnum->m_required = 
          (xmlBase::Dom::getAttribute(rtype, "use") == "require");
        if (!(newEnum->m_required) && 
            (newType->m_type == Datatype::TYPEenum)) { //improper enum decl.
          delete newEnum;
          delete newType;
          std::cerr << "From rdbMode::XercesBuilder::buildDatatype" 
                    << std::endl;
          std::cerr << "Bad enum type. List must be 'required' " << std::endl;
          newType = 0;
          return newType;
        }  // end improprer enum decl.
          
        std::string enums = xmlBase::Dom::getAttribute(rtype, "values");

        unsigned int start = 0;
        unsigned int blankLoc = enums.find(std::string(" "), start);

        while (blankLoc != std::string::npos) {
          newEnum->m_choices.push_back(enums.substr(start, blankLoc-start));
          start = blankLoc + 1;
          blankLoc = enums.find(std::string(" "), start);
        }   // parsing enum list
        newEnum->m_choices.push_back(enums.substr(start));
        newType->m_enum = newEnum;
      }   // end processing of enum restriction
    }
    else {   // no restriction specified
      newType->m_restrict = Datatype::RESTRICTnone;
      if (newType->m_type == Datatype::TYPEenum) { 
        std::cerr << "From rdbMode::XercesBuilder::buildDatatype" 
                  << std::endl;
        std::cerr << "Bad enum type. Missing value list " << std::endl;
        delete newType;
        newType = 0;
      }
    }
    return newType;
  }

  Index* XercesBuilder::buildIndex(DOMElement* e, bool primaryElt,
                                   Table* myTable) {
    Index* newIndex = new Index(myTable);

    if (primaryElt) { // DOMElement* is a <primary> 
      newIndex->m_primary = true;
      std::string col = newIndex->m_name = xmlBase::Dom::getAttribute(e, "col");
      newIndex->m_indexCols.push_back(newIndex->m_name);
      Column* myCol = myTable->getColumnByName(col);
      myCol->m_isPrimaryKey = true;
    }
    else { // DOMElement* is <index>
      newIndex->m_name = xmlBase::Dom::getAttribute(e, "name");

      std::string primaryVal = 
        xmlBase::Dom::getAttribute(e, "primary");
      newIndex->m_primary = (primaryVal == "yes");

      // Value of "cols" attribute is a blank-separated list of column names
      std::string cols = xmlBase::Dom::getAttribute(e, "cols");

      // Could make this more robust by checking there is really just one below
      if (newIndex->m_primary) {   // had better be just one column
        Column* myCol = myTable->getColumnByName(cols);
        myCol->m_isPrimaryKey = true;
      }

      unsigned int start = 0;
      unsigned int blankLoc = cols.find(std::string(" "), start);

      while (blankLoc != std::string::npos) {
        newIndex->m_indexCols.push_back(cols.substr(start, blankLoc-start));
        start = blankLoc + 1;
        blankLoc = cols.find(std::string(" "), start);
      }
      newIndex->m_indexCols.push_back(cols.substr(start));

    }
    return newIndex;
  }

  Assertion* XercesBuilder::buildAssertion(DOMElement* e, Table* myTable) {

    
    std::string when = xmlBase::Dom::getAttribute(e, "case");
    
    Assertion::WHEN whenType = (when == "globalCheck") ? 
      Assertion::WHENglobalCheck  : Assertion::WHENchangeRow;
    DOMElement* opElt = xmlBase::Dom::getFirstChildElement(e);
    Assertion::Operator* op = buildOperator(opElt, myTable);

    Assertion* newAssert = new Assertion(whenType, op, myTable);

    return newAssert;
  }


  Assertion::Operator* XercesBuilder::buildOperator(DOMElement* e, 
                                                    Table* myTable) {
    std::string opName = xmlBase::Dom::getTagName(e);
    OPTYPE opType;
    if (opName == "isNull") {
      return new Assertion::Operator(OPTYPEisNull, 
                                     xmlBase::Dom::getAttribute(e, "col"),
                                     std::string(""), false, false);
    }
    else if (opName == "compare") {
      std::string relation = xmlBase::Dom::getAttribute(e, "relation");
      if (relation == "lessThan") opType = OPTYPElessThan;
      else if (relation == "greaterThan") {
        opType = OPTYPEgreaterThan;
      }
      else if (relation == "equal") opType = OPTYPEequal;
      else if (relation == "notEqual") 
        opType = OPTYPEnotEqual;
      else if (relation == "lessOrEqual") {
        opType = OPTYPElessOrEqual;
      }
      else if (relation == "greaterOrEqual") {
        opType = OPTYPEgreaterOrEqual;
      }
      DOMElement* child[2];
      child[0] = xmlBase::Dom::getFirstChildElement(e);
      child[1] = xmlBase::Dom::getSiblingElement(child[0]);

      std::string compareArgs[2];
      bool isLit[2];
      for (unsigned iChild = 0; iChild < 2; iChild++) {
        compareArgs[iChild] = 
          xmlBase::Dom::getAttribute(child[iChild], "val");
      
        // Element is either a <colRef> or a <value>  
        isLit[iChild] = (xmlBase::Dom::checkTagName(child[iChild], "value")) ;
      }
      Assertion::Operator* newOp = 
        new Assertion::Operator(opType, compareArgs[0], compareArgs[1],
                                isLit[0], isLit[1]);
      if (!newOp->validCompareOp(myTable)) {
        delete newOp;
        return 0;
      }
      return newOp;
    } 

    // All other cases have other operators as children
    else if (opName == "exists") {
      std::string tableName;
      opType = OPTYPEexists;
      if (xmlBase::Dom::hasAttribute(e, "tableName") ) {
        tableName = 
          xmlBase::Dom::getAttribute(e, "tableName");
      }
      else tableName = myTable->getName();
      DOMElement* child = xmlBase::Dom::getFirstChildElement(e);
      Assertion::Operator* childOp = buildOperator(child, myTable);
      return new Assertion::Operator(opType, tableName, childOp);
    }

    else if (opName == "or") opType = OPTYPEor;
    else if (opName == "and") opType = OPTYPEand;
    else if (opName == "not") opType = OPTYPEnot;

    // Recursively handle child operators
    std::vector<DOMElement*> children;
    std::vector<Assertion::Operator*> childOps;
    xmlBase::Dom::getChildrenByTagName(e, "*", children);
    unsigned nChild = children.size();
    for (unsigned iChild = 0; iChild < nChild; iChild++) {
      Assertion::Operator* childOp = buildOperator(children[iChild], myTable);
      if (childOp) {
        childOps.push_back(childOp);
      }
      else { // one bad apple and we're dead
        return 0;
      }
    }
    return new Assertion::Operator(opType, childOps);
  }
}
