include_directories(
  ${GSTREAMER_INCLUDE_DIRS}
  ${GSTREAMER_VIDEO_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${OPENCV_INCLUDE_DIRS}
)

set(CURLENDPOINT_SOURCES
  curlendpoint.c
  gstcurlendpoint.c
  gstcurlendpoint.h
)
set(PCM_SOURCES
  gstrtppcmupay.c
  gstrtppcmupay20.h
  gstrtppcmapay.c
  gstrtppcmapay.h
)

add_library(curlendpoint MODULE ${CURLENDPOINT_SOURCES})
add_library(rtppcmpay20 MODULE ${PCM_SOURCES})

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-as-needed")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--no-as-needed")

target_link_libraries(curlendpoint
  ${GSTREAMER_LIBRARIES}
  ${KMSELEMENTS_LIBRARIES}
)
target_link_libraries(rtppcmpay20
  ${GSTREAMER_LIBRARIES}
  ${GSTREAMER_BASE_LIBRARIES}
  ${GSTREAMER_RTP_LIBRARIES}
)

install(
  TARGETS curlendpoint rtppcmpay20
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_GST_PLUGINS_DIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
add_subdirectory(recorderendpoint)
