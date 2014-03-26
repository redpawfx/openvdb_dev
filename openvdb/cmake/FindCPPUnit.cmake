#-*-cmake-*-
#
# yue.nicholas@gmail.com
#
# This auxiliary CMake file helps in find the CPPunit headers and libraries
#
# CPPUNIT_FOUND            set if CPPUnit is found.
# CPPUNIT_INCLUDE_DIR      CPPUnit's include directory
# CPPUNIT_LIBRARY_DIR      CPPUnit's library directory
# CPPUNIT_LIBRARIES        all cppunit libraries

SET( CPPUNIT_FOUND "NO" )

##
## Obtain CPPUnit install location
##

FIND_PATH( CPPUNIT_LOCATION include/cppunit/TestRunner.h
  $ENV{CPPUNIT_ROOT}
  /usr/local
  /usr
)

IF (CPPUNIT_LOCATION)
  # MESSAGE ( STATUS ${CPPUNIT_LOCATION} )
  SET( CPPUNIT_FOUND "YES" )
ENDIF (CPPUNIT_LOCATION)

SET( CPPUNIT_INCLUDE_DIR       "${CPPUNIT_LOCATION}/include" )
SET( CPPUNIT_LIBRARY_DIR       "${CPPUNIT_LOCATION}/lib" )
IF (WIN32)
  IF (CMAKE_BUILD_TYPE MATCHES Release)
    FIND_LIBRARY( CPPUNIT_cppunit_LIBRARY cppunit
      ${CPPUNIT_LIBRARY_DIR}
      NO_DEFAULT_PATH
      )
  ELSE (CMAKE_BUILD_TYPE MATCHES Release)
    FIND_LIBRARY( CPPUNIT_cppunit_LIBRARY cppunit
      ${CPPUNIT_LIBRARY_DIR}
      NO_DEFAULT_PATH
      )
  ENDIF (CMAKE_BUILD_TYPE MATCHES Release)
ELSE (WIN32)
  # TODO : Fix me
  SET ( CMAKE_FIND_LIBRARY_SUFFIXES ".a" )
  FIND_LIBRARY( CPPUNIT_cppunit_LIBRARY cppunit
    ${CPPUNIT_LIBRARY_DIR}
    NO_DEFAULT_PATH
    )
ENDIF (WIN32)

IF ( WIN32 )
  SET( CPPUNIT_LIBRARIES 
    ${CPPUNIT_cppunit_LIBRARY}
    )
ELSE (WIN32 )
  IF (CMAKE_BUILD_TYPE MATCHES Debug)
    SET ( GCOV_LIB "gcov" )
  ENDIF (CMAKE_BUILD_TYPE MATCHES Debug)
  SET( CPPUNIT_LIBRARIES 
    ${CPPUNIT_cppunit_LIBRARY} dl
    )
ENDIF ( WIN32 )

STRING ( REGEX MATCH "[/a-zA-Z0-9.]*/"
  CPPUNIT_LIBRARY_DIR "${CPPUNIT_cppunit_LIBRARY}")

INCLUDE_DIRECTORIES( ${CPPUNIT_INCLUDE_DIR} )
# LINK_DIRECTORIES( ${CPPUNIT_LIBRARY_DIR} )
