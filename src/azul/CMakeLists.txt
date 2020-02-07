add_library (azul STATIC)

target_sources (azul PRIVATE
  center.cc
)

add_subdirectory (tests)
