set (tests center board state)

foreach (test ${tests})
  set (name ${test}_test)

  add_executable (${name}
    ${name}.cc
  )

  target_include_directories (${name} PUBLIC
    ${CMAKE_SOURCE_DIR}/src
  )

  target_link_libraries (${name}
    ${GTEST_BOTH_LIBRARIES}
    ${GLOG_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    azul
  )

  add_test (${name} ${CMAKE_BINARY_DIR}/${name})
endforeach()

