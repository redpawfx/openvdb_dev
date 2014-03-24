# yue.nicholas@gmail.com
#
CMAKE_MINIMUM_REQUIRED ( VERSION 2.8 )

PROJECT ( OpenVDB )

MESSAGE ( "BEFORE CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}") 
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake") 
MESSAGE ( "AFTER CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}") 

FIND_PACKAGE ( Boost COMPONENTS python)
FIND_PACKAGE ( TBB )
FIND_PACKAGE ( ILMBase ) 
FIND_PACKAGE ( OpenEXR ) 
FIND_PACKAGE ( GLFW )
FIND_PACKAGE ( OpenGL )
FIND_PACKAGE ( ZLIB )
FIND_PACKAGE ( GLEW )
FIND_PACKAGE ( PythonLibs )
FIND_PACKAGE ( PythonInterp )
FIND_PACKAGE ( CPPUnit )
FIND_PACKAGE ( Doxygen )

INCLUDE_DIRECTORIES ( ${Boost_INCLUDE_DIR} )
INCLUDE_DIRECTORIES ( ${ILMBASE_INCLUDE_DIRS} )
INCLUDE_DIRECTORIES ( ${OPENEXR_INCLUDE_DIRS} )
INCLUDE_DIRECTORIES ( ${TBB_INCLUDE_DIR} )
INCLUDE_DIRECTORIES ( ${GLFW_INCLUDE_DIR} )
INCLUDE_DIRECTORIES ( ${GLEW_INCLUDE_DIR} )
INCLUDE_DIRECTORIES ( ${CMAKE_SOURCE_DIR}/.. )
INCLUDE_DIRECTORIES ( ${PYTHON_INCLUDE_DIR} )
INCLUDE_DIRECTORIES ( ${NUMPY_INCL_DIR} )
INCLUDE_DIRECTORIES ( . )

IF (WIN32)
  ADD_DEFINITIONS (
	/DNOMINMAX
	/DHALF_EXPORTS
	)
  LINK_DIRECTORIES ( ${TBB_LIBRARY_DIR} ) # TBB header has implicit linkage
ELSE ()
  IF (NOT APPLE)
    SET ( XRANDR_LIBRARY Xrandr)
  ENDIF ()
  ADD_DEFINITIONS (
	-pthread
	)
ENDIF ()

ADD_LIBRARY ( openvdb SHARED
  ./Grid.cc
  ./io/Archive.cc
  ./io/Compression.cc
  ./io/File.cc
  ./io/GridDescriptor.cc
  ./io/Queue.cc
  ./io/Stream.cc
  ./math/Hermite.cc
  ./math/Maps.cc
  ./math/Proximity.cc
  ./math/QuantizedUnitVec.cc
  ./math/Transform.cc
  ./metadata/Metadata.cc
  ./metadata/MetaMap.cc
  ./openvdb.cc
  ./Platform.cc
  ./util/Formats.cc
  ./util/Util.cc
  ./viewer/Camera.cc
  ./viewer/ClipBox.cc
  ./viewer/Font.cc
  ./viewer/RenderModules.cc
  ./viewer/Viewer.cc
  )

IF (WIN32)
  # On Windows, TBB is linked implicitly (Intel's design decision)
  TARGET_LINK_LIBRARIES ( openvdb
	${Ilmbase_HALF_LIBRARY}
	${ZLIB_LIBRARY}
	${OPENGL_gl_LIBRARY}
	${OPENGL_glu_LIBRARY}
	${GLFW_glfw_LIBRARY}
	)
ELSE ()
  TARGET_LINK_LIBRARIES ( openvdb
	${Ilmbase_HALF_LIBRARY}
	${ZLIB_LIBRARY}
	${Tbb_TBB_LIBRARY}
	${OPENGL_gl_LIBRARY}
	${OPENGL_glu_LIBRARY}
	${GLFW_glfw_LIBRARY}
	${XRANDR_LIBRARY}
	)
ENDIF ()

SET_TARGET_PROPERTIES ( openvdb
  PROPERTIES
  COMPILE_FLAGS -DOPENVDB_PRIVATE
  )

# Python interface
ADD_LIBRARY ( pyopenvdb SHARED
  python/pyFloatGrid.cc
  python/pyIntGrid.cc
  python/pyMetadata.cc
  python/pyOpenVDBModule.cc
  python/pyTransform.cc
  python/pyVec3Grid.cc
  )

TARGET_LINK_LIBRARIES ( pyopenvdb
  ${Tbb_TBB_LIBRARY}
  ${Boost_LIBRARIES}
  ${PYTHON_LIBRARY}
  ${Ilmbase_HALF_LIBRARY}
  openvdb
  )

SET_TARGET_PROPERTIES ( pyopenvdb
  PROPERTIES
  COMPILE_FLAGS -DPY_OPENVDB_USE_NUMPY
  PREFIX ""
  )

# Executables

ADD_EXECUTABLE ( vdb_print
  cmd/openvdb_print/main.cc
)

ADD_EXECUTABLE ( vdb_render
  cmd/openvdb_render/main.cc 
)

ADD_EXECUTABLE ( vdb_view
  cmd/openvdb_view/main.cc
)

ADD_EXECUTABLE ( vdb_test
  unittest/main.cc
  unittest/TestBBox.cc
  unittest/TestCoord.cc
  unittest/TestCpt.cc
  unittest/TestCurl.cc
  unittest/TestDense.cc
  unittest/TestDivergence.cc
  unittest/TestDoubleMetadata.cc
  unittest/TestExceptions.cc
  unittest/TestFile.cc
  unittest/TestFloatMetadata.cc
  unittest/TestGradient.cc
  unittest/TestGridBbox.cc
  unittest/TestGrid.cc
  unittest/TestGridDescriptor.cc
  unittest/TestGridIO.cc
  unittest/TestGridTransformer.cc
  unittest/TestHermite.cc
  unittest/TestInit.cc
  unittest/TestInt32Metadata.cc
  unittest/TestInt64Metadata.cc
  unittest/TestInternalOrigin.cc
  unittest/TestLaplacian.cc
  unittest/TestLeafBool.cc
  unittest/TestLeaf.cc
  unittest/TestLeafIO.cc
  unittest/TestLeafOrigin.cc
  unittest/TestLevelSetRayIntersector.cc
  unittest/TestLevelSetUtil.cc
  unittest/TestLinearInterp.cc
  unittest/TestMaps.cc
  unittest/TestMat4Metadata.cc
  unittest/TestMath.cc
  unittest/TestMeanCurvature.cc
  unittest/TestMeshToVolume.cc
  unittest/TestMetadata.cc
  unittest/TestMetadataIO.cc
  unittest/TestMetaMap.cc
  unittest/TestName.cc
  unittest/TestNodeIterator.cc
  unittest/TestNodeMask.cc
  unittest/TestParticlesToLevelSet.cc
  unittest/TestPrePostAPI.cc
  unittest/TestQuadraticInterp.cc
  unittest/TestQuantizedUnitVec.cc
  unittest/TestQuat.cc
  unittest/TestRay.cc
  unittest/TestStats.cc
  unittest/TestStream.cc
  unittest/TestStringMetadata.cc
  unittest/TestTools.cc
  unittest/TestTransform.cc
  unittest/TestTree.cc
  unittest/TestTreeCombine.cc
  unittest/TestTreeGetSetValues.cc
  unittest/TestTreeIterators.cc
  unittest/TestTreeVisitor.cc
  unittest/TestValueAccessor.cc
  unittest/TestVec2Metadata.cc
  unittest/TestVec3Metadata.cc
  unittest/TestVolumeRayIntersector.cc
  unittest/TestVolumeToMesh.cc
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
  ${GLFW_glfw_LIBRARY}
  ${GLEW_GLEW_LIBRARY}
  ${OPENGL_gl_LIBRARY}
  ${OPENGL_glu_LIBRARY}
  )

TARGET_LINK_LIBRARIES ( vdb_render
  ${APPLICATIONS_DEPENDENT_LIBRARIES}
  ${GLEW_GLEW_LIBRARY}
  ${Openexr_ILMIMF_LIBRARY}
  )

TARGET_LINK_LIBRARIES ( vdb_view
  ${APPLICATIONS_DEPENDENT_LIBRARIES}
  ${GLFW_glfw_LIBRARY}
  ${GLEW_GLEW_LIBRARY}
  ${OPENGL_gl_LIBRARY}
  ${OPENGL_glu_LIBRARY}
  )

TARGET_LINK_LIBRARIES ( vdb_test
  ${APPLICATIONS_DEPENDENT_LIBRARIES}
  ${GLEW_GLEW_LIBRARY}
  ${CPPUNIT_cppunit_LIBRARY}
  )

##################
# INSTALLATION
##################

# Applications

INSTALL ( TARGETS
  vdb_print
  vdb_render
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
  PlatformConfig.h
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

# Python
INSTALL ( TARGETS
  pyopenvdb
  DESTINATION
  python/lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}
  )
INSTALL ( FILES
  ./python/pyopenvdb.h
  DESTINATION
  python/include/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}
  )

# Doxygen
# share/doc/openvdb/html
# echo 'OUTPUT_DIRECTORY=./doc' | cat doxygen-config - | doxygen -
# cp -r doc/html share/doc/openvdb
EXECUTE_PROCESS ( COMMAND "echo 'OUTPUT_DIRECTORY=./doc' | cat doxygen-config - | ${DOXYGEN_EXECUTABLE} -")
INSTALL ( DIRECTORY
  doc/html
  DESTINATION
  share/doc/openvdb
  )
