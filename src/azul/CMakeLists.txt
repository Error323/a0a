add_library (azul STATIC)

target_sources (azul PRIVATE
  center.cc
  board.cc
  move.cc
  magics.cc
  state.cc
)

target_include_directories (azul PUBLIC
  ${CMAKE_SOURCE_DIR}/src
)

target_compile_options (azul PUBLIC
  -fPIC
)

target_link_libraries (azul
  ${CMAKE_THREAD_LIBS_INIT}
  ${GLOG_LIBRARIES}
  utils
)

add_subdirectory (tests)
