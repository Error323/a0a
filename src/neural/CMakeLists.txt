cuda_add_library (neural STATIC neuralnet.cc gpumanager.cc nnlogger.cc)

target_include_directories (neural PUBLIC
  ${CMAKE_SOURCE_DIR}/src
  ${TensorRT_INCLUDE_DIRS}
)

add_executable (nnbuilder nnbuilder.cc nnlogger.cc)

target_link_libraries (nnbuilder
  ${GLOG_LIBRARIES}
  ${GFLAGS_LIBRARIES}
  ${TensorRT_LIBRARIES}
)

cuda_add_executable(nninfer gpumanager.cc nninfer.cc nnlogger.cc)

target_include_directories (nninfer PUBLIC
  ${CMAKE_SOURCE_DIR}/src
  ${TensorRT_INCLUDE_DIRS}
)

target_link_libraries (nninfer
  ${GLOG_LIBRARIES}
  ${TensorRT_LIBRARIES}
)
