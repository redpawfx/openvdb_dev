CMAKE_MINIMUM_REQUIRED ( VERSION 2.8 )

PROJECT ( OpenVDB )

FIND_PACKAGE ( Boost )
FIND_PACKAGE ( TBB )
FIND_PACKAGE ( ILMBase ) 
FIND_PACKAGE ( GLFW )
FIND_PACKAGE ( OpenGL )
FIND_PACKAGE ( ZLIB )
FIND_PACKAGE ( GLEW )

INCLUDE_DIRECTORIES ( ${Boost_INCLUDE_DIR} )
INCLUDE_DIRECTORIES ( ${ILMBASE_INCLUDE_DIRS} )
INCLUDE_DIRECTORIES ( ${TBB_INCLUDE_DIR} )
INCLUDE_DIRECTORIES ( ${GLFW_INCLUDE_DIR} )
INCLUDE_DIRECTORIES ( ${GLEW_INCLUDE_DIR} )
INCLUDE_DIRECTORIES ( ${CMAKE_SOURCE_DIR}/.. )

IF (WIN32)
  ADD_DEFINITIONS (
	/DNOMINMAX
	/DHALF_EXPORTS
	)
  LINK_DIRECTORIES ( ${TBB_LIBRARY_DIR} ) # TBB header has implicit linkage
ELSE ()
  ADD_DEFINITIONS (
	-pthread
	)
ENDIF ()

ADD_LIBRARY ( openvdb SHARED
  Grid.cc
  io/Archive.cc
  io/Compression.cc
  io/File.cc
  io/GridDescriptor.cc
  io/Stream.cc
  math/Hermite.cc
  math/Maps.cc
  math/Proximity.cc
  math/QuantizedUnitVec.cc
  math/Transform.cc
  metadata/Metadata.cc
  metadata/MetaMap.cc
  openvdb.cc
  Platform.cc
  util/Formats.cc
  util/Util.cc
  )

IF (WIN32)
  # On Windows, TBB is linked implicitly (Intel's design decision)
  TARGET_LINK_LIBRARIES ( openvdb
	${Ilmbase_HALF_LIBRARY}
	${ZLIB_LIBRARY}
	)
ELSE ()
  TARGET_LINK_LIBRARIES ( openvdb
	${Ilmbase_HALF_LIBRARY}
	${ZLIB_LIBRARY}
	${Tbb_TBB_LIBRARY}
	)
ENDIF ()

SET_TARGET_PROPERTIES ( openvdb
  PROPERTIES
  COMPILE_FLAGS -DOPENVDB_PRIVATE
  )

ADD_EXECUTABLE ( vdb_print
  cmd/openvdb_print/main.cc
)

ADD_EXECUTABLE ( vdb_view
  cmd/openvdb_view/main.cc
  cmd/openvdb_view/RenderModules.cc
  cmd/openvdb_view/Viewer.cc
)

SET_TARGET_PROPERTIES ( vdb_view
  PROPERTIES
  COMPILE_FLAGS -DGL_GLEXT_PROTOTYPES=1
  )

SET ( APPLICATIONS_DEPENDENT_LIBRARIES
  openvdb
  ${Ilmbase_HALF_LIBRARY}
  ${Tbb_TBB_LIBRARY}
  ${ZLIB_LIBRARY}
  )

TARGET_LINK_LIBRARIES ( vdb_print
  ${APPLICATIONS_DEPENDENT_LIBRARIES}
  )

TARGET_LINK_LIBRARIES ( vdb_view
  ${APPLICATIONS_DEPENDENT_LIBRARIES}
  ${GLFW_glfw_LIBRARY}
  ${GLEW_GLEW_LIBRARY}
  ${OPENGL_gl_LIBRARY}
  ${OPENGL_glu_LIBRARY}
  )

##################
# INSTALLATION
##################

# Applications

INSTALL ( TARGETS
  vdb_print
  vdb_view
  DESTINATION
  bin
)

# SDK Libraries

INSTALL ( TARGETS
  openvdb
  DESTINATION
  lib
)

# SDK Headers

INSTALL ( FILES
  Exceptions.h
  Grid.h
  Metadata.h
  Platform.h
  Types.h
  openvdb.h
  version.h
  DESTINATION
  include/openvdb
)

FILE ( GLOB io_headers io/*.h )
FILE ( GLOB math_headers math/*.h )
FILE ( GLOB metadata_headers metadata/*.h )
FILE ( GLOB tools_headers tools/*.h )
FILE ( GLOB tree_headers tree/*.h )
FILE ( GLOB util_headers util/*.h )

MESSAGE ( "io_headers = ${io_headers}")

INSTALL ( FILES
  ${io_headers}
  DESTINATION
  include/openvdb/io
)

INSTALL ( FILES
  ${math_headers}
  DESTINATION
  include/openvdb/math
)

INSTALL ( FILES
  ${metadata_headers}
  DESTINATION
  include/openvdb/metadata
)

INSTALL ( FILES
  ${tools_headers}
  DESTINATION
  include/openvdb/tools
)

INSTALL ( FILES
  ${tree_headers}
  DESTINATION
  include/openvdb/tree
)

INSTALL ( FILES
  ${util_headers}
  DESTINATION
  include/openvdb/util
)
