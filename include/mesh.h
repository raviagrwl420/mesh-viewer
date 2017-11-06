#ifndef MESH_H
#define MESH_H

#include <map>
#include <string>
#include <glm/glm.hpp>
#include <random>

#define PI 3.14159265

using std::string;
using std::to_string;

using std::map;
using std::make_pair;

using glm::vec3;
using glm::vec4;
using glm::normalize;
using glm::cross;
using glm::dot;

using glm::mat4;
using glm::outerProduct;

// Subdivision Types
enum SubdivisionType {BUTTERFLY, LOOP};

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
	vec3 position;
	vec3 normal;
	mat4 quadric;

	struct W_edge *edge;
	
	Vertex (float x, float y, float z);
	Vertex (vec3 position);
};

// Face
struct Face {
	struct W_edge *edge;
	
	Face (W_edge *edge) : edge(edge) {};
};

// Mesh
struct Mesh {
	int numVertices, numEdges, numFaces;
	map<string, W_edge*> edgeMap;
	map<int, string> edgeKeyMap;
	map<W_edge*, int> edgeIndexMap;
	map<int, Vertex*> vertexMap;
	map<int, Face*> faceMap;
	map<Vertex*, int> vertexIndexMap;
	map<Face*, int> faceIndexMap;
	map<string, vec3> faceNormalMap;
	float xMin, yMin, zMin, xMax, yMax, zMax;

	Mesh () {};

	Mesh (int numVertices, int numFaces) : numVertices(numVertices), numFaces(numFaces) {};

	Vertex *insertVertex (float x, float y, float z);

	Vertex *insertVertex (Vertex *vertex);

	void deleteVertex (Vertex *vertex);

	W_edge *insertEdge (int v1, int v2);

	void deleteEdge (W_edge *edge);

	Face *insertFace (W_edge *edge);

	void deleteFace (Face *face);

	void insertTriangle (int v1, int v2, int v3);

	void insertFaceNormal (Face *f, int v1, int v2, int v3);

	void updateVertexNormalForVertex (Face *f, int v1);

	void updateVertexNormalForEachVertex (Face *f, int v1, int v2, int v3);

	W_edge *getEdge (int v1, int v2);
	
	vec3 getFaceNormal (Face *f);

	void getAllEdgesForFace (Face *f, W_edge *edges[3]);

	void getAllVerticesForFace (Face *f, Vertex *vertices[3]);

	void computeBoundingBox ();

	// Subdivision
	Mesh *loopSubdivision ();

	Mesh *butterflySubdivision ();

	Mesh *subdivideMesh (int subdivisionType, int subdivisionLevel);

	// Decimation

	mat4 getQuadric (Face *f, Vertex *v);

	void updateQuadricForVertex (Face *f, int v1);

	void updateQuadricForEachVertex (Face *f, int v1, int v2, int v3);

	vec4 computeNewVertexPositionForEdgeCollapse(W_edge *edge);

	W_edge *getCandidateEdgeToCollapse (int k, map<W_edge*, bool> flagged);

	bool canCauseFoldOver (W_edge *edge);
	
	bool canCauseNonManifoldMesh (W_edge *edge);

	void collapseEdge (W_edge *edge);

	void decimate (int k, int n);
};

string getEdgeKey (int v1, int v2);

vec3 getFaceNormalVector (Vertex *v1, Vertex *v2, Vertex *v3);

Vertex *getNextVertex (W_edge *edge, W_edge *next_edge);

#endif