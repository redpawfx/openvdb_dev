#-*-cmake-*-
#
# yue.nicholas@gmail.com
#
# This auxiliary CMake file helps in find the GLEW headers and libraries
#
# GLEW_FOUND            set if 3Glew is found.
# GLEW_INCLUDE_DIR      GLEW's include directory
# [DEPRECATED] GLEW_LIBRARY_DIR      GLEW's library directory
# GLEW_glew_LIBRARY        GLEW libraries
# GLEW_glewmx_LIBRARY      GLEWmx libraries (Mulitple Rendering Context)

SET ( GLEW_FOUND FALSE )

FIND_PATH( GLEW_LOCATION include/GL/glew.h
  "$ENV{GLEW_ROOT}"
  NO_DEFAULT_PATH
  NO_SYSTEM_ENVIRONMENT_PATH
  )

SET( GLEW_INCLUDE_DIR "${GLEW_LOCATION}/include" CACHE STRING "GLEW include path")

SET ( ORIGINAL_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
IF (GLEW_USE_STATIC_LIBS)
  IF (APPLE)
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    FIND_LIBRARY ( GLEW_LIBRARY_PATH GLEW PATHS ${GLEW_LOCATION}/lib
      NO_DEFAULT_PATH
      NO_SYSTEM_ENVIRONMENT_PATH
	  )
    FIND_LIBRARY ( GLEWmx_LIBRARY_PATH GLEWmx PATHS ${GLEW_LOCATION}/lib
      NO_DEFAULT_PATH
      NO_SYSTEM_ENVIRONMENT_PATH
	  )
	MESSAGE ( "APPLE STATIC" )
	MESSAGE ( "GLEW_LIBRARY_PATH = " ${GLEW_LIBRARY_PATH} )
  ELSEIF (WIN32)
    # Link library
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
    FIND_LIBRARY ( GLEW_LIBRARY_PATH GLEW32S PATHS ${GLEW_LOCATION}/lib )
    FIND_LIBRARY ( GLEWmx_LIBRARY_PATH GLEW32MXS PATHS ${GLEW_LOCATION}/lib )
  ELSE (APPLE)
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    FIND_LIBRARY ( GLEW_LIBRARY_PATH GLEW PATHS ${GLEW_LOCATION}/lib
      NO_DEFAULT_PATH
      NO_SYSTEM_ENVIRONMENT_PATH
	  )
    FIND_LIBRARY ( GLEWmx_LIBRARY_PATH GLEWmx PATHS ${GLEW_LOCATION}/lib
      NO_DEFAULT_PATH
      NO_SYSTEM_ENVIRONMENT_PATH
	  )
	MESSAGE ( "LINUX STATIC" )
	MESSAGE ( "GLEW_LIBRARY_PATH = " ${GLEW_LIBRARY_PATH} )
  ENDIF (APPLE)
ELSE ()
  IF (APPLE)
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ".dylib")
    FIND_LIBRARY ( GLEW_LIBRARY_PATH GLEW PATHS ${GLEW_LOCATION}/lib )
    FIND_LIBRARY ( GLEWmx_LIBRARY_PATH GLEWmx PATHS ${GLEW_LOCATION}/lib )
  ELSEIF (WIN32)
    # Link library
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
    FIND_LIBRARY ( GLEW_LIBRARY_PATH GLEW32 PATHS ${GLEW_LOCATION}/lib )
    FIND_LIBRARY ( GLEWmx_LIBRARY_PATH GLEW32mx PATHS ${GLEW_LOCATION}/lib )
    # Load library
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ".dll")
    FIND_LIBRARY ( GLEW_DLL_PATH GLEW32 PATHS ${GLEW_LOCATION}/bin
      NO_DEFAULT_PATH
      NO_SYSTEM_ENVIRONMENT_PATH
      )
    FIND_LIBRARY ( GLEWmx_DLL_PATH GLEW32mx PATHS ${GLEW_LOCATION}/bin
      NO_DEFAULT_PATH
      NO_SYSTEM_ENVIRONMENT_PATH
      )
  ELSE (APPLE)
	# Unices
    FIND_LIBRARY ( GLEW_LIBRARY_PATH GLEW PATHS ${GLEW_LOCATION}/lib
      NO_DEFAULT_PATH
      NO_SYSTEM_ENVIRONMENT_PATH
	  )
    FIND_LIBRARY ( GLEWmx_LIBRARY_PATH GLEWmx PATHS ${GLEW_LOCATION}/lib
      NO_DEFAULT_PATH
      NO_SYSTEM_ENVIRONMENT_PATH
	  )
  ENDIF (APPLE)
ENDIF ()
# MUST reset
SET(CMAKE_FIND_LIBRARY_SUFFIXES ${ORIGINAL_CMAKE_FIND_LIBRARY_SUFFIXES})

SET( GLEW_GLEW_LIBRARY ${GLEW_LIBRARY_PATH} CACHE STRING "GLEW library")
SET( GLEW_GLEWmx_LIBRARY ${GLEWmx_LIBRARY_PATH} CACHE STRING "GLEWmx library")
