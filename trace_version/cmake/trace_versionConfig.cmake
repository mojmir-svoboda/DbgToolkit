include("${CMAKE_CURRENT_LIST_DIR}/trace_versionTargets.cmake")

get_filename_component(trace_version_INCLUDE_DIRS "${SELF_DIR}/../../include/trace_version" ABSOLUTE)
set(trace_version_LIBRARIES trace_version)
