# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/netV_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/netV_autogen.dir/ParseCache.txt"
  "netV_autogen"
  )
endif()
