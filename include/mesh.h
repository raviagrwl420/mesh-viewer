#ifndef MESH_H
#define MESH_H

#include <vector.h>
#include <map>
#include <string>

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

	W_edge (Vertex *start, Vertex *end) : start(start), end(end) {};
	
	W_edge (Vertex *start, Vertex *end, Face *left) : start(start), end(end), left(left) {};
};

// Vertex
struct Vertex {
	float x, y, z;
	struct W_edge *edge;
	struct Vector *vector;
	struct Vector *normal;
	
	Vertex (float x, float y, float z);
};

// Face
struct Face {
	struct W_edge *edge;
	
	Face (W_edge *edge) : edge(edge) {};
};

// Mesh
struct Mesh {
	int numVertices, numFaces;
	map<string, W_edge*> edgeMap;
	map<int, Vertex*> vertexMap;
	map<int, Face*> faceMap;
	map<Vertex*, int> vertexIndexMap;
	map<Face*, int> faceIndexMap;
	map<string, Vector*> faceNormalMap;
	float xMin, yMin, zMin, xMax, yMax, zMax;

	Mesh (int numVertices, int numFaces) : numVertices(numVertices), numFaces(numFaces) {};

	Vertex *insertVertex (float x, float y, float z);

	W_edge *insertEdge (int v1, int v2);

	Face *insertFace (W_edge *edge);

	void insertTriangle (int v1, int v2, int v3);

	void insertFaceNormal (Face *f, int v1, int v2, int v3);

	void updateVertexNormalForVertex (Face *f, int v1);

	void updateVertexNormalForEachVertex (Face *f, int v1, int v2, int v3);

	W_edge *getEdge (int v1, int v2);
	
	Vector *getFaceNormal (Face *f);

	void getAllEdgesForFace (Face *f, W_edge *edges[3]);

	void getAllVerticesForFace (Face *f, Vertex *vertices[3]);

	void computeBoundingBox ();
};

string getEdgeKey (int v1, int v2);

Vector *getFaceNormalVector (Vertex *v1, Vertex *v2, Vertex *v3);

#endif