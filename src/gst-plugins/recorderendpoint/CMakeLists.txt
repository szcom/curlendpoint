cmake_minimum_required(VERSION 2.8)

generic_find (LIBNAME gstreamer-1.5 VERSION ${GST_REQUIRED} REQUIRED)
generic_find (LIBNAME gstreamer-base-1.5 VERSION ${GST_REQUIRED} REQUIRED)
generic_find (LIBNAME gstreamer-video-1.5 VERSION ${GST_REQUIRED} REQUIRED)
generic_find (LIBNAME gstreamer-app-1.5 VERSION ${GST_REQUIRED} REQUIRED)
generic_find (LIBNAME gstreamer-check-1.5 VERSION ${GST_REQUIRED} REQUIRED)
generic_find (LIBNAME gstreamer-sdp-1.5 VERSION ${GST_REQUIRED} REQUIRED)
generic_find (LIBNAME gstreamer-rtp-1.5 VERSION ${GST_REQUIRED} REQUIRED)
generic_find (LIBNAME gstreamer-pbutils-1.5 VERSION ${GST_REQUIRED} REQUIRED)
generic_find (LIBNAME glibmm-2.4 VERSION ${GLIBMM_REQUIRED} REQUIRED)
generic_find (LIBNAME KmsGstCommons REQUIRED)
generic_find (LIBNAME libsoup-2.4 VERSION ${SOUP_REQUIRED} REQUIRED)
generic_find (LIBNAME nice VERSION ${NICE_REQUIRED} REQUIRED)
generic_find (LIBNAME gio-2.0 VERSION ${GLIB_REQUIRED} REQUIRED)
generic_find (LIBNAME uuid REQUIRED)


include_directories(
  ${KmsGstCommons_INCLUDE_DIRS}
  ${gstreamer-1.5_INCLUDE_DIRS}
  ${gstreamer-base-1.5_INCLUDE_DIRS}
  ${gstreamer-app-1.5_INCLUDE_DIRS}
  ${gstreamer-pbutils-1.5_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${libsoup-2.4_INCLUDE_DIRS}
)

set(KMS_RECORDERENDPOINT_SOURCES
  kmsbasemediamuxer.c
  kmsavmuxer.c
  kmsksrmuxer.c
  kmsrecorderendpoint.c
)

set(KMS_RECORDERENDPOINT_HEADERS
  kmsbasemediamuxer.h
  kmsavmuxer.h
  kmsksrmuxer.h
  kmsrecorderendpoint.h
)

add_library(lolrecorderendpoint MODULE ${KMS_RECORDERENDPOINT_SOURCES} ${KMS_RECORDERENDPOINT_HEADERS})

set_property (TARGET lolrecorderendpoint
  PROPERTY INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_BINARY_DIR}/../../..
    ${gstreamer-1.5_INCLUDE_DIRS}
    ${KmsGstCommons_INCLUDE_DIRS}
)

target_link_libraries(lolrecorderendpoint
  ${KmsGstCommons_LIBRARIES}
  ${gstreamer-1.5_LIBRARIES}
  ${gstreamer-base-1.5_LIBRARIES}
  ${gstreamer-app-1.5_LIBRARIES}
  ${gstreamer-pbutils-1.5_LIBRARIES}
)

install(
  TARGETS lolrecorderendpoint
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_GST_PLUGINS_DIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
