add_library (azul STATIC)

target_sources (azul PRIVATE
  center.cc
)

target_compile_options (azul PUBLIC
  -fPIC
)

target_link_libraries (azul
  ${CMAKE_THREAD_LIBS_INIT}
)

add_subdirectory (tests)
