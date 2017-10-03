#include <vector.h>
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
	struct Vector *normal;
	Vertex () {
		this->normal = NULL;
	};
	Vertex (float x, float y, float z) : x(x), y(y), z(z) {
		this->normal = NULL;
	};
};

// Face
struct Face {
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

extern map<string, Vector*> faceNormalMap;
extern map<Vertex*, Vector*> vertexNormalMap;

Vector *getVertexNormal(Vertex*);
string getFaceNormalKey(int, Vertex*);
void getAllVerticesForFace(Face*, Vertex**);
void writeSmfFile(string);
void displaySMF(string);
