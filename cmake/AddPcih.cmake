# Macro for adding a precompiled internal header file
macro(add_pcih _target _ih_path)
  string(REPLACE ".ih" ".ih.gch" _pcih_path ${_ih_path})
  get_filename_component(_ih_name ${_ih_path} NAME_WE)

  set(_ih_full_path ${CMAKE_CURRENT_SOURCE_DIR}/${_ih_path})
  set(_pcih_full_dir_path ${CMAKE_CURRENT_BINARY_DIR}/${_pcih_path})
  set(_pcih_full_path ${_pcih_full_dir_path}/${CMAKE_BUILD_TYPE}.c++)

  get_target_property(_COMPILE_FLAGS ${_target} COMPILE_FLAGS)

  if(CMAKE_BUILD_TYPE)
    string(TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" _cmake_cxx_flags_var)
  else(CMAKE_BUILD_TYPE)
    string(TOUPPER "CMAKE_CXX_FLAGS" _cmake_cxx_flags_var)
  endif(CMAKE_BUILD_TYPE)

  set(_cmake_cxx_flags ${${_cmake_cxx_flags_var}})
  list(APPEND _COMPILE_FLAGS ${_cmake_cxx_flags})

  get_directory_property(_directory_flags INCLUDE_DIRECTORIES)
  foreach(item ${_directory_flags})
    list(APPEND _COMPILE_FLAGS "-I${item}")
  endforeach(item)

  get_directory_property(_directory_flags DEFINITIONS)
  list(APPEND _COMPILE_FLAGS ${_directory_flags})

  separate_arguments(_COMPILE_FLAGS)

  add_custom_command(OUTPUT ${_pcih_full_path}
		     COMMAND mkdir -p ${_pcih_full_dir_path}
                     COMMAND ${CMAKE_CXX_COMPILER} -x c++-header ${_COMPILE_FLAGS} -c ${_ih_full_path} -o ${_pcih_full_path}
                     IMPLICIT_DEPENDS CXX ${_ih_full_path})

  set(_pcih_target "${_ih_name}_pcih")
  add_custom_target(${_pcih_target} DEPENDS ${_pcih_full_path})
  add_dependencies(${_target} ${_pcih_target})

endmacro(add_pcih)
