add_library (utils STATIC)

target_sources (utils PRIVATE
  random.cc
)

target_compile_options (utils PUBLIC
  -fPIC
)

target_link_libraries (utils
  ${CMAKE_THREAD_LIBS_INIT}
)
