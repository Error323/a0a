add_subdirectory (azul)
add_subdirectory (mcts)
add_subdirectory (utils)
add_subdirectory (neural)

add_executable (a0a
  main.cc
)

target_include_directories (a0a PRIVATE
  ${PROJECT_BINARY_DIR}
)

target_link_libraries (a0a
  ${GFLAGS_LIBRARIES}
  ${GLOG_LIBRARIES}
  ${CUDNN_LIBRARIES}
  azul
  mcts
  profiler
  neural
  nvinfer
)

target_link_options (a0a PRIVATE -flto)
