#ifndef SMF_PARSER_H
#define SMF_PARSER_H

#include <mesh.h>
#include <iostream>
#include <fstream>
#include <cstdlib>

using std::cout;
using std::cin;
using std::cerr;
using std::endl;

using std::ifstream;
using std::ofstream;

Mesh *parseSmfFile(string);
void writeSmfFile(Mesh *mesh, string);

#endif