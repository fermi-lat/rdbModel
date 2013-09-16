// $Header: /nfs/slac/g/glast/ground/cvs/GlastRelease-scons/rdbModel/src/test/copyCalibration.cxx,v 1.3 2013/09/16 21:15:59 jrb Exp $
// Make copies of existing calib. metadata, substituting values for flavor
// as specified.   Arguments are
//   filepath for file containing list of serial numbers
//   new value for flavor field
//   (optional) If it's a string starting with "prod" or "Prod" use 
//              production db calib.  Otherwise use calib_test.
//   .my.cnf needs entry for group copyCalibration including
//    host, user, password

#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include "rdbModel/Rdb.h"
#include "rdbModel/RdbException.h"
#include "rdbModel/Management/Manager.h"
#include "rdbModel/Management/XercesBuilder.h"
#include "rdbModel/Db/MysqlConnection.h"
#include "rdbModel/Db/MysqlResults.h"
#include "facilities/Util.h"
#include "facilities/commonUtilities.h"


int copyOneByFlavor(rdbModel::Rdb* rdb, const std::string& serial, 
                    const std::string& newFlavor, bool doWrite);

rdbModel::Rdb* makeConnection(bool isProduction, bool forWrite=false);

int main(int argc, char* argv[]) {
  using rdbModel::FieldVal;

  if (argc < 3) {
    std::cout << "copyCalibration call looks like this:" << std::endl;
    std::cout << "copyCalibration FILEPATH FLAVOR [Test | Prod]" << std::endl;
    std::cout << "FILEPATH is path to file containing list of serial numbers"
              << std::endl;
    std::cout << "of calibrations to be copied" << std::endl;
    std::cout << "By default no actual inserts are done unless optional third arg." << std::endl;
    std::cout << "is Prod or Test" << std::endl;
    std::cout << "For prod inserts, add group [copyCalibration] with suitable values"
              << std::endl;
    std::cout << "for host, user, password to .my.cnf" << std::endl;
    std::cout << "See also rdbModel/doc/copyCalibration_instructions.txt." 
              << std::endl;
    exit(0);
  }
  std::string infile = std::string(argv[1]);
  std::string newFlavor = std::string(argv[2]);
  
  // Set doWrite to true if so requested
  bool doWrite = false;
  bool useProduction = true;
  if (argc > 3) {
    std::string a3 = std::string(argv[3]).substr(0, 4);
    if ((a3 == "Test") || (a3 == "test")) {
      // really do write, but to test db
      useProduction = false;
      doWrite = true;
    }
    else if ((a3 == "Prod") || (a3 == "prod")) { // write to production db
      doWrite = true;
    }
  }

  rdbModel::Rdb* rdb = makeConnection(useProduction, doWrite);
  if (!rdb) {
    std::cerr << "FATAL: unable to connect " << std::endl;
    exit(1);
  }
  // Open file (1st argument)
  std::ifstream f(infile.c_str());
  if (!f.is_open()) {
    std::cerr << "Unable to open file " << infile << std::endl;
    exit(1);
  }
  bool more = 1;

  while (more) {
    char buf[21];
    f.getline(buf, 20);
    if (f.eof() ) {
      more = false;
      continue;
    }
    std::string num(buf);
    try {
      facilities::Util::stringToInt(num);
    }
    catch (facilities::WrongType ex) {
      std::cout << num << " is not an integer. Continuing with next line.."
                << std::endl;
      continue;
    }
    copyOneByFlavor(rdb, num, newFlavor, doWrite);
  }
  // Close file
  f.close();
  // Close db connection
}

