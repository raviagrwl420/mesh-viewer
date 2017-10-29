#include <mesh.h>

// Vertex Constructor
Vertex::Vertex (float x, float y, float z) {
	this->position = vec3(x, y, z);
	this->normal = vec3(0.0, 0.0, 0.0);
}

void Mesh::computeBoundingBox () {
	vec3 position = vertexMap[1]->position;
	xMin = xMax = position.x;
	yMin = yMax = position.y;
	zMin = zMax = position.z;

	for (int i = 1; i <= numVertices; i++) {
		vec3 position = vertexMap[i]->position;
		float xVal = position.x;
		float yVal = position.y;
		float zVal = position.z;
		
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
	vec3 faceNormal = getFaceNormalVector(vertexMap[v1], vertexMap[v2], vertexMap[v3]);
	faceNormalMap.insert(make_pair(to_string(faceIndexMap[f]), faceNormal));
}

// Update the vertex normal for vertex v by adding the face normal
void Mesh::updateVertexNormalForVertex (Face *f, int v) {
	Vertex *vertex = vertexMap[v];
	vertex->normal = vertex->normal + getFaceNormal(f);
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
vec3 Mesh::getFaceNormal (Face *f) {
	map<string, vec3>::iterator it = faceNormalMap.find(to_string(faceIndexMap[f]));

	if (it != faceNormalMap.end())
		return it->second;
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
vec3 getFaceNormalVector (Vertex *v1, Vertex *v2, Vertex *v3) {
	vec3 e1 = v2->position - v1->position;
	vec3 e2 = v3->position - v1->position;

	return cross(e1, e2);
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

	vec3 newVertexPosition = edge->start->position * (3.0f/8.0f);
	newVertexPosition += edge->end->position * (3.0f/8.0f);
	newVertexPosition += left_vertex->position * (1.0f/8.0f);
	newVertexPosition += right_vertex->position * (1.0f/8.0f);

	return new Vertex(newVertexPosition.x, newVertexPosition.y, newVertexPosition.z);
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

	vec3 newVertexPosition = vertex->position * (1.0f - k*beta);

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

		newVertexPosition += nextVertex->position * beta;
	}

	return new Vertex(newVertexPosition.x, newVertexPosition.y, newVertexPosition.z);
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

// Check if a vertex is regular
bool isRegular (Vertex *vertex) {
	int degree = getDegreeOfVertex(vertex);
	return degree == 6;
}

// Compute weights for Butterfly subdivision
float computeButterflyWeight (int k, int j) {
	float weight;

	if (k == 3) {
		if (j == 0)
			weight = 5.0/12.0;
		else
			weight = -1.0/12.0;
	} else if (k == 4) {
		if (j == 0)
			weight = 3.0/8.0;
		else if (j == 2)
			weight = -1.0/8.0;
		else
			weight = 0;
	} else {
		weight = (1.0/k) * (1.0/4.0 + cos((2.0*j*PI) / k) + (1.0/2.0)*cos((4.0*j*PI) / k));
	}

	return weight;
}

// If we have an edge and a vertex, get the other vertex
Vertex *getOtherVertex(W_edge *edge, Vertex *vertex) {
	if (edge->start == vertex)
		return edge->end;
	else 
		return edge->start;
}

// Compute new vertex around irregular vertex
Vertex *computeNewVertexAroundIrregularVertex (W_edge *edge, Vertex *vertex) {
	int k = getDegreeOfVertex(vertex);
	float total_weights = 0;

	Vertex *otherVertex = getOtherVertex(edge, vertex);
	float s0weight = computeButterflyWeight(k, 0);
	total_weights += s0weight;

	vec3 newVertexPosition = otherVertex->position * s0weight;

	Vertex *nextVertex;
	W_edge *e = edge;
	for (int i = 1; i < k; i++) {
		float s = computeButterflyWeight(k, i);
		total_weights += s;

		if (e->start == vertex) {
			nextVertex = getNextVertex(e, e->left_prev);
			e = e->left_prev;
		} else {
			nextVertex = getNextVertex(e, e->right_next);
			e = e->right_next;
		}

		newVertexPosition += nextVertex->position * s;
	}

	newVertexPosition += vertex->position * (1 - total_weights);

	return new Vertex(newVertexPosition.x, newVertexPosition.y, newVertexPosition.z);
}

// Helper method to get a butterfly vertex
Vertex *getButterflyVertex (W_edge *edge, W_edge *next_edge) {
	Vertex *nextVertex = getNextVertex(edge, next_edge);

	if (edge->left == next_edge->left || edge->right == next_edge->left)
		return getNextVertex(next_edge, next_edge->right_next);
	else if (edge->left == next_edge->right || edge->right == next_edge->right)
		return getNextVertex(next_edge, next_edge->left_next);
}

// Helper method to get all butterfly vertices
void getButterflyVertices (W_edge *edge, Vertex *vertices[4]) {
	vertices[0] = getButterflyVertex(edge, edge->left_prev);
	vertices[1] = getButterflyVertex(edge, edge->left_next);
	vertices[2] = getButterflyVertex(edge, edge->right_prev);
	vertices[3] = getButterflyVertex(edge, edge->right_next);
}

// Compute new vertex for an edge
Vertex *computeMidpointButterfly (W_edge *edge) {
	bool isStartVertexRegular = isRegular(edge->start);
	bool isEndVertexRegular = isRegular(edge->end);

	vec3 newVertexPosition;
	if (isStartVertexRegular && isEndVertexRegular) {
		Vertex *left_vertex = getNextVertex(edge, edge->left_next);
		Vertex *right_vertex = getNextVertex(edge, edge->right_next);
		Vertex *butterflyVertices[4];

		getButterflyVertices(edge, butterflyVertices);

		newVertexPosition += edge->start->position * (1.0f/2.0f);
		newVertexPosition += edge->end->position * (1.0f/2.0f);
		newVertexPosition += left_vertex->position * (1.0f/8.0f);
		newVertexPosition += right_vertex->position * (1.0f/8.0f);

		for (int i = 0; i < 4; i++) {
			newVertexPosition += butterflyVertices[i]->position * (-1.0f/16.0f);
		}
	} else {
		if (isStartVertexRegular)
			return computeNewVertexAroundIrregularVertex(edge, edge->end);
		else if (isEndVertexRegular)
			return computeNewVertexAroundIrregularVertex(edge, edge->start);
		else {
			Vertex *v1 = computeNewVertexAroundIrregularVertex(edge, edge->start);
			Vertex *v2 = computeNewVertexAroundIrregularVertex(edge, edge->end);
			newVertexPosition = v1->position * (1.0f/2.0f) + v2->position * (1.0f/2.0f);
		}
	}

	return new Vertex(newVertexPosition.x, newVertexPosition.y, newVertexPosition.z);
}

// Perform Butterfly subdivision
Mesh *Mesh::butterflySubdivision () {
	Mesh *subdividedMesh = new Mesh();
	map<string, Vertex*> edgeVertexMap;
	Vertex *newVertex;

	// Add existing vertices to subdividedMesh
	for (int i = 1; i <= numVertices; i++) {
		subdividedMesh->insertVertex(vertexMap[i]);
	}

	// Compute new mid point vertices for each edge
	W_edge *edge;
	for (map<string, W_edge*>::const_iterator it = edgeMap.begin(); it != edgeMap.end(); it++) {
		edge = it->second;

		int startIndex = vertexIndexMap[edge->start];
		int endIndex = vertexIndexMap[edge->end];

		newVertex = computeMidpointButterfly(edge);
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

// Subdivide a mesh
Mesh *Mesh::subdivideMesh (int subdivisionType, int subdivisionLevel) {
	Mesh *subdividedMesh = this;

	for (int i = 0; i < subdivisionLevel; i++) {
		if (subdivisionType == LOOP)
			subdividedMesh = subdividedMesh->loopSubdivision();
		else if (subdivisionType == BUTTERFLY)
			subdividedMesh = subdividedMesh->butterflySubdivision();
	}

	return subdividedMesh;
}