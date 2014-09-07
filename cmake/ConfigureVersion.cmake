find_package(Git)

#
## Query version information from repository and save in source code for subsequent compile
#
# the commit's SHA1, and whether the building workspace was dirty or not
execute_process(COMMAND
  "${GIT_EXECUTABLE}" describe --match=NeVeRmAtCh --always --abbrev=40 --dirty
  WORKING_DIRECTORY "${VERSION_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_SHA1
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
# the date of the commit
execute_process(COMMAND
  "${GIT_EXECUTABLE}" log -1 --format=%ad --date=local
  WORKING_DIRECTORY "${VERSION_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_DATE
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
# the subject of the commit
execute_process(COMMAND
  "${GIT_EXECUTABLE}" log -1 --format=%s
  WORKING_DIRECTORY "${VERSION_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_COMMIT_SUBJECT
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
# the host name of the machine making the build
execute_process(COMMAND
  hostname
  WORKING_DIRECTORY "${VERSION_SOURCE_DIR}"
  OUTPUT_VARIABLE BUILT_ON_HOST_NAME
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

# generate version.cc
configure_file("${VERSION_SOURCE_DIR}/Version/version.cc.in" "${VERSION_BINARY_DIR}/Version/version.cc" @ONLY)
configure_file("${VERSION_SOURCE_DIR}/Version/version.hh" "${VERSION_BINARY_DIR}/Version/version.hh" @ONLY)
