#include <smf_parser.h>
#include <GL/glut.h>
#include <GL/glui.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

using std::runtime_error;
using std::shared_ptr;
using std::array;

#define WIDTH 800
#define HEIGHT 600

enum DisplayType {FLAT_SHADED, SMOOTH_SHADED, WIREFRAME, SHADED_WITH_EDGES};
enum Buttons {ROTATION, OPEN, SAVE, QUIT, RESET};

float xy_aspect;
int last_x, last_y;
float rotationX = 0.0, rotationY = 0.0;

// Live Variables
int main_window;
float scale = 1.0;
int displayType = FLAT_SHADED;
int wireframe = 0;
int segments = 8;
float view_rotate[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
float obj_pos[] = {0.0, 0.0, 0.0};

void myGlutIdle(void) {
	if (glutGetWindow() != main_window)
		glutSetWindow(main_window);
	glutPostRedisplay();
}

void myGlutReshape(int x, int y) {	
	xy_aspect = (float) x / (float) y;
	GLUI_Master.auto_set_viewport();
	
	// glMatrixMode(GL_PROJECTION);
	// glLoadIdentity();
	// glFrustum(-xy_aspect*0.08, xy_aspect*0.08, -0.08, 0.08, 0.1, 15.0);

	glutPostRedisplay();
}


void myGlutDisplay(void) {
	glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-xy_aspect*0.08, xy_aspect*0.08, -0.08, 0.08, 0.1, 15.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glTranslatef(obj_pos[0], obj_pos[1], obj_pos[2]);
	glScalef(scale, scale, scale);
	glMultMatrixf(view_rotate);

	glColor3f(1.0, 1.0, 0.0);

	if (displayType == FLAT_SHADED || displayType == SHADED_WITH_EDGES) {
		glShadeModel(GL_FLAT);
		glEnable(GL_NORMALIZE);
		glBegin(GL_TRIANGLES);
		
		if (mesh != NULL) {
			Vertex* vertices[3];
			for (int i = 1; i <= mesh->numFaces; i++) {
				Face* f = mesh->faceMap[i];
				mesh->getAllVerticesForFace(f, vertices);

				Vector *faceNormal = mesh->getFaceNormal(f);

				glNormal3f(faceNormal->x, faceNormal->y, faceNormal->z);

				glVertex3f(vertices[0]->x, vertices[0]->y, vertices[0]->z);
				glVertex3f(vertices[1]->x, vertices[1]->y, vertices[1]->z);
				glVertex3f(vertices[2]->x, vertices[2]->y, vertices[2]->z);
			}	
		}

		glEnd();
	}

	if (displayType == SMOOTH_SHADED) {
		glShadeModel(GL_SMOOTH);
		glBegin(GL_TRIANGLES);
		
		Vertex* vertices[3];
		for (int i = 1; i <= mesh->numFaces; i++) {
			Face* f = mesh->faceMap[i];
			mesh->getAllVerticesForFace(f, vertices);

			for (int j = 0; j < 3; j++) {
				Vector *normal = (vertices[j]->normal)->normalize();
				glNormal3f(normal->x, normal->y, normal->z);
				glVertex3f(vertices[j]->x, vertices[j]->y, vertices[j]->z);
			}
		}

		glEnd();
	}

	if (displayType == WIREFRAME) {
		glBegin(GL_LINES);
		for (map<string, W_edge*>::const_iterator it = mesh->edgeMap.begin(); it != mesh->edgeMap.end(); it++) {
			W_edge *edge = it->second;
			glVertex3f(edge->start->x, edge->start->y, edge->start->z);
			glVertex3f(edge->end->x, edge->end->y, edge->end->z);
		}
		glEnd();
	}

	if(displayType == SHADED_WITH_EDGES)
	{
		glPolygonOffset(1.0, 1.0);
		glBegin(GL_LINES);
		for (map<string, W_edge*>::const_iterator it = mesh->edgeMap.begin(); it != mesh->edgeMap.end(); it++) {
			W_edge *edge = it->second;
			glVertex3f(edge->start->x, edge->start->y, edge->start->z);
			glVertex3f(edge->end->x, edge->end->y, edge->end->z);
		}
		glEnd();
	}

	glutSwapBuffers();
}

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

// GLUI Control Callback
void control_cb(int control) {
	switch (control) {
		case OPEN: {
			string inputFilePath;
			inputFilePath = exec("zenity --file-selection --file-filter='SMF files (smf) | *.smf' --title=\"Select a SMF file\" 2>/dev/null");
			// Remove the newline character at the end
			inputFilePath = inputFilePath.substr(0, inputFilePath.size() - 1);
			displaySMF(inputFilePath);
			break;
		}

		case SAVE: {
			string saveFilePath;
			saveFilePath = exec("zenity --file-selection --save --confirm-overwrite --title=\"Save SMF file\" 2>/dev/null");
			// Remove the newline character at the end
			saveFilePath = saveFilePath.substr(0, saveFilePath.size() - 1);
			if (saveFilePath.size() != 0)
				writeSmfFile(saveFilePath);
			break;
		}

		case RESET: {
			// Todo: Reset!!
		}
	}
};

void pickDisplayType(int displayType) {

}

// Setup GLUI
void setupGlui () {
	// Initialize GLUI Subwindow
	GLUI* glui = GLUI_Master.create_glui_subwindow(main_window, GLUI_SUBWINDOW_RIGHT);
	
	// Set Main GFX Window	
	glui->set_main_gfx_window(main_window);
	
	// Setup UI
	
	// Add Heading
	new GLUI_StaticText(glui, "SMF Viewer");
	glui->add_separator();

	// Add Panel "Display Options"
	GLUI_Panel *displayOptionsPanel = glui->add_panel("Display Options");

	// Add Listbox
	GLUI_Listbox *listbox = new GLUI_Listbox(displayOptionsPanel, "Display Type:", &displayType);
	listbox->add_item(FLAT_SHADED, "Flat Shaded");
	listbox->add_item(SMOOTH_SHADED, "Smooth Shaded");
	listbox->add_item(WIREFRAME, "Wireframe");
	listbox->add_item(SHADED_WITH_EDGES, "Shaded with Edges");

	// Add Scale Spinner
	GLUI_Spinner *scale_spinner = new GLUI_Spinner(displayOptionsPanel, "Scale:", &scale);
  	scale_spinner->set_float_limits(0.2f, 5.0);
  	scale_spinner->set_alignment(GLUI_ALIGN_RIGHT);
	
	// Add Rotation
	GLUI_Panel *rotation_panel = glui->add_panel_to_panel(displayOptionsPanel, "", GLUI_PANEL_NONE);
	GLUI_Rotation *view_rot = glui->add_rotation_to_panel(rotation_panel, "Rotate", view_rotate, ROTATION, control_cb);
	view_rot->set_spin(1.0);

	// Add Translation
	GLUI_Panel *translation_panel = glui->add_panel_to_panel(displayOptionsPanel, "", GLUI_PANEL_NONE);
	GLUI_Translation *trans_xy = glui->add_translation_to_panel(translation_panel, "Translate XY", GLUI_TRANSLATION_XY, obj_pos);
	trans_xy->scale_factor = 0.1f;
	GLUI_Translation *trans_z = glui->add_translation_to_panel(translation_panel, "Translate Z", GLUI_TRANSLATION_Z, &obj_pos[2]);
	trans_z->scale_factor = 0.1f;

	// Add Buttons
	glui->add_separator();
	glui->add_button("Open", OPEN, control_cb);
	glui->add_button("Save", SAVE, control_cb);
	glui->add_button("Reset", RESET, control_cb);
	glui->add_button("Quit", QUIT, (GLUI_Update_CB)exit);
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
	glutDisplayFunc(myGlutDisplay);
	GLUI_Master.set_glutReshapeFunc(myGlutReshape);
	GLUI_Master.set_glutIdleFunc(myGlutIdle);
	
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

	setupGlui();

	// Start Main Loop
	glutMainLoop();
}
