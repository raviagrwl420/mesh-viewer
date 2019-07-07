# A simple mesh viewer/editor in C++

## Features
* Open, view and save modified mesh in .smf format
* Mesh decimation using quadric-based errors
* Butterfly and Loop subdivision

## Instructions
* Run `make` to build the executable
* Run `./mcaq` to launch the program
* Run `make clean` to clean build files

## Dependencies
* OpenGL Utility Toolkit (GLUT) 2.8.1
* GLUT-based C++ User Interface (GLUI) 2.36
* OpenGL Mathematics (GLM) library 

## Limitations
At extremely low polygon count (say less than 200 for the Horse mesh), a non-manifold mesh is almost unavoidable due to the local geometry. If we try to further decimate an edge, the non-manifold mesh causes the program to break. 
