//
// File IO helpers

#pragma once

#include <map>
#include <string>

using namespace std;

#include "BaseClasses.h"

namespace FileHelpers {
map<string, map<string, string>> readMapFromFile(string filename);
bool WriteMapToFile(map<string, map<string, string>> filemap, string filename);
int WriteKeyStringToINIFile(string sSection,         // section name
                            string sKeyName,         // key name
                            string sValue,           // value to write
                            string sFilename);       // initialization file name
int GetKeyStringFromINIFile(string sSection,         // section name
                            string sKeyName,         // key name
                            string sDefault,         // default string
                            string &sReturnedString, // destination buffer
                            string sFilename);       // initialization file name
int GetKeyDoubleFromINIFile(string sSection,         // section name
                            string sKeyName,         // key name
                            double dDefault,         // default
                            double &dReturnedDouble, // destination buffer
                            string sFilename);       // initialization file name
int GetKeyIntFromINIFile(string sSection,            // section name
                         string sKeyName,            // key name
                         int nDefault,               // default
                         int &nReturnedInt,          // destination buffer
                         string sFilename);          // initialization file name
int GetKeyVectorFromINIFile(string sSection,         // section name
                            string sKeyName,         // key name
                            BVector vDefault,        // default
                            BVector &vReturnedVector, // destination buffer
                            string sFilename); // initialization file name
}; // namespace FileHelpers
