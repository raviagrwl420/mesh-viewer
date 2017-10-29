#include <mesh.h>

// Vertex Constructor
Vertex::Vertex (float x, float y, float z) {
	this->position = vec3(x, y, z);
	this->normal = vec3(0.0, 0.0, 0.0);
	this->quadric = mat4(0.0);
}

// Vertex Constructor
Vertex::Vertex (vec3 position) {
	this->position = position;
	this->normal = vec3(0.0, 0.0, 0.0);
	this->quadric = mat4(0.0);
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

// Delete a vertex from the mesh
void Mesh::deleteVertex (Vertex *vertex) {
	int index = vertexIndexMap[vertex];
	vertexIndexMap.erase(vertex);

	if (index == numVertices) {
		vertexMap.erase(numVertices);
	} else {
		Vertex *last = vertexMap[numVertices];
		vertexMap.erase(numVertices);

		vertexMap[index] = last;
		vertexIndexMap[last] = index;	
	}
	
	numVertices--;
}

// Insert an edge in the mesh
W_edge *Mesh::insertEdge (int v1, int v2) {
	W_edge *edge = new W_edge(vertexMap[v1], vertexMap[v2]);

	int index = edgeMap.size() + 1;

	string edgeKey = getEdgeKey(v1, v2);
	edgeMap.insert(make_pair(edgeKey, edge));
	edgeKeyMap.insert(make_pair(index, edgeKey));
	edgeIndexMap.insert(make_pair(edge, index));

	numEdges = edgeMap.size();
	
	return edge;
}

// Delete an edge from the mesh
void Mesh::deleteEdge (W_edge *edge) {
	int index = edgeIndexMap[edge];
	edgeIndexMap.erase(edge);

	string edgeKey = edgeKeyMap[index];

	edgeMap.erase(edgeKey);

	if (index == numEdges) {
		edgeKeyMap.erase(numEdges);
	} else {
		string lastEdgeKey = edgeKeyMap[numEdges];
		edgeKeyMap.erase(numEdges);

		edgeKeyMap[index] = lastEdgeKey;
		edgeIndexMap[edgeMap[lastEdgeKey]] = index;	
	}
	
	numEdges--;
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

// Delete a face from the mesh
void Mesh::deleteFace (Face *face) {
	int index = faceIndexMap[face];
	faceIndexMap.erase(face);

	if (index == numFaces) {
		faceMap.erase(numFaces);
		faceNormalMap.erase(to_string(numFaces));
	} else {
		Face *last = faceMap[numFaces];
		faceMap.erase(numFaces);

		vec3 lastNormal = faceNormalMap[to_string(numFaces)];
		faceNormalMap.erase(to_string(numFaces));

		faceMap[index] = last;
		faceNormalMap[to_string(index)] = lastNormal;
		faceIndexMap[last] = index;	
	}

	numFaces--;	
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

	updateQuadricForEachVertex(f, v1, v2, v3);
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

	return normalize(cross(e1, e2));
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

	return new Vertex(newVertexPosition);
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

	return new Vertex(newVertexPosition);
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

	return new Vertex(newVertexPosition);
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

		newVertexPosition = edge->start->position * (1.0f/2.0f);
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

	return new Vertex(newVertexPosition);
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

// Decimate a mesh

mat4 Mesh::getQuadric (Face *f, Vertex *v) {
	vec3 faceNormal = getFaceNormal(f);
	vec3 vertexPosition = v->position;

	float d = -dot(faceNormal, vertexPosition);

	vec4 p = vec4(faceNormal, d);

	return outerProduct(p, p);
}

void Mesh::updateQuadricForVertex (Face *f, int v1) {
	Vertex *vertex = vertexMap[v1];
	vertex->quadric += getQuadric(f, vertex);
}

void Mesh::updateQuadricForEachVertex (Face *f, int v1, int v2, int v3) {
	updateQuadricForVertex(f, v1);
	updateQuadricForVertex(f, v2);
	updateQuadricForVertex(f, v3);
}

vec4 Mesh::computeNewVertexPositionForEdgeCollapse (W_edge *edge) {
	mat4 quadric = edge->start->quadric + edge->end->quadric;

	quadric[0][3] = 0;
	quadric[1][3] = 0;
	quadric[2][3] = 0;
	quadric[3][3] = 1;

	vec4 vec = vec4(0.0f, 0.0f, 0.0f, 1.0f);

	return inverse(quadric) * vec;
}

float getError (W_edge *edge, vec4 p) {
	mat4 Q = edge->start->quadric + edge->end->quadric;
	return dot(p*Q, p); 	
}

W_edge *Mesh::getCandidateEdgeToCollapse (int k) {
	std::random_device rd;  //Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
	std::uniform_int_distribution<> dis(1, numEdges);

	float minError = FLT_MAX;
	W_edge *candidateEdge;

	for(int i = 0; i < k; i++) {
		int edgeIndex = dis(gen);

		string edgeKey = edgeKeyMap[edgeIndex];
		W_edge *edge = edgeMap[edgeKey];

		vec4 newVertexPosition = computeNewVertexPositionForEdgeCollapse(edge);

		float error = getError(edge, newVertexPosition);

		if (error < minError) {
			minError = error;
			candidateEdge = edge;
		}
	}

	return candidateEdge;
}

bool Mesh::canCauseFoldOver (W_edge *edge) {
	vec3 newVertexPosition(computeNewVertexPositionForEdgeCollapse(edge));
	Vertex *newVertex = new Vertex(newVertexPosition);

	W_edge *e = edge->left_next;

	while (e != edge->right_next) {
		W_edge *other;
		if (e->start == edge->end) {
			other = e->right_next;
			e = e->right_prev;
		} else {
			other = e->left_prev;
			e = e->left_next;
		}

		vec3 oldNormal = getFaceNormalVector(edge->end, other->start, other->end);
		vec3 newNormal = getFaceNormalVector(newVertex, other->start, other->end);

		if (dot(oldNormal, newNormal) < 0) {
			return true;
		}
	}

	e = edge->right_prev;

	while (e != edge->left_prev) {
		W_edge *other;
		if (e->start == edge->start) {
			other = e->right_next;
			e = e->right_prev;
		} else {
			other = e->left_prev;
			e = e->left_next;
		}

		vec3 oldNormal = getFaceNormalVector(edge->start, other->start, other->end);
		vec3 newNormal = getFaceNormalVector(newVertex, other->start, other->end);

		if (dot(oldNormal, newNormal) < 0) {
			return true;
		}
	}

	return false;
}

bool Mesh::canCauseNonManifoldMesh (W_edge *edge) {
	Vertex *leftOther = getNextVertex(edge, edge->left_next);
	Vertex *rightOther = getNextVertex(edge, edge->right_next);
	return getDegreeOfVertex(edge->start) == 3 || getDegreeOfVertex(edge->end) == 3 ||
		getDegreeOfVertex(leftOther) == 3 || getDegreeOfVertex(rightOther) == 3;
}

void Mesh::collapseEdge (W_edge *edge) {
	vec3 newVertexPosition(computeNewVertexPositionForEdgeCollapse(edge));

	Vertex *startVertex = edge->start;
	startVertex->position = newVertexPosition;
	startVertex->quadric += edge->end->quadric;

	W_edge *leftNext = edge->left_next;
	W_edge *rightNext = edge->right_next;
	W_edge *leftPrev = edge->left_prev;
	W_edge *rightPrev = edge->right_prev;

	// Update vertex references
	W_edge *e = leftNext;
	while (e != rightNext) {
		if (e->start == edge->end || e->start == edge->start) {
			e = e->right_prev;
		} else {
			e = e->left_next;
		}

		if (e->start == edge->end) {
			e->start = edge->start;
		} else {
			e->end = edge->start;
		}
	}

	// Update edge references
	if (leftNext->left == edge->left) {
		if (leftPrev->left == edge->left) {
			leftPrev->left_next = leftNext->right_prev;
			leftPrev->left_prev = leftNext->right_next;
		} else {
			leftPrev->right_prev = leftNext->right_prev;
			leftPrev->right_next = leftNext->right_next;
		}

		if (leftNext->right_prev->right_next == leftNext) {
			leftNext->right_prev->right_next = leftPrev;
		} else {
			leftNext->right_prev->left_prev = leftPrev;
		}

		if (leftNext->right_next->right_prev == leftNext) {
			leftNext->right_next->right_prev = leftPrev;
		} else {
			leftNext->right_next->left_next = leftPrev;
		}
	} else {
		if (leftPrev->left == edge->left) {
			leftPrev->left_next = leftNext->left_next;
			leftPrev->left_prev = leftNext->left_prev;
		} else {
			leftPrev->right_prev = leftNext->left_next;
			leftPrev->right_next = leftNext->left_prev;
		}

		if (leftNext->left_next->left_prev == leftNext) {
			leftNext->left_next->left_prev = leftPrev;
		} else {
			leftNext->left_next->right_next = leftPrev;
		}

		if (leftNext->left_prev->left_next == leftNext) {
			leftNext->left_prev->left_next = leftPrev;
		} else {
			leftNext->left_prev->right_prev = leftPrev;
		}
	}

	if (rightNext->left == edge->right) {
		if (rightPrev->left == edge->right) {
			rightPrev->left_next = rightNext->right_prev;
			rightPrev->left_prev = rightNext->right_next;
		} else {
			rightPrev->right_prev = rightNext->right_prev;
			rightPrev->right_next = rightNext->right_next;
		}

		if (rightNext->right_prev->right_next == rightNext) {
			rightNext->right_prev->right_next = rightPrev;
		} else {
			rightNext->right_prev->left_prev = rightPrev;
		}

		if (rightNext->right_next->right_prev == rightNext) {
			rightNext->right_next->right_prev = rightPrev;
		} else {
			rightNext->right_next->left_next = rightPrev;
		}
	} else {
		if (rightPrev->left == edge->right) {
			rightPrev->left_next = rightNext->left_next;
			rightPrev->left_prev = rightNext->left_prev;
		} else {
			rightPrev->right_prev = rightNext->left_next;
			rightPrev->right_next = rightNext->left_prev;
		}

		if (rightNext->left_next->left_prev == rightNext) {
			rightNext->left_next->left_prev = rightPrev;
		} else {
			rightNext->left_next->right_next = rightPrev;
		}

		if (rightNext->left_prev->left_next == rightNext) {
			rightNext->left_prev->left_next = rightPrev;
		} else {
			rightNext->left_prev->right_prev = rightPrev;
		}
	}

	// Update face and edge references
	if (leftNext->left == edge->left) {
		if (leftPrev->left == edge->left) {
			leftPrev->left = leftNext->right;
		} else {
			leftPrev->right = leftNext->right;
		}
		leftNext->right->edge = leftPrev;
	} else {
		if (leftPrev->left == edge->left) {
			leftPrev->left = leftNext->left;
		} else {
			leftPrev->right = leftNext->left;
		}
		leftNext->left->edge = leftPrev;
	}

	if (rightNext->left == edge->right) {
		if (rightPrev->left == edge->right) {
			rightPrev->left = rightNext->right;
		} else {
			rightPrev->right = rightNext->right;
		}
		rightNext->right->edge = rightPrev;
	} else {
		if (rightPrev->left == edge->right) {
			rightPrev->left = rightNext->left;
		} else {
			rightPrev->right = rightNext->left;
		}
		rightNext->left->edge = rightPrev;
	}

	// Update edge reference for start vertex
	edge->start->edge = leftPrev;
	if (leftNext->left == edge->left) {
		leftNext->end->edge = leftPrev;
	} else {
		leftNext->start->edge = leftPrev;
	}

	if (rightNext->left == edge->right) {
		rightNext->start->edge = rightPrev;
	} else {
		rightNext->end->edge = rightPrev;
	}

	deleteVertex(edge->end);
	deleteFace(edge->left);
	deleteFace(edge->right);
	deleteEdge(edge);
	deleteEdge(leftNext);
	deleteEdge(rightNext);
}

void Mesh::decimate (int k, int n) {
	for(int i = 0; i < n; i++) {
		W_edge *edge = getCandidateEdgeToCollapse(k);

		while (canCauseNonManifoldMesh(edge) || canCauseFoldOver(edge)) {
			edge = getCandidateEdgeToCollapse(k);
		}

		collapseEdge(edge);
	}
}