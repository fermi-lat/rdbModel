// $Header: /nfs/slac/g/glast/ground/cvs/rdbModel/src/test/test_build.cxx,v 1.1 2004/03/06 01:16:08 jrb Exp $
// Test program for rdbModel primitive buiding blocks

#include <iostream>
#include <string>
#include "rdbModel/Rdb.h"
#include "rdbModel/Management/Manager.h"
#include "rdbModel/Management/XercesBuilder.h"


int main(int, char**) {
  std::string infile("$(RDBMODELROOT)/xml/calibMetaDb.xml)");

  rdbModel::Manager* man = rdbModel::Manager::getManager();

  man->setBuilder(new rdbModel::XercesBuilder);
  man->setInputSource(infile);

  // good errcode is 0
  int errcode = man->build();

  if (!errcode) {
    rdbModel::Rdb* rdb = man->getRdb();
  }
  return 0;
}
  
  
