#include <smf_parser.h>

// Global Variables
int numVertices, numFaces;
map<string, W_edge*> edgeMap;
map<int, Vertex*> vertexMap;
map<int, Face*> faceMap;
map<Vertex*, int> vertexIndexMap;

// Compute Edge Key
string getEdgeKey (int v1, int v2) {
	if (v1 < v2)
		return to_string(v1) + "|" + to_string(v2);
	else
		return to_string(v2) + "|" + to_string(v1);
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
				f++;
				break;
		}
	}
	
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
