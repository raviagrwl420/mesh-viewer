#ifndef SMF_PARSER_H
#define SMF_PARSER_H

#include <mesh.h>

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <map>
#include <string>
#include <math.h>

using std::cout;
using std::cin;
using std::cerr;
using std::endl;

using std::ifstream;
using std::ofstream;

using std::string;
using std::to_string;

using std::map;
using std::make_pair;

extern Mesh *mesh;

void writeSmfFile(string);
void displaySMF(string);

#endif