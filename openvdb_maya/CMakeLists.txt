CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT ( openvdb_maya )

FIND_PACKAGE ( Maya REQUIRED )
FIND_PACKAGE ( Boost REQUIRED )
FIND_PACKAGE ( IlmBase REQUIRED )

ADD_DEFINITIONS ( ${MAYA_DEFINITIONS} )

# INCLDIRS := \
#     -I . \
#     -I $(OPENVDB_INCL_DIR) \
#     -I $(BOOST_INCL_DIR) \
#     -I $(HALF_INCL_DIR) \
#     -I $(TBB_INCL_DIR) \
#     -I $(MAYA_INCL_DIR) \
INCLUDE_DIRECTORIES ( ${OpenVDB_INCLUDE_DIR} )
INCLUDE_DIRECTORIES ( ${ILMBASE_INCLUDE_DIRS} )
INCLUDE_DIRECTORIES ( ${Boost_INCLUDE_DIR} )

# 	-lOpenMayaFX \
#     -lOpenMaya \
#     -lFoundation \

ADD_LIBRARY ( OpenVDB MODULE
  maya/OpenVDBCopyNode.cc
  maya/OpenVDBData.cc
  maya/OpenVDBFilterNode.cc
  maya/OpenVDBFromMayaFluidNode.cc
  maya/OpenVDBFromPolygonsNode.cc
  maya/OpenVDBPlugin.cc
  maya/OpenVDBReadNode.cc
  maya/OpenVDBTransformNode.cc
  maya/OpenVDBUtil.cc
  maya/OpenVDBVisualizeNode.cc
  maya/OpenVDBWriteNode.cc
  )

TARGET_LINK_LIBRARIES ( OpenVDB
  ${MAYA_Foundation_LIBRARY}
  ${MAYA_OpenMayaFX_LIBRARY}
  ${MAYA_OpenMaya_LIBRARY}
  ${OpenVDB_openvdb_LIBRARY}
  )

INSTALL ( TARGETS
  OpenVDB
  DESTINATION
  ${CMAKE_INSTALL_PREFIX}/plug-ins
  )

INSTALL ( FILES
  maya/AEOpenVDBCopyTemplate.mel
  maya/AEOpenVDBFilterTemplate.mel
  maya/AEOpenVDBFromMayaFluidTemplate.mel
  maya/AEOpenVDBFromPolygonsTemplate.mel
  maya/AEOpenVDBReadTemplate.mel
  maya/AEOpenVDBTransformTemplate.mel
  maya/AEOpenVDBVisualizeTemplate.mel
  maya/AEOpenVDBWriteTemplate.mel
  DESTINATION
  ${CMAKE_INSTALL_PREFIX}/scripts
  )
