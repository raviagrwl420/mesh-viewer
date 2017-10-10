#include <mesh.h>

// Vertex Constructor
Vertex::Vertex (float x, float y, float z) {
	this->x = x;
	this->y = y;
	this->z = z;
	this->vector = new Vector(x, y, z);
	this->normal = new Vector(0.0, 0.0, 0.0);
}

void Mesh::computeBoundingBox () {
	xMin = xMax = vertexMap[1]->x;
	yMin = yMax = vertexMap[1]->y;
	zMin = zMax = vertexMap[1]->z;

	for (int i = 1; i <= numVertices; i++) {
		float xVal = vertexMap[i]->x;
		float yVal = vertexMap[i]->y;
		float zVal = vertexMap[i]->z;
		
		if (xVal < xMin)
			xMin = xVal;
		
		if (xVal > xMax)
			xMax = xVal;

		if (yVal < yMin)
			yMin = yVal;
		
		if (yVal > yMax)
			yMax = yVal;

		if (zVal < zMin)
			zMin = zVal;
		
		if (zVal > zMax)
			zMax = zVal;
	}
}

// Insert a vertex in the mesh
Vertex *Mesh::insertVertex (float x, float y, float z) {
	Vertex *vertex = new Vertex(x, y, z);

	return insertVertex(vertex);
}

// Insert a vertex in the mesh
Vertex *Mesh::insertVertex (Vertex *vertex) {
	int index = vertexMap.size() + 1;

	vertexMap.insert(make_pair(index, vertex));
	vertexIndexMap.insert(make_pair(vertex, index));

	numVertices = vertexMap.size();

	return vertex;
}

// Insert an edge in the mesh
W_edge *Mesh::insertEdge (int v1, int v2) {
	W_edge *edge = new W_edge(vertexMap[v1], vertexMap[v2]);

	edgeMap.insert(make_pair(getEdgeKey(v1, v2), edge));
	
	return edge;
}

// Insert a face in the mesh
Face *Mesh::insertFace (W_edge *edge) {
	Face *face = new Face(edge);

	int index = faceMap.size() + 1;
	
	faceMap.insert(make_pair(index, face));
	faceIndexMap.insert(make_pair(face, index));

	numFaces = faceMap.size();

	return face;
}

// Insert a triangle in the mesh
void Mesh::insertTriangle (int v1, int v2, int v3) {
	W_edge *e1, *e2, *e3;
	Face *f;
	int e1Dir, e2Dir, e3Dir;
	
	e1 = getEdge(v1, v2);

	if (e1 == NULL) {
		e1 = insertEdge(v1, v2);
		e1Dir = 1;
		
		f = insertFace(e1); 
		e1->left = f;
		
		vertexMap[v1]->edge = e1;
	} else {
		e1Dir = -1;
		f = insertFace(e1); 
		e1->right = f;		
	}

	e2 = getEdge(v2, v3);

	if (e2 == NULL) {
		e2 = insertEdge(v2, v3);
		e2Dir = 1;

		e2->left = f;
		
		vertexMap[v2]->edge = e2;
	} else {
		e2Dir = -1;
		e2->right = f;
	}
	
	e3 = getEdge(v3, v1);

	if (e3 == NULL) {
		e3 = insertEdge(v3, v1);
		e3Dir = 1;

		e3->left = f;

		vertexMap[v3]->edge = e3;
	} else {
		e3Dir = -1;
		e3->right = f;
	}

	// Update edge references
	if (e1Dir > 0) {
		e1->left_next = e2;
		e1->left_prev = e3;
	} else {
		e1->right_next = e3;
		e1->right_prev = e2;
	}

	if (e2Dir > 0) {
		e2->left_next = e3;
		e2->left_prev = e1;
	} else {
		e2->right_next = e1;
		e2->right_prev = e3;
	}

	if (e3Dir > 0) {
		e3->left_next = e1;
		e3->left_prev = e2;
	} else {
		e3->right_next = e2;
		e3->right_prev = e1;
	}

	insertFaceNormal(f, v1, v2, v3);
	updateVertexNormalForEachVertex(f, v1, v2, v3);
}

