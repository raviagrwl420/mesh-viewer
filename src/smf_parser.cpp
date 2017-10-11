#include <smf_parser.h>

// Read SMF file and parse data
Mesh *parseSmfFile (string filename) {
	ifstream smf_file;
	smf_file.open(filename.c_str());

	if (smf_file.fail()) {
		cerr << "Error opening file." << endl << "Exiting..." << endl;
		exit(1);
	}

	char type;
	int numVertices, numFaces;

	smf_file >> type >> numVertices >> numFaces; // Read first line for number of vertices and faces

	Mesh *mesh = new Mesh(numVertices, numFaces); // Initialize Mesh

	// Read remaining data
	while (smf_file >> type) {
		string line;
		float x, y, z;
		int v1, v2, v3;

		switch (type) {
			case '#':
				getline(smf_file, line); // Ignore comments starting with '#'
				break;
			case 'v':
				smf_file >> x >> y >> z;
				mesh->insertVertex(x, y, z);
				break;
			case 'f':
				smf_file >> v1 >> v2 >> v3;
				mesh->insertTriangle(v1, v2, v3);
				break;
		}
	}

	smf_file.close();

	return mesh;
}

void writeSmfFile (Mesh *mesh, string filename) {
	ofstream smf_file;
	smf_file.open(filename.c_str());
	
	if (smf_file.fail()) {
		cerr << "Error opening file." << endl << "Exiting..." << endl;
		exit(1);
	}

	smf_file << "# " << mesh->numVertices << " " << mesh->numFaces << endl;
	
	for (int i = 1; i <= mesh->numVertices; i++) {
		Vertex *vertex = mesh->vertexMap[i];
		smf_file << "v " << vertex->x << " " << vertex->y << " " << vertex->z << endl;
	}

	for (int i = 1; i <= mesh->numFaces; i++) {
		Face *face = mesh->faceMap[i];
		Vertex *vertices[3];
		mesh->getAllVerticesForFace(face, vertices);
		smf_file << "f " << mesh->vertexIndexMap[vertices[0]] << " " << mesh->vertexIndexMap[vertices[1]] << " " << mesh->vertexIndexMap[vertices[2]] << endl;
	}
	
	smf_file.close();
}