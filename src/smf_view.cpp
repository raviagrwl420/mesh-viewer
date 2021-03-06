#include <smf_parser.h>
#include <GL/glut.h>
#include <GL/glui.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

#define WIDTH 1200
#define HEIGHT 800

using std::runtime_error;
using std::shared_ptr;
using std::array;

enum DisplayType {FLAT_SHADED, SMOOTH_SHADED, WIREFRAME, SHADED_WITH_EDGES};
enum Buttons {ROTATION, OPEN, SAVE, QUIT, SUBDIVIDE, DECIMATE};

Mesh *mesh;

float xy_aspect;
int last_x, last_y;
float rotationX = 0.0, rotationY = 0.0;

// Live Variables
int main_window;
int displayType = FLAT_SHADED;
float scale = 1.0;
float view_rotate[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
float obj_pos[] = {0.0, 0.0, 0.0};

// Subdivision
int subdivisionType = BUTTERFLY;
int subdivisionLevel = 1;

// Decimation
int decimationK = 1;
int decimationNumber = 1;

// GLUT idle function
void glutIdle (void) {
	if (glutGetWindow() != main_window)
		glutSetWindow(main_window);

	glutPostRedisplay();
}

// GLUT reshape function
void glutReshape (int x, int y) {	
	xy_aspect = (float) x / (float) y;
	GLUI_Master.auto_set_viewport();

	glutPostRedisplay();
}

// Displays mesh as flat shaded
void displayFlatShaded (void) {
	glShadeModel(GL_FLAT);
	glEnable(GL_NORMALIZE);
	glBegin(GL_TRIANGLES);
	
	Face *f;
	Vertex *vertices[3];

	vec3 faceNormal;

	for (int i = 1; i <= mesh->numFaces; i++) {
		f = mesh->faceMap[i];

		faceNormal = mesh->getFaceNormal(f);
		glNormal3f(faceNormal.x, faceNormal.y, faceNormal.z);

		mesh->getAllVerticesForFace(f, vertices);

		for (int j = 0; j < 3; j++) {
			vec3 position = vertices[j]->position;
			glVertex3f(position.x, position.y, position.z);
		}
	}

	glEnd();
}

// Displays mesh as smooth shaded
void displaySmoothShaded (void) {
	glShadeModel(GL_SMOOTH);
	glEnable(GL_NORMALIZE);
	glBegin(GL_TRIANGLES);
	
	Face *f;
	Vertex *vertices[3];
	vec3 vertexNormal;

	for (int i = 1; i <= mesh->numFaces; i++) {
		f = mesh->faceMap[i];

		mesh->getAllVerticesForFace(f, vertices);

		for (int j = 0; j < 3; j++) {
			vertexNormal = vertices[j]->normal;
			glNormal3f(vertexNormal.x, vertexNormal.y, vertexNormal.z);

			vec3 position = vertices[j]->position;
			glVertex3f(position.x, position.y, position.z);
		}
	}

	glEnd();
}

// Display mesh as wireframe
void displayWireframe (void) {
	glBegin(GL_LINES);

	W_edge *edge;

	for (map<string, W_edge*>::const_iterator it = mesh->edgeMap.begin(); it != mesh->edgeMap.end(); it++) {
		edge = it->second;

		if (edge == NULL) {
			std::cout << "First!!::" << it->first << std::endl;
			std::cout << "edge:" << edge << std::endl;
			std::cout << "edge index:" << mesh->edgeIndexMap[edge] << std::endl;
			std::cout << "edge key:" << mesh->edgeKeyMap[mesh->edgeIndexMap[edge]] << std::endl;
		}
		

		vec3 startPosition = edge->start->position;
		vec3 endPosition = edge->end->position;

		glVertex3f(startPosition.x, startPosition.y, startPosition.z);
		glVertex3f(endPosition.x, endPosition.y, endPosition.z);

	}

	glEnd();
}

// Display mesh function
void displayMesh (void) {
	if (mesh == NULL)
		return;

	switch (displayType) {
		case FLAT_SHADED:
			displayFlatShaded();
			break;
		case SMOOTH_SHADED:
			displaySmoothShaded();
			break;
		case WIREFRAME:
			displayWireframe();
			break;
		case SHADED_WITH_EDGES:
			displayFlatShaded();
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(1.0, 1.0);
			displayWireframe();
			break;
	}
}

// GLUT display function
void glutDisplay (void) {
	glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-xy_aspect*0.08, xy_aspect*0.08, -0.08, 0.08, 0.1, 500.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	// Handle transformations
	glTranslatef(obj_pos[0], obj_pos[1], obj_pos[2]);
	glScalef(scale, scale, scale);
	glMultMatrixf(view_rotate);

	glColor3f(1.0, 1.0, 0.0);

	displayMesh();

	glutSwapBuffers();
}

// Execute shell commands
string exec(const char* cmd) {
    array<char, 128> buffer;
    string result;
    shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            result += buffer.data();
    }
    return result;
}

// Initialize mesh
void initMesh (string smf_filename) {
	mesh = parseSmfFile(smf_filename);
	mesh->computeBoundingBox();
	obj_pos[0] = -(mesh->xMin + mesh->xMax) / 2;
	obj_pos[1] = -(mesh->yMin + mesh->yMax) / 2;
	obj_pos[2] = 1.5 * (mesh->zMin - mesh->zMax);
}

// GLUI control callback
void control_cb(int control) {
	switch (control) {
		case OPEN: {
			string inputFilePath;
			inputFilePath = exec("zenity --file-selection --file-filter='SMF files (smf) | *.smf' --title=\"Select a SMF file\" 2>/dev/null");
			// Remove the newline character at the end
			inputFilePath = inputFilePath.substr(0, inputFilePath.size() - 1);
			if (inputFilePath.size() != 0)
				initMesh(inputFilePath);
			break;
		}

		case SAVE: {
			string saveFilePath;
			saveFilePath = exec("zenity --file-selection --save --confirm-overwrite --title=\"Save SMF file\" 2>/dev/null");
			// Remove the newline character at the end
			saveFilePath = saveFilePath.substr(0, saveFilePath.size() - 1);
			if (saveFilePath.size() != 0)
				writeSmfFile(mesh, saveFilePath);
			break;
		}
	}
};

void subdivision_cb (int control) {
	if (mesh == NULL)
		return;

	mesh = mesh->subdivideMesh(subdivisionType, subdivisionLevel);
}

void decimation_cb (int control) {
	if (mesh == NULL)
		return;

	mesh->decimate(decimationK, decimationNumber);
}

