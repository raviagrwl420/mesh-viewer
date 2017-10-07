#include <mesh.h>

// Vertex Constructor
Vertex::Vertex (float x, float y, float z) {
	this->x = x;
	this->y = y;
	this->z = z;
	this->vector = new Vector(x, y, z);
	this->normal = new Vector(0.0, 0.0, 0.0);
};

// Insert a vertex in the mesh
Vertex *Mesh::insertVertex (float x, float y, float z) {
	Vertex *vertex = new Vertex(x, y, z);

	int index = vertexMap.size() + 1;

	vertexMap.insert(make_pair(index, vertex));
	vertexIndexMap.insert(make_pair(vertex, index));

	return vertex;
};

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
	W_edge *edge1 = f->edge;
	
	W_edge *e = edge1;
	int count = 0;
	do {
		edges[count] = e;

		if (e->right == f)
			e = e->right_prev;
		else 
			e = e->left_next;

		count++;
	} while (e != edge1);
}

// Get all vertices for a face
void Mesh::getAllVerticesForFace (Face *f, Vertex *vertices[3]) {
	W_edge *edge1 = f->edge;

	if (edge1->right == f) {
		vertices[0] = edge1->end;
		vertices[1] = edge1->start;

		W_edge *edge2 = edge1->right_next;
		if (edge1->end == edge2->start)
			vertices[2] = edge2->end;
		else
			vertices[2] = edge2->start;
	} else {
		vertices[0] = edge1->start;
		vertices[1] = edge1->end;

		W_edge *edge2 = edge1->left_next;
		if (edge1->end == edge2->start)
			vertices[2] = edge2->end;
		else
			vertices[2] = edge2->start;
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