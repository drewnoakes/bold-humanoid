find_package(SWIG REQUIRED)
find_package(PythonLibs)

include(${SWIG_USE_FILE})
#set(CMAKE_SWIG_FLAGS "-builtin") # builtin has one layer less, but can't extend classes with Python code
set(CMAKE_SWIG_FLAGS "")

#todo: this is a hack
include_directories("${PYTHON_INCLUDE_PATH};/usr/include/i386-linux-gnu/python3.3m/;${CMAKE_SOURCE_DIR}")

set_source_files_properties(${CMAKE_SOURCE_DIR}/swig/boldpy.i PROPERTIES CPLUSPLUS ON)
set_source_files_properties(${CMAKE_SOURCE_DIR}/swig/boldpy.i PROPERTIES SWIG_FLAGS "")

swig_add_module(bold python ${CMAKE_SOURCE_DIR}/swig/boldpy.i $<TARGET_OBJECTS:boldhumanoid_objects>)

swig_link_libraries(bold ${PYTHON_LIBRARIES})

set_target_properties(${SWIG_MODULE_bold_REAL_NAME} PROPERTIES COMPILE_FLAGS "${BOLDHUMANOID_COMPILE_FLAGS} -w")
add_dependencies(${SWIG_MODULE_bold_REAL_NAME} boldhumanoid_objects)
target_link_libraries(${SWIG_MODULE_bold_REAL_NAME} ${BOLDHUMANOID_LINK_LIBRARIES})