// Insert the face normal for vertex v1
void Mesh::insertFaceNormal (Face *f, int v1, int v2, int v3) {
	Vector *faceNormal = getFaceNormalVector(vertexMap[v1], vertexMap[v2], vertexMap[v3]);
	faceNormalMap.insert(make_pair(to_string(faceIndexMap[f]), faceNormal));
}

// Update the vertex normal for vertex v by adding the face normal
void Mesh::updateVertexNormalForVertex (Face *f, int v) {
	Vertex *vertex = vertexMap[v];
	Vector *faceNormal = getFaceNormal(f);
	vertex->normal = *(vertex->normal) + *faceNormal;
}

// Update the vertex normal for each vertex v1, v2, and v3
void Mesh::updateVertexNormalForEachVertex (Face *f, int v1, int v2, int v3) {
	updateVertexNormalForVertex(f, v1);
	updateVertexNormalForVertex(f, v2);
	updateVertexNormalForVertex(f, v3);	
}

// Fetch an edge between two vertices v1 and v2
W_edge *Mesh::getEdge (int v1, int v2) {	
	map<string, W_edge*>::iterator it = edgeMap.find(getEdgeKey(v1, v2));
	
	if (it != edgeMap.end())
		return it->second;
	else
		return NULL;
}

// Fetch the face normal for vertex
Vector *Mesh::getFaceNormal (Face *f) {
	map<string, Vector*>::iterator it = faceNormalMap.find(to_string(faceIndexMap[f]));

	if (it != faceNormalMap.end())
		return it->second;
	else
		return NULL;
}

// Get all edges for a face
void Mesh::getAllEdgesForFace (Face *f, W_edge *edges[3]) {
	W_edge *e = f->edge;

	for (int i = 0; i < 3; i++) {
		edges[i] = e;

		if (e->right == f)
			e = e->right_prev;
		else
			e = e->left_next;
	}
}

// Get all vertices for a face
void Mesh::getAllVerticesForFace (Face *f, Vertex *vertices[3]) {
	W_edge *edge1 = f->edge;

	if (edge1->right == f) {
		vertices[0] = edge1->end;
		vertices[1] = edge1->start;
		vertices[2] = getNextVertex(edge1, edge1->right_next);
	} else {
		vertices[0] = edge1->start;
		vertices[1] = edge1->end;
		vertices[2] = getNextVertex(edge1, edge1->left_next);
	}
}

// Compute the key to store the edge between vertices v1, and v2 in edgeMap
string getEdgeKey (int v1, int v2) {
	if (v1 < v2)
		return to_string(v1) + "|" + to_string(v2);
	else
		return to_string(v2) + "|" + to_string(v1);
}

// Get the face normal from the first vertex
Vector *getFaceNormalVector (Vertex *v1, Vertex *v2, Vertex *v3) {
	Vector *e1 = *v2->vector - *v1->vector;
	Vector *e2 = *v3->vector - *v1->vector;

	return cross(*e1, *e2);
}

// Get next vertex
Vertex *getNextVertex (W_edge *edge, W_edge *next_edge) {
	if (edge->start == next_edge->start || edge->end == next_edge->start) {
		return next_edge->end;
	} else if (edge->start == next_edge->end || edge->end == next_edge->end) {
		return next_edge->start;
	}
}

// Subdivision

// Compute mid point vertex for an edge
Vertex *computeMidpoint (W_edge *edge) {
	Vertex *left_vertex = getNextVertex(edge, edge->left_next);
	Vertex *right_vertex = getNextVertex(edge, edge->right_next);

	Vector *newVertexPosition = edge->start->vector->scalar_mult(3.0/8.0);
	newVertexPosition = *newVertexPosition + *edge->end->vector->scalar_mult(3.0/8.0);
	newVertexPosition = *newVertexPosition + *left_vertex->vector->scalar_mult(1.0/8.0);
	newVertexPosition = *newVertexPosition + *right_vertex->vector->scalar_mult(1.0/8.0);

	return new Vertex(newVertexPosition->x, newVertexPosition->y, newVertexPosition->z);
}

