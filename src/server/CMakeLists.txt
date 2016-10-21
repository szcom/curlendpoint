cmake_minimum_required (VERSION 2.8)

include (CodeGenerator)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-as-needed -lkmselementsimpl -Wl,--as-needed")

generate_code (
  MODELS ${CMAKE_CURRENT_SOURCE_DIR}/interface
  SERVER_STUB_DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/implementation/objects
)
