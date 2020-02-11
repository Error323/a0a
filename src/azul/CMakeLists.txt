add_library (azul STATIC)
add_executable (magics magics.cc)

target_sources (azul PRIVATE
  center.cc
  board.cc
  move.cc
)

target_include_directories (azul PUBLIC
  ${CMAKE_SOURCE_DIR}/src
)

target_compile_options (azul PUBLIC
  -fPIC
)

target_link_libraries (azul
  ${CMAKE_THREAD_LIBS_INIT}
  utils
)


add_subdirectory (tests)