// Setup GLUI
void setupGlui () {
	// Initialize GLUI subwindow
	GLUI* glui = GLUI_Master.create_glui_subwindow(main_window, GLUI_SUBWINDOW_RIGHT);
	
	// Set main GFX window	
	glui->set_main_gfx_window(main_window);
	
	// Setup UI
	// Add Panel "Display Options"
	GLUI_Panel *displayOptionsPanel = glui->add_panel("Display Options");
	GLUI_Panel *subdivisionPanel = glui->add_panel("Subdivision");
	GLUI_Panel *decimationPanel = glui->add_panel("Decimation");
	GLUI_Panel *transformationsPanel = glui->add_panel("Transformations");
	GLUI_Panel *controlsPanel = glui->add_panel("Controls");

	// Add Listbox
	GLUI_Listbox *listbox = new GLUI_Listbox(displayOptionsPanel, "Display Type:", &displayType);
	listbox->add_item(FLAT_SHADED, "Flat Shaded");
	listbox->add_item(SMOOTH_SHADED, "Smooth Shaded");
	listbox->add_item(WIREFRAME, "Wireframe");
	listbox->add_item(SHADED_WITH_EDGES, "Shaded with Edges");

	// Subdivision
	GLUI_Listbox *subdivisionListbox = new GLUI_Listbox(subdivisionPanel, "Type:", &subdivisionType);
	subdivisionListbox->add_item(BUTTERFLY, "Butterfly");
	subdivisionListbox->add_item(LOOP, "Loop");
	subdivisionListbox->set_alignment(GLUI_ALIGN_RIGHT);

	GLUI_Spinner *subdivision_level_spinner = new GLUI_Spinner(subdivisionPanel, "Level:", &subdivisionLevel);
  	subdivision_level_spinner->set_int_limits(1, 5);
  	subdivision_level_spinner->set_alignment(GLUI_ALIGN_RIGHT);

  	glui->add_button_to_panel(subdivisionPanel, "subdivide", SUBDIVIDE, subdivision_cb);

  	// Decimation
	GLUI_Spinner *decimation_k_spinner = new GLUI_Spinner(decimationPanel, "k:", &decimationK);
	decimation_k_spinner->set_int_limits(1, 20000);
	decimation_k_spinner->set_alignment(GLUI_ALIGN_RIGHT);

	GLUI_Spinner *decimation_number_spinner = new GLUI_Spinner(decimationPanel, "Edge Count:", &decimationNumber);
	decimation_number_spinner->set_int_limits(1, 20000);
	decimation_number_spinner->set_alignment(GLUI_ALIGN_RIGHT);

	glui->add_button_to_panel(decimationPanel, "decimate", DECIMATE, decimation_cb);

	// Add Scale Spinner
	GLUI_Spinner *scale_spinner = new GLUI_Spinner(transformationsPanel, "Scale:", &scale);
  	scale_spinner->set_float_limits(0.2f, 5.0);
  	scale_spinner->set_alignment(GLUI_ALIGN_RIGHT);
	
	// Add Rotation
	GLUI_Panel *rotation_panel = glui->add_panel_to_panel(transformationsPanel, "", GLUI_PANEL_NONE);
	GLUI_Rotation *view_rot = glui->add_rotation_to_panel(rotation_panel, "Rotate", view_rotate, ROTATION, control_cb);
	view_rot->set_spin(1.0);

	// Add Translation
	GLUI_Panel *translation_panel = glui->add_panel_to_panel(transformationsPanel, "", GLUI_PANEL_NONE);
	GLUI_Translation *trans_xy = glui->add_translation_to_panel(translation_panel, "Translate XY", GLUI_TRANSLATION_XY, obj_pos);
	trans_xy->scale_factor = 0.1f;
	GLUI_Translation *trans_z = glui->add_translation_to_panel(translation_panel, "Translate Z", GLUI_TRANSLATION_Z, &obj_pos[2]);
	trans_z->scale_factor = 0.1f;

	// Add Buttons
	glui->add_button_to_panel(controlsPanel, "Open", OPEN, control_cb);
	glui->add_button_to_panel(controlsPanel, "Save", SAVE, control_cb);
	glui->add_button_to_panel(controlsPanel, "Quit", QUIT, (GLUI_Update_CB)exit);
};

int main(int argc, char* argv[]) {
	// Initialize GLUT
	glutInit(&argc, argv);
	glutInitWindowPosition(50, 50);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

	// Initialize Window
	main_window = glutCreateWindow("SMF View");

	// Register Callbacks
	glutDisplayFunc(glutDisplay);
	GLUI_Master.set_glutReshapeFunc(glutReshape);
	GLUI_Master.set_glutIdleFunc(glutIdle);
	
	// Setup Lights
	GLfloat light0_ambient[] = {0.1f, 0.1f, 0.3f, 1.0f};
	GLfloat light0_diffuse[] = {0.6f, 0.6f, 1.0f, 1.0f};
	GLfloat light0_position[] = {1.0f, 1.0f, 1.0f, 0.0f};
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);

	// Enable z-buffering
	glEnable(GL_DEPTH_TEST);

	// Setup GLUI
	setupGlui();

	// Start Main Loop
	glutMainLoop();
}
