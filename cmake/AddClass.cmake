# add_class(<prefix> source1 [source2 [source3 ...]]
#  uses variables:
#  <prefix>_COMPILE_FLAGS - adds these flags to ih target
#  sets up these variables:
#  <_prefix>_SOURCES - gets the given source files appended
#  <_prefix>_PCH_TARGETS - if there is an internal header for this class, a target to recompile it is added
macro(add_class _prefix)
  # Get sources and set up variables
  set(_sources ${ARGN})
  list(GET _sources 0 _fstSource)
  get_filename_component(_path ${_fstSource} PATH)
  get_filename_component(_class ${_path} NAME)
  set(_prefix_sources_var ${_prefix}_SOURCES)
  set(_prefix_pch_targets_var ${_prefix}_PCH_TARGETS)
  
  # Append source files
  list(APPEND ${_prefix_sources_var} ${_sources})

  # Check if there is an internal header
  string(TOLOWER ${_class}.ih _ih_name)
  set(_ih_path ${_path}/${_ih_name})
  set(_ih_full_path ${CMAKE_SOURCE_DIR}/${_ih_path})
  if(EXISTS ${_ih_full_path})
    message("internal header found")
    string(REPLACE ".ih" ".ih.gch" _pcih_name ${_ih_name})

    # Path for precompiled header
    set(_pcih_full_dir_path ${CMAKE_CURRENT_BINARY_DIR}/${_path}/${_pcih_name})
    set(_pcih_full_path ${_pcih_full_dir_path}/${CMAKE_BUILD_TYPE}.c++)

    message("pcih full path: "${_pcih_full_path})
    # Copy compile flags
    set(_COMPILE_FLAGS ${${_prefix}_COMPILE_FLAGS})

    # Add general cmake compile flags
    if(CMAKE_BUILD_TYPE)
      string(TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" _cmake_cxx_flags_var)
    else(CMAKE_BUILD_TYPE)
      set(_cmake_cxx_flags_var "CMAKE_CXX_FLAGS")
    endif(CMAKE_BUILD_TYPE)
    set(_cmake_cxx_flags ${${_cmake_cxx_flags_var}})
    list(APPEND _COMPILE_FLAGS ${_cmake_cxx_flags})
    
    # Get include directories
    get_directory_property(_directory_flags INCLUDE_DIRECTORIES)
    foreach(item ${_directory_flags})
      list(APPEND _COMPILE_FLAGS "-I${item}")
    endforeach(item)

    # Get additional definitions
    get_directory_property(_directory_flags DEFINITIONS)
    list(APPEND _COMPILE_FLAGS ${_directory_flags})

    # Split all elements with spaces
    separate_arguments(_COMPILE_FLAGS)

    # Make a command for precompiling the internal header
    add_custom_command(OUTPUT ${_pcih_full_path}
      COMMAND mkdir -p ${_pcih_full_dir_path}
      COMMAND ${CMAKE_CXX_COMPILER} -x c++-header ${_COMPILE_FLAGS} -c ${_ih_full_path} -o ${_pcih_full_path}
      IMPLICIT_DEPENDS CXX ${_ih_full_path})

    # Make a custom target for the internal header file
    set(_pcih_target "${_ih_name}_pcih")
    add_custom_target(${_pcih_target} DEPENDS ${_pcih_full_path})
    list(APPEND ${_prefix_pch_targets_var} ${_pcih_target})
  endif(EXISTS ${_ih_full_path})

  message("_path: " ${_path})
  message("_ih_path: " ${_ih_path})
endmacro(add_class)
