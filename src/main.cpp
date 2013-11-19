#include <QApplication>

#include "viewer.h"

int main(int argc, char** argv) {
   QApplication app(argc, argv);
   kirkpatrick_viewer viewer;
   visualization::run_viewer(&viewer, "Kirkpatrick triangulation refinement");
}
