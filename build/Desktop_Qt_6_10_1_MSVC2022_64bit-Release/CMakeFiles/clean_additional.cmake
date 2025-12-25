# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles\\malak_browser_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\malak_browser_autogen.dir\\ParseCache.txt"
  "malak_browser_autogen"
  )
endif()
