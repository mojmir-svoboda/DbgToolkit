add_definitions("-DTRACE_ENABLED")
add_definitions("-DTRACE_STATIC")
add_library (trace_client STATIC trace.cpp) 
