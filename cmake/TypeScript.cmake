macro(add_typescript_target)
  set(_outFiles)
  foreach(_tlSource ${ARGN})
    get_filename_component(_path ${_tlSource} PATH)
    get_filename_component(_name ${_tlSource} NAME_WE)

#    message(${_path})
#    message(${_name})

    set(_jsOutput "${_path}/${_name}.js")
#    message(${_jsOutput})

    add_custom_command(OUTPUT ${_jsOutput}
      COMMAND tsc --sourcemap --module amd ${_tlSource}
      DEPENDS ${_tlSource}
      VERBATIM)
    list(APPEND _outFiles ${_jsOutput})
  endforeach()

#  message("TypeScript output: ${_outFiles}")
  add_custom_target(typescript DEPENDS ${_outFiles})
endmacro(add_typescript_target)
