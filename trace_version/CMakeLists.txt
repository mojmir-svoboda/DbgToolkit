include("${CMAKE_SOURCE_DIR}/build/version.cmake")
cmake_policy(SET CMP0048 NEW)
project (trace_version VERSION 1.1.0)

add_custom_target(generate_version ALL)
add_custom_command(TARGET generate_version COMMAND ${CMAKE_COMMAND}
         -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
         -P ${CMAKE_SOURCE_DIR}/build/version.cmake)

add_library(trace_version trace_version.cpp)
#install(TARGETS trace_version DESTINATION lib EXPORT generate_version)
#install(EXPORT generate_version DESTINATION lib)

# properties of targets
target_compile_definitions(trace_version PUBLIC "trace_version_DEBUG=$<CONFIG:Debug>")

#set(config_install_dir "lib/cmake/${PROJECT_NAME}")
set(config_install_dir "lib/${PROJECT_NAME}")
set(lib_install_dir "lib/${PROJECT_NAME}")
set(include_install_dir "include/${PROJECT_NAME}")
set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")

set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
set(targets_export_name "${PROJECT_NAME}Targets")
#set(namespace "${PROJECT_NAME}::")

include(CMakePackageConfigHelpers)
# Configure '<PROJECT-NAME>ConfigVersion.cmake' Note: PROJECT_VERSION is used as a VERSION
write_basic_package_version_file( "${version_config}" COMPATIBILITY SameMajorVersion)

# Configure '<prj>Config.cmake'
# Note: variable 'targets_export_name' used
configure_file("cmake/Config.cmake.in" "${project_config}" @ONLY)

install(TARGETS trace_version 
  EXPORT "${targets_export_name}"
  LIBRARY DESTINATION "${lib_install_dir}"
  ARCHIVE DESTINATION "${lib_install_dir}"
  RUNTIME DESTINATION "bin"
  INCLUDES DESTINATION "${include_install_dir}"
)

# Headers:
install(FILES trace_version.h DESTINATION "${include_install_dir}")

# Config
#   * <prefix>/lib/cmake/<prj>/<prj>Config.cmake
#   * <prefix>/lib/cmake/<prj>/<prj>ConfigVersion.cmake
install(FILES "${project_config}" "${version_config}" DESTINATION "${config_install_dir}")

# Config
#   * <prefix>/lib/cmake/<prj>/<prj>Targets.cmake
install(EXPORT "${targets_export_name}" NAMESPACE "${namespace}" DESTINATION "${config_install_dir}")

