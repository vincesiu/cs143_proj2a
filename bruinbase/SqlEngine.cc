/**
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include "Bruinbase.h"
#include "SqlEngine.h"

#include <string>
#include "BTreeIndex.h"

#define DEBUG 1

using namespace std;

// external functions and variables for load file and sql command parsing 
extern FILE* sqlin;
int sqlparse(void);


RC SqlEngine::run(FILE* commandline)
{
  fprintf(stdout, "Bruinbase> ");

  // set the command line input and start parsing user input
  sqlin = commandline;
  sqlparse();  // sqlparse() is defined in SqlParser.tab.c generated from
               // SqlParser.y by bison (bison is GNU equivalent of yacc)

  return 0;
}

RC SqlEngine::select(int attr, const string& table, const vector<SelCond>& cond)
{
  RecordFile rf;   // RecordFile containing the table
  RecordId   rid;  // record cursor for table scanning

  RC     rc;
  int    key;     
  string value;
  int    count;
  int    diff;

  // Index Variables
  BTreeIndex index;
  bool indexUse = false;
  IndexCursor cursor;
  bool searchLower = false;
  bool searchUpper = false;
  int searchLowerBound;
  int searchUpperBound;
  int searchVal;

  // open the table file
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }


  // attempt to open the index file
  if (index.open(table + ".idx", 'r') == 0) {
      if (DEBUG) fprintf(stdout, "Success opening index file\n");
      indexUse = true; 
      for (unsigned i = 0; i < cond.size(); i++) {
          // Don't use index if any are invalid
          if ((cond[i].comp == SelCond::NE) || (cond[i].attr == 2)) {
              if (DEBUG) fprintf(stdout, "Inappropriate conditions for using index");
              index.close();
              indexUse = false;
              break;
          }
      }
  }


  // My code
  // RecordId struct:
  // RecordId.pid pageid
  // RecordId.sid slot in the page
  if (indexUse) {
      // So far, no test cases which have both locates and ranges
      // I guess they're mutually exclusive?
      index.debugPrintout();

      // Range algorithm
      for (unsigned i = 0; i < cond.size(); i++) {
          searchVal = atoi(cond[i].value);
          switch(cond[i].comp) {
              case SelCond::LT:
                if (searchLower && searchVal >= searchLowerBound) {
                    searchLowerBound = searchVal - 1;
                } else {
                    searchLower = true;
                    searchLowerBound = searchVal - 1;
                }
                break;
              case SelCond::LE:
                if (searchLower && searchVal > searchLowerBound) {
                    searchLowerBound = searchVal;
                } else {
                    searchLower = true;
                    searchLowerBound = searchVal;
                }
                break;
              case SelCond::GT:
                if (searchLower && searchVal <= searchLowerBound) {
                    searchLowerBound = searchVal + 1;
                } else {
                    searchLower = true;
                    searchLowerBound = searchVal + 1;
                }
                break;
              case SelCond::GE:
                if (searchLower && searchVal < searchLowerBound) {
                    searchLowerBound = searchVal;
                } else {
                    searchLower = true;
                    searchLowerBound = searchVal;
                }
                break;

          }
      }

      if (searchLower && !searchUpper) {
          // lower bound, find lower bound and go up 
            if (DEBUG) fprintf(stdout, "lower bound %d\n", searchLowerBound);
            index.locate(searchLowerBound, cursor);
            while(index.readForward(cursor, key, rid) == 0) {
              rf.read(rid, key, value);
              switch (attr) {
                  case 1:  // SELECT key
                      fprintf(stdout, "%d\n", key);
                      break;
                  case 2:  // SELECT value
                      fprintf(stdout, "%s\n", value.c_str());
                      break;
                  case 3:  // SELECT *
                      fprintf(stdout, "%d '%s'\n", key, value.c_str());
                      break;
              }
            }
      } else if (!searchLower && searchUpper) {
          // upper bound, find lowest item and go to upper bound
            if (DEBUG) fprintf(stdout, "upper bound %d\n", searchUpperBound);

            index.getFirstElement(cursor);
            while(index.readForward(cursor, key, rid) == 0) {
              if (key > searchUpperBound) {
                  break;
              }
              rf.read(rid, key, value);
              switch (attr) {
                  case 1:  // SELECT key
                      fprintf(stdout, "%d\n", key);
                      break;
                  case 2:  // SELECT value
                      fprintf(stdout, "%s\n", value.c_str());
                      break;
                  case 3:  // SELECT *
                      fprintf(stdout, "%d '%s'\n", key, value.c_str());
                      break;
              }
            }
      } else if (searchLower && searchUpper) {
            if (DEBUG) fprintf(stdout, "lower bound %d\n", searchLowerBound);
            if (DEBUG) fprintf(stdout, "upper bound %d\n", searchUpperBound);
          // find lower bound and go to upper bound
      }

      //Locate algorithm
      for (unsigned i = 0; i < cond.size(); i++) {
          if (cond[i].comp == SelCond::EQ) {
              searchVal = atoi(cond[i].value);
              index.locate(searchVal, cursor);
              index.readForward(cursor, key, rid);
              rf.read(rid, key, value);
              switch (attr) {
                  case 1:  // SELECT key
                      fprintf(stdout, "%d\n", key);
                      break;
                  case 2:  // SELECT value
                      fprintf(stdout, "%s\n", value.c_str());
                      break;
                  case 3:  // SELECT *
                      fprintf(stdout, "%d '%s'\n", key, value.c_str());
                      break;
              }
          }
      }

    goto exit_select;
  }
  // scan the table file from the beginning
  rid.pid = rid.sid = 0;
  count = 0;
  while (rid < rf.endRid()) {
    // read the tuple
    if ((rc = rf.read(rid, key, value)) < 0) {
      fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
      goto exit_select;
    }

    // check the conditions on the tuple
    for (unsigned i = 0; i < cond.size(); i++) {
      // compute the difference between the tuple value and the condition value
      switch (cond[i].attr) {
      case 1:
	diff = key - atoi(cond[i].value);
	break;
      case 2:
	diff = strcmp(value.c_str(), cond[i].value);
	break;
      }

      // skip the tuple if any condition is not met
      switch (cond[i].comp) {
      case SelCond::EQ:
	if (diff != 0) goto next_tuple;
	break;
      case SelCond::NE:
	if (diff == 0) goto next_tuple;
	break;
      case SelCond::GT:
	if (diff <= 0) goto next_tuple;
	break;
      case SelCond::LT:
	if (diff >= 0) goto next_tuple;
	break;
      case SelCond::GE:
	if (diff < 0) goto next_tuple;
	break;
      case SelCond::LE:
	if (diff > 0) goto next_tuple;
	break;
      }
    }

    // the condition is met for the tuple. 
    // increase matching tuple counter
    count++;

    // print the tuple 
    switch (attr) {
    case 1:  // SELECT key
      fprintf(stdout, "%d\n", key);
      break;
    case 2:  // SELECT value
      fprintf(stdout, "%s\n", value.c_str());
      break;
    case 3:  // SELECT *
      fprintf(stdout, "%d '%s'\n", key, value.c_str());
      break;
    }

    // move to the next tuple
    next_tuple:
    ++rid;
  }

  // print matching tuple count if "select count(*)"
  if (attr == 4) {
    fprintf(stdout, "%d\n", count);
  }
  rc = 0;

  // close the table file and return
  exit_select:
  rf.close();
  return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
  /* your code here */

  //input data file
  std::ifstream input;
  std::string input_line;
  input.open(loadfile.c_str(), std::ifstream::in);

  //output data file
  RecordFile * out = new RecordFile(table + ".tbl", 'w');

  //Index data structures
  BTreeIndex btree;



  //Create index if needed
  if (index) {
      //Check if the file exists, aborting? or overwrite?
      //TODO
        
      if (btree.open(table + ".idx", 'w') != 0) {
          //Error checking, abort
          //TODO
      }
  }

  while(std::getline(input, input_line)) {
          RecordId rid;
          int key;
          std::string value;
          SqlEngine::parseLoadLine(input_line, key, value);
          out->append(key, value, rid);
          if (index) {
              btree.insert(key, rid);
          }
  }


  if (index) {
      //Error checking needed? Probably nah
      btree.close();
  }

  input.close();
  out->close();

  return 0;
}

RC SqlEngine::parseLoadLine(const string& line, int& key, string& value)
{
    const char *s;
    char        c;
    string::size_type loc;
    
    // ignore beginning white spaces
    c = *(s = line.c_str());
    while (c == ' ' || c == '\t') { c = *++s; }

    // get the integer key value
    key = atoi(s);

    // look for comma
    s = strchr(s, ',');
    if (s == NULL) { return RC_INVALID_FILE_FORMAT; }

    // ignore white spaces
    do { c = *++s; } while (c == ' ' || c == '\t');
    
    // if there is nothing left, set the value to empty string
    if (c == 0) { 
        value.erase();
        return 0;
    }

    // is the value field delimited by ' or "?
    if (c == '\'' || c == '"') {
        s++;
    } else {
        c = '\n';
    }

    // get the value string
    value.assign(s);
    loc = value.find(c, 0);
    if (loc != string::npos) { value.erase(loc); }

    return 0;
}
