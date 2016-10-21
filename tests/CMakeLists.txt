include (TestHelpers)
set (TEST_PROPERTIES
  GST_PLUGIN_PATH=$ENV{GST_PLUGIN_PATH}:${CMAKE_BINARY_DIR}
)

set (VALGRING_TEST_PROPERTIES
  ${TEST_PROPERTIES}
  DEBUG_MEDIASET=TRUE
)

#set (SUPPRESSIONS "${CMAKE_CURRENT_SOURCE_DIR}/valgrind.supp")

add_test_program (test_recorder recorderEndpoint.cpp)
add_dependencies(test_recorder kmselementsplugins ${LIBRARY_NAME}impl)
set_property (TARGET test_recorder
  PROPERTY INCLUDE_DIRECTORIES
    ${KmsJsonRpc_INCLUDE_DIRS}
    ${sigc++-2.0_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/server/implementation/objects
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/server/implementation
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/server/interface
    ${CMAKE_CURRENT_BINARY_DIR}/../src/server/interface/generated-cpp
    ${CMAKE_CURRENT_BINARY_DIR}/../src/server/implementation/generated-cpp
    ${KMSCORE_INCLUDE_DIRS}
    ${gstreamer-1.5_INCLUDE_DIRS}
)
MESSAGE( STATUS "Boost Libs:         " ${Boost_LIBRARIES} )
target_link_libraries(test_recorder
  ${LIBRARY_NAME}impl
  ${KMSCORE_LIBRARIES}
  ${Boost_LIBRARIES}
)
