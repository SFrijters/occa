add_executable(occa occa.cpp)

if (ENABLE_STATIC)
  target_link_libraries(occa libocca_static)
else()
  target_link_libraries(occa libocca)
endif()
target_include_directories(occa PRIVATE
  $<BUILD_INTERFACE:${OCCA_SOURCE_DIR}/src>)

install(TARGETS occa DESTINATION bin)

add_custom_command(TARGET occa POST_BUILD COMMAND occa info)
