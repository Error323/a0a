add_executable (center_test
  center_test.cc
)

target_include_directories (center_test PUBLIC
  ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries (center_test
  ${GTEST_BOTH_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  azul
)

add_test (center_test ${CMAKE_BINARY_DIR}/center_test)
