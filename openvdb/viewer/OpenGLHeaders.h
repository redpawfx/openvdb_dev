#ifndef OPENVDB_VIEWER_OPENGLHEADERS_HAS_BEEN_INCLUDED
#define OPENVDB_VIEWER_OPENGLHEADERS_HAS_BEEN_INCLUDED

#if defined(__APPLE__) || defined(MACOSX)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#endif // OPENVDB_VIEWER_OPENGLHEADERS_HAS_BEEN_INCLUDED
