// $Header: $
// Test program for rdbModel primitive buiding blocks

#include <iostream>
#include <string>
#include "rdbModel/Rdb.h"
#include "rdbModel/Management/Manager.h"
#include "rdbModel/Management/XercesBuilder.h"


main(int, char**) {
  std::string infile("$(RDBMODELROOT)/xml/calibMetaDb.xml)");

  rdbModel::Manager* man = rdbModel::Manger::getManager();

  man->setBuilder(new rdbModel::XercesBuilder);
  man->setInputSource(infile);

  // good errcode is 0
  int errcode = man->build();

  if (!errcode) {
    rdbModel::Rdb rdb = getRdb();
  }

}
  
  
