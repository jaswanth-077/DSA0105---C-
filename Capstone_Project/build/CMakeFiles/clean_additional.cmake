# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles\\SmartStudyPlanner_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\SmartStudyPlanner_autogen.dir\\ParseCache.txt"
  "SmartStudyPlanner_autogen"
  )
endif()