int copyOneByFlavor(rdbModel::Rdb* rdb, const std::string& serial, 
                    const std::string& newFlavor, bool doWrite)  {
  // check value read is a valid integer
  // select almost all columns for the serial number
  //   if fails, put out message and continue
  // Compose new row, 
  //   insert
  // Make a query

  // Make a bad query with con->select
  rdbModel::Connection* con = rdb->getConnection();
  static std::vector<std::string>* getCols = 0;
  std::string where(" WHERE ser_no='");
  where += serial + std::string("'");

  if (getCols == 0) { // initialize
    getCols = new std::vector<std::string>;
    getCols->reserve(12);
    // These crucial columns must be copied
    getCols->push_back("instrument");
    getCols->push_back("calib_type");
    getCols->push_back("data_fmt");
    getCols->push_back("data_ident");
    getCols->push_back("vstart");
    getCols->push_back("vend");
    getCols->push_back("proc_level");
    getCols->push_back("completion");


    // Copy these as well, except append a note to "notes" (which
    // typically is empty)
    getCols->push_back("locale");
    getCols->push_back("input_desc");
    getCols->push_back("fmt_version");
    getCols->push_back("notes");
  }

  // Also will explicitly set flavor as specified and set creator
  // Ignore prod-start, prod_end, input_start, input_end, data_size

  // insert utility will handle uid, enter_time and update_time

  rdbModel::ResultHandle* res;
  try {
    res = con->select("metadata_v2r1", *getCols, *getCols, where);
  }
  catch (rdbModel::RdbException ex) {
    std::cerr << "select failed with error: " << ex.getMsg() << std::endl;
      //      std::cerr << "Code " << ex.getCode() << std::endl;
  }
  // fetch results
  if (res->getNRows() == 0) {
    std::cerr << "No existing entry with ser_no=" << serial << std::endl;
    std::cerr << "Skipping this item" << std::endl;
    delete res;
    return 0;
  }
  std::vector<std::string> oldVals;
  
  bool ok = res->getRow(oldVals);
  delete res;

  ///////////   needs revision; stolen from old test program
  if (doWrite) {
    using rdbModel::FieldVal;
    using rdbModel::Row;

    std::vector<FieldVal> fields;
    fields.reserve(15);

    fields.push_back(FieldVal("instrument", oldVals[0]));      // copied
    fields.push_back(FieldVal("calib_type",oldVals[1]));  // copied
    fields.push_back(FieldVal("data_fmt", oldVals[2]));     // copied
    fields.push_back(FieldVal("data_ident",oldVals[3]));
    fields.push_back(FieldVal("vstart", oldVals[4]));  // copied
    fields.push_back(FieldVal("vend", oldVals[5]));  // copied
    fields.push_back(FieldVal("proc_level", oldVals[6]));  // copied
    fields.push_back(FieldVal("completion", oldVals[7]));   // copied
    fields.push_back(FieldVal("locale", oldVals[8]));       // copied
    fields.push_back(FieldVal("input_desc", oldVals[9]));
    fields.push_back(FieldVal("fmt_version", oldVals[10]));
    fields.push_back(FieldVal("notes", 
                              oldVals[11] + " near-copy of " + serial));
    fields.push_back(FieldVal("flavor",  newFlavor));          // use arg
    fields.push_back(FieldVal("creator", "CopyCalibration"));
    int  newSerial = 0;
  
    Row row(fields);

    int ret = rdb->insertRow("metadata_v2r1", row, &newSerial);
    if (ret) {
      unsigned errcode = rdb->getConnection()->getLastError();
      std::cerr << "From doBadInsert.  Last error code was " << errcode 
                << std::endl;
      if (rdb->duplicateError() ) {
        std::cerr << "Last error was duplicate insert " << std::endl;
      }
      else {
        std::cerr << "Last error was NOT duplicate insert " << std::endl;
      }
    }  else {
      std::cout << "Inserted near-copy of  " << serial << " with ser_no="
                << newSerial << std::endl;
    }
    
    return newSerial;
  }   else {
    std::cout << "Would write new row with " << std::endl;
    std::cout << "instrument=" << oldVals[0] << ", calib_type=" << oldVals[1]
              << std::endl;
    std::cout << "data_fmt=" << oldVals[2] << ", data_ident=" << oldVals[3]
              << std::endl;
    std::cout << "vstart=" << oldVals[4] << ", vend=" <<  oldVals[5] 
              << std::endl;
    std::cout << "proc_level=" << oldVals[6] << ", completion=" << oldVals[7]
              << std::endl;
    std::cout << "locale=" << oldVals[8] << ", input_desc=" << oldVals[9]
              << std::endl;
    std::cout << "fmt_version="  << oldVals[10] << std::endl;
    std::cout << "notes=" << oldVals[11] << " near-copy of " << serial 
              << std::endl;
    std::cout << "flavor=" << newFlavor << ", creator=CopyCalibration"
              << std::endl;
    return 0;
  }
}

rdbModel::Rdb* makeConnection(bool isProduction, bool forWrite) {

  facilities::commonUtilities::setupEnvironment();
  std::string xmlPath = facilities::commonUtilities::getXmlPath("rdbModel");
  std::string infile = xmlPath + std::string("/calib.xml");
  std::string readOnly("glastreader");

  rdbModel::Builder* b = new rdbModel::XercesBuilder;
  rdbModel::Rdb* rdb = new rdbModel::Rdb;
  int errcode = rdb->build(infile, b);

  if (errcode) {
    std::cerr << "Build failed with error code " << errcode << std::endl;
    return 0;
  }
  
  // Connect to production database with write access
  rdbModel::MysqlConnection* con = new rdbModel::MysqlConnection();

  // Use defaults file
  con->init();
  if (isProduction) {
    con->setOption(rdbModel::DBreadDefaultGroup, "copyCalibration");
    if (forWrite) {
      con->open("glastCalibDB.slac.stanford.edu", 0, 0, "calib");
    } else {
      con->open("glastCalibDB.slac.stanford.edu", readOnly.c_str(), 
                readOnly.c_str(), "calib");
    }
  } else {
    con->setOption(rdbModel::DBreadDefaultGroup, "copyCalibration_test");
    con->open("glastCalibDB.slac.stanford.edu", 0, 0, "calib_test");
  }

    // Look for compatibility at least
  rdbModel::MATCH match = con->matchSchema(rdb, false);

  switch (match) {
  case rdbModel::MATCHequivalent:
    std::cout << "XML schema and MySQL database are equivalent!" << std::endl;
    break;
  case rdbModel::MATCHcompatible:
    std::cout << "XML schema and MySQL database are compatible" << std::endl;
    break;
  case rdbModel::MATCHfail:
    std::cout << "XML schema and MySQL database are NOT compatible" 
              << std::endl;
    con->close();
    return 0;
  case rdbModel::MATCHnoConnection:
    std::cout << "Connection failed while attempting match" << std::endl;
    con->close();
    return 0;
  }
  return rdb;
}
