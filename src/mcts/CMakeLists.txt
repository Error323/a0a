add_library (mcts STATIC)

target_sources (mcts PRIVATE
  mcts.cc
)

target_include_directories (mcts PUBLIC
  ${CMAKE_SOURCE_DIR}/src
)

target_compile_options (mcts PUBLIC
  -fPIC
)

target_link_libraries (mcts
  ${CMAKE_THREAD_LIBS_INIT}
  ${GLOG_LIBRARIES}
  utils
  azul
)
