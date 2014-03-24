# yue.nicholas@gmail.com
# Python interface
ADD_DEFINITIONS (
)

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
  openvdb
  # ${APPLICATIONS_DEPENDENT_LIBRARIES}
  )