// Get the degree of a vertex
int getDegreeOfVertex(Vertex *vertex) {
	W_edge *e1 = vertex->edge;
	int count = 0;

	W_edge *e = e1;
	do {
		if (e->start == vertex) {
			e = e->left_prev;
		} else {
			e = e->right_next;
		}
		count++;
	} while (e != e1);

	return count;
}

// Compute beta for Loop subdivision
float computeBeta (int k) {
	float beta = (1.0/4.0) * cos((2*PI)/k);
	beta = powf((3.0/8.0) + beta, 2);
	beta = (5.0/8.0 - beta) / k;
	return beta;
}

// Compute new vertex for a vertex
Vertex *computeNewVertex (Vertex *vertex) {
	int k = getDegreeOfVertex(vertex);
	float beta = computeBeta(k);

	Vector *newVertexPosition = vertex->vector->scalar_mult(1 - k*beta);

	Vertex *nextVertex;
	W_edge *e = vertex->edge;
	for (int i = 0; i < k; i++) {
		if (e->start == vertex) {
			nextVertex = getNextVertex(e, e->left_prev);
			e = e->left_prev;
		} else {
			nextVertex = getNextVertex(e, e->right_next);
			e = e->right_next;
		}

		newVertexPosition = *newVertexPosition + *nextVertex->vector->scalar_mult(beta);
	}

	return new Vertex(newVertexPosition->x, newVertexPosition->y, newVertexPosition->z);
}

// Perform Loop subdivision
Mesh *Mesh::loopSubdivision () {
	Mesh *subdividedMesh = new Mesh();
	map<string, Vertex*> edgeVertexMap;
	Vertex *newVertex;

	// Compute new vertices for each vertex
	for (int i = 1; i <= numVertices; i++) {
		newVertex = computeNewVertex(vertexMap[i]);
		subdividedMesh->insertVertex(newVertex);
	}

	// Compute new mid point vertices for each edge
	W_edge *edge;
	for (map<string, W_edge*>::const_iterator it = edgeMap.begin(); it != edgeMap.end(); it++) {
		edge = it->second;

		int startIndex = vertexIndexMap[edge->start];
		int endIndex = vertexIndexMap[edge->end];

		newVertex = computeMidpoint(edge);
		edgeVertexMap.insert(make_pair(getEdgeKey(startIndex, endIndex), newVertex));
		subdividedMesh->insertVertex(newVertex);
	}

	// Finish by adding connections
	Face *face;
	Vertex *vertices[3];
	for (int i = 1; i <= numFaces; i++) {
		face = faceMap[i];
		getAllVerticesForFace(face, vertices);

		int a = vertexIndexMap[vertices[0]];
		int b = vertexIndexMap[vertices[1]];
		int c = vertexIndexMap[vertices[2]];

		int p = subdividedMesh->vertexIndexMap[edgeVertexMap[getEdgeKey(a, b)]];
		int q = subdividedMesh->vertexIndexMap[edgeVertexMap[getEdgeKey(b, c)]];
		int r = subdividedMesh->vertexIndexMap[edgeVertexMap[getEdgeKey(c, a)]];

		subdividedMesh->insertTriangle(a, p, r);
		subdividedMesh->insertTriangle(b, q, p);
		subdividedMesh->insertTriangle(c, r, q);
		subdividedMesh->insertTriangle(p, q, r);
	}

	return subdividedMesh;
}

Mesh *Mesh::subdivideMesh (int subdivisionType, int subdivisionLevel) {
	Mesh *subdividedMesh = this;

	for (int i = 0; i < subdivisionLevel; i++) {
		subdividedMesh = subdividedMesh->loopSubdivision();
	}

	return subdividedMesh;
}