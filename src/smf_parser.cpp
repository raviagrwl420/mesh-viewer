#include <smf_parser.h>

// Global Variables
int numVertices, numFaces;
map<string, W_edge*> edgeMap;
map<int, Vertex*> vertexMap;
map<int, Face*> faceMap;
map<Vertex*, int> vertexIndexMap;
map<string, Vector*> faceNormalMap;
map<Vertex*, Vector*> vertexNormalMap;

// Compute Edge Key
string getEdgeKey (int v1, int v2) {
	if (v1 < v2)
		return to_string(v1) + "|" + to_string(v2);
	else
		return to_string(v2) + "|" + to_string(v1);
}

string getFaceNormalKey (int faceId, Vertex *v) {
	return to_string(faceId) + "|" + to_string(v->x) + "|" + to_string(v->y) + "|" + to_string(v->z); 
}

Vector *getVertexNormal(Vertex *v) {
	map<Vertex*, Vector*>::iterator it = vertexNormalMap.find(v);

	if (it != vertexNormalMap.end())
		return it->second;
	else
		return NULL;
}

// Check If Edge Exists
W_edge *getEdge (int v1, int v2) {	
	map<string, W_edge*>::iterator it = edgeMap.find(getEdgeKey(v1, v2));
	
	if (it != edgeMap.end())
		return it->second;
	else
		return NULL;
}

// Create And Insert An Edge Into The Map
W_edge *createEdgeAndInsert(int v1, int v2) {
	W_edge *edge;
	edge = new W_edge(vertexMap[v1], vertexMap[v2]);

	edgeMap.insert(make_pair(getEdgeKey(v1, v2), edge));
	
	return edge;
}

// Create And Insert A Face Into The Map
Face *createFaceAndInsert(W_edge *edge, int count) {
	Face *face;
	face = new Face(edge);
	
	faceMap.insert(make_pair(count, face));

	return face;
}

// Consume A Face And Create Edges And Faces With Appropriate References
void consumeFace (int v1, int v2, int v3, int count) {
	W_edge *e1, *e2, *e3;
	Face *f;
	int e1Dir, e2Dir, e3Dir;
	
	e1 = getEdge(v1, v2);

	if (e1 == NULL) {
		e1 = createEdgeAndInsert(v1, v2);
		e1Dir = 1;
		
		f = createFaceAndInsert(e1, count); 
		e1->left = f;
		
		vertexMap[v1]->edge = e1;
	} else {
		e1Dir = -1;
		f = createFaceAndInsert(e1, count); 
		e1->right = f;		
	}

	e2 = getEdge(v2, v3);

	if (e2 == NULL) {
		e2 = createEdgeAndInsert(v2, v3);
		e2Dir = 1;

		e2->left = f;
		
		vertexMap[v2]->edge = e2;
	} else {
		e2Dir = -1;
		e2->right = f;
	}
	
	e3 = getEdge(v3, v1);

	if (e3 == NULL) {
		e3 = createEdgeAndInsert(v3, v1);
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
}

Vector *faceNormalFromVertexUnnormalized(Face *f, Vertex *v1, Vertex *v2, Vertex *v3) {
	float x = ((v2->y - v1->y)*(v3->z - v1->z)) - ((v2->z - v1->z)*(v3->y - v1->y));
	float y = ((v3->x - v1->x)*(v2->z - v1->z)) - ((v3->z - v1->z)*(v2->x - v1->x));
	float z = ((v2->x - v1->x)*(v3->y - v1->y)) - ((v2->y - v1->y)*(v3->x - v1->x));

	return new Vector(x, y, z);
}

void updateVertexNormal(int faceId, Vertex *v1, Vertex *v2, Vertex *v3) {
	Vector *vertexNormal;
	Vector *faceNormal;
	map<Vertex*, Vector*>::iterator it = vertexNormalMap.find(v1);

	if (it != vertexNormalMap.end()) {
		vertexNormal = it->second;
	} else {
		vertexNormal = new Vector(0.0, 0.0, 0.0);
		vertexNormalMap.insert(make_pair(v1, vertexNormal));
	}

	map<string, Vector*>::iterator it2 = faceNormalMap.find(getFaceNormalKey(faceId, v1));

	if (it2 != faceNormalMap.end()) {
		faceNormal = it2->second;
		float normalLength = sqrtf((faceNormal->x * faceNormal->x) + (faceNormal->y * faceNormal->y) + (faceNormal->z * faceNormal->z));

		float vector1X = v2->x - v1->x;
		float vector1Y = v2->y - v1->y;
		float vector1Z = v2->z - v1->z;
		
		float vector2X = v3->x - v1->x;
		float vector2Y = v3->y - v1->y;
		float vector2Z = v3->z - v1->z;
		
		float vector1Length = sqrtf((vector1X * vector1X) + (vector1Y * vector1Y) + (vector1Z * vector1Z));
		float vector2Length = sqrtf((vector2X * vector2X) + (vector2Y * vector2Y) + (vector2Z * vector2Z));

		float sin_alpha = normalLength/(vector1Length*vector2Length);

		float normalX = faceNormal->x/normalLength;
		float normalY = faceNormal->y/normalLength;
		float normalZ = faceNormal->z/normalLength;

		normalX = normalX * asin(sin_alpha);
		normalY = normalY * asin(sin_alpha);
		normalZ = normalZ * asin(sin_alpha);

		//updating the entry in vertexNormal
		vertexNormal->x = vertexNormal->x + normalX;
		vertexNormal->y = vertexNormal->y + normalY;
		vertexNormal->z = vertexNormal->z + normalZ;
	}
}

void getAllEdgesForFace(Face *f, W_edge *edges[3]) {
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

void getAllVerticesForFace(Face *f, Vertex *vertices[3]) {
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

// Read file and parse data
void parseSmfFile(string filename) {
	ifstream smf_file;
	smf_file.open(filename.c_str());

	if (smf_file.fail()) {
		cerr << "Error opening file." << endl << "Exiting..." << endl;
		exit(1);
	}

	char type;
	int v = 1, f = 1;

	// Read Data
	while (smf_file >> type) {
		float x, y, z;
		int v1, v2, v3;
		string line;
		
		switch (type) {
			case '#':
				if (!numVertices && !numFaces)
					smf_file >> numVertices >> numFaces;
				else 
					getline(smf_file, line);
				break;
			case 'v':
				smf_file >> x >> y >> z;
				vertexMap.insert(make_pair(v, new Vertex(x, y, z)));
				v++;
				break;
			case 'f':
				smf_file >> v1 >> v2 >> v3;
				consumeFace(v1, v2, v3, f);

				// Normal Stuff
				Vector *vec1, *vec2, *vec3;

				vec1 = faceNormalFromVertexUnnormalized(faceMap[f], vertexMap[v1], vertexMap[v2], vertexMap[v3]);
				vec2 = faceNormalFromVertexUnnormalized(faceMap[f], vertexMap[v2], vertexMap[v3], vertexMap[v1]);
				vec3 = faceNormalFromVertexUnnormalized(faceMap[f], vertexMap[v3], vertexMap[v1], vertexMap[v2]);

				faceNormalMap.insert(make_pair(getFaceNormalKey(f, vertexMap[v1]), vec1));
				faceNormalMap.insert(make_pair(getFaceNormalKey(f, vertexMap[v2]), vec2));
				faceNormalMap.insert(make_pair(getFaceNormalKey(f, vertexMap[v3]), vec3));

				f++;
				break;
		}
	}

	for(int i = 1; i <= numFaces; i++) {
		Vertex *vertices[3];
		getAllVerticesForFace(faceMap[i], vertices);
		updateVertexNormal(i, vertices[0], vertices[1], vertices[2]);
		updateVertexNormal(i, vertices[1], vertices[2], vertices[0]);
		updateVertexNormal(i, vertices[2], vertices[0], vertices[1]);
		// W_edge *faceEdgeArray[3];
		// getAllEdgesForFace(faceMap[i], faceEdgeArray);

		// W_edge *edge1 = faceEdgeArray[0];
	
		// W_edge *edge2 = faceEdgeArray[1];

		// if(edge1->start == edge2->start || edge1->end == edge2->start)
		// {
		// 	updateVertexNormal(i, edge1->start, edge1->end, edge2->end);
		// 	updateVertexNormal(i, edge1->end, edge2->end, edge1->start);
		// 	updateVertexNormal(i, edge2->end, edge1->start, edge1->end);
		// }
		// else
		// {	
		// 	updateVertexNormal(i, edge1->start, edge1->end, edge2->start);
		// 	updateVertexNormal(i, edge1->end, edge2->start, edge1->start);
		// 	updateVertexNormal(i, edge2->start, edge1->start, edge1->end);
		// }
	};
	
	smf_file.close();
}

void writeSmfFile(string filename) {
	ofstream smf_file;
	smf_file.open(filename.c_str());
	
	if (smf_file.fail()) {
		cerr << "Error opening file." << endl << "Exiting..." << endl;
		exit(1);
	}

	smf_file << "# " << numVertices << " " << numFaces << endl;
	
	for (int i = 1; i <= numVertices; i++) {
		Vertex *vertex = vertexMap[i];
		vertexIndexMap.insert(make_pair(vertex, i));

		smf_file << "v " << vertex->x << " " << vertex->y << " " << vertex->z << endl;
	}

	for (int i = 1; i <= numFaces; i++) {
		// TODO: Add Faces to SMF	
	}
	
	smf_file.close();
}

void displaySMF(string smf_filename) {
	parseSmfFile(smf_filename);
}