cmake_minimum_required (VERSION 2.8)
project (Flogging)
set (CMAKE_VERBOSE_MAKEFILE on)
set (FLOGGING_MAJOR_VERSION 0)
set (FLOGGING_MINOR_VERSION 1)
set (FLOGGING_PATCH_VERSION 0)
set (FLOGGING_VERSION ${FLOGGING_MAJOR_VERSION}.${FLOGGING_MINOR_VERSION}.${FLOGGING_PATCH_VERSION})

# Offer the user the choice of overriding the installation directories
set (INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set (INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
set (INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for header files")
if (WIN32 AND NOT CYGWIN)
	set (DEF_INSTALL_CMAKE_DIR CMake)
else ()
	set (DEF_INSTALL_CMAKE_DIR lib/CMake/Flogging)
endif ()
set (INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH "Installation directory for CMake files")

# Make relative paths absolute (needed later on)
foreach (p LIB BIN INCLUDE CMAKE)
  set (var INSTALL_${p}_DIR)
  if (NOT IS_ABSOLUTE "${${var}}")
    set (${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
  endif ()
endforeach ()

# set up include-directories
include_directories (
  "${PROJECT_SOURCE_DIR}"   # to find trace_client/trace.h
  "${PROJECT_BINARY_DIR}")  # to find trace_client/config.h

add_subdirectory (trace_client)

# Add all targets to the build-tree export set
export (TARGETS trace_client FILE "${PROJECT_BINARY_DIR}/FloggingLibraryDepends.cmake")

# Export the package for use from the build-tree (this registers the build-tree with a global CMake-registry)
export (PACKAGE Flogging)

# Create a FloggingBuildTreeSettings.cmake file for the use from the build tree
configure_file (FloggingBuildTreeSettings.cmake.in "${PROJECT_BINARY_DIR}/FloggingBuildTreeSettings.cmake" @ONLY)

# Create the FloggingConfig.cmake and FloggingConfigVersion files
file (RELATIVE_PATH CONF_REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}" "${INSTALL_INCLUDE_DIR}")
configure_file (FloggingConfig.cmake.in "${PROJECT_BINARY_DIR}/FloggingConfig.cmake" @ONLY)
configure_file (FloggingConfigVersion.cmake.in "${PROJECT_BINARY_DIR}/FloggingConfigVersion.cmake" @ONLY)

# Install the export set for use with the install-tree
install (EXPORT FloggingLibraryDepends DESTINATION "${INSTALL_DATA_DIR}/Flogging/CMake" COMPONENT dev)
