#include <smf_parser.h>

Mesh *mesh;

// Read file and parse data
void parseSmfFile(string filename) {
	ifstream smf_file;
	smf_file.open(filename.c_str());

	if (smf_file.fail()) {
		cerr << "Error opening file." << endl << "Exiting..." << endl;
		exit(1);
	}

	int numVertices, numFaces;

	char type;
	int v = 1, f = 1;

	// Read first line for number of vertices and faces
	smf_file >> type >> numVertices >> numFaces;

	mesh = new Mesh(numVertices, numFaces);

	// Read Data
	while (smf_file >> type) {
		float x, y, z;
		int v1, v2, v3;
		string line;
		
		switch (type) {
			case '#': {
				// Ignore comments
				getline(smf_file, line); 
				break;
			}

			case 'v': {
				smf_file >> x >> y >> z;
				mesh->insertVertex(x, y, z);
				v++;
				break;
			}

			case 'f': {
				smf_file >> v1 >> v2 >> v3;
				mesh->insertTriangle(v1, v2, v3);
				f++;
				break;
			}
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

void displaySMF(string smf_filename) {
	parseSmfFile(smf_filename);
}
