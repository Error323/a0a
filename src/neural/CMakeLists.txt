add_executable (nnbuilder nnbuilder.cc)

target_link_libraries (nnbuilder
  ${GLOG_LIBRARIES}
  nvinfer
)

cuda_add_executable(nninfer nninfer.cc)

target_link_libraries (nninfer
  ${GLOG_LIBRARIES}
  nvinfer
)
