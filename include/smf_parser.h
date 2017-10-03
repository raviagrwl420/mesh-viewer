#include <iostream>
#include <fstream>
#include <cstdlib>
#include <map>
#include <string>

using std::cout;
using std::cin;
using std::cerr;
using std::endl;

using std::string;
using std::to_string;

using std::map;
using std::make_pair;

using std::ifstream;
using std::ofstream;

// Winged Edge
struct W_edge {
	struct Vertex *start, *end;
	struct Face *left, *right;
	struct W_edge *left_prev, *left_next;
	struct W_edge *right_prev, *right_next;
	W_edge () {};
	W_edge (Vertex *start, Vertex *end) : start(start), end(end) {};
	W_edge (Vertex *start, Vertex *end, Face *left) : start(start), end(end), left(left) {};
};

// Vertex
struct Vertex {
	float x, y, z;
	struct W_edge *edge;
	Vertex () {};
	Vertex (float x, float y, float z) : x(x), y(y), z(z) {};
};

// Face
struct Face {
	int a, b, c;
	struct W_edge *edge;
	Face () {};
	Face (W_edge *edge) : edge(edge) {};
};

// Global Variables
extern int numVertices, numFaces;
extern map<string, W_edge*> edgeMap;
extern map<int, Vertex*> vertexMap;
extern map<int, Face*> faceMap;
extern map<Vertex*, int> vertexIndexMap;

void writeSmfFile(string);
void displaySMF(string);
