cmake_minimum_required (VERSION 2.8)
project (BoldHumanoid)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)

set(BUILDRUN_UNIT_TEST ON CACHE BOOL "Choose whether to build and run unit tests")
set(BUILD_TOOLS ON CACHE BOOL "Choose whether to build tools")
set(BUILD_BOLDPY OFF CACHE BOOL "Choose whether to Python library")
set(PRECOMPILE_HEADERS OFF CACHE BOOL "Choose whether to precompile headers")
set(EIGEN_VECTORIZE OFF CACHE BOOL "Choose whether to enable Eigen vectorization")
set(INCLUDE_ASSERTIONS ON CACHE BOOL "Choose whether to include assertions in compiled code")
set(TARGET_ATOM ON CACHE BOOL "Choose whether to optimize for atom")
set(BUILD_CORE_COUNT 1 CACHE STRING "Number of parallel jobs to run when building")

include(ExternalProject)
include(AddPcih)
include(AddClass)

set(VERSION_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(VERSION_BINARY_DIR ${CMAKE_BINARY_DIR})
include(ConfigureVersion)

find_package(Eigen3 REQUIRED)
find_package(OpenCV REQUIRED core highgui imgproc)
find_package(SigC++ REQUIRED)

set(JOYSTICKDIR ${CMAKE_SOURCE_DIR}/joystick)
set(LIBWEBSOCKETDIR ${CMAKE_SOURCE_DIR}/libwebsockets)
set(RAPIDJSONDIR ${CMAKE_SOURCE_DIR}/rapidjson)

#
## Compile libwebsockets as an external project
#
ExternalProject_Add(libwebsockets_lib
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${LIBWEBSOCKETDIR}
  BINARY_DIR ${LIBWEBSOCKETDIR}
  CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DLWS_WITHOUT_TESTAPPS=ON -DLWS_WITHOUT_CLIENT=ON -DLWS_WITHOUT_DAEMONIZE=ON -DLWS_WITH_SSL=OFF
  INSTALL_COMMAND ""
)

#
## Boldhumanoid sources
#
# Only add sources here that are not in class directory structure! Add classes separately below
#
set(BOLDHUMANOID_SOURCES
  ${JOYSTICKDIR}/joystick.cc
)

#
## Custom target to ensure version is reconfigured at build time
#
add_custom_target(
  version
  cmake -DVERSION_SOURCE_DIR=${VERSION_SOURCE_DIR} -DVERSION_BINARY_DIR=${VERSION_BINARY_DIR} -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ConfigureVersion.cmake"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# explicitly provide the dependency for version.cc, in case it has been deleted
list(APPEND BOLDHUMANOID_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/Version/version.cc")

#
## Include directories
#
set(BOLDHUMANOID_INCLUDE_DIRS
  ${JOYSTICKDIR}
  ${LIBWEBSOCKETDIR}/lib
  ${EIGEN3_INCLUDE_DIR}
  ${SigC++_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIR}
  ${RAPIDJSONDIR}/include
  )

include_directories(${BOLDHUMANOID_INCLUDE_DIRS})

#
## Link directories
#
set(BOLDHUMANOID_LINK_DIRS ${LIBWEBSOCKETDIR}/lib)
link_directories(${BOLDHUMANOID_LINK_DIRS})

#
## Parallel compilation settings
#

if (BUILD_CORE_COUNT)
    set(CMAKE_MAKE_PROGRAM "${CMAKE_MAKE_PROGRAM}") # -j${BUILD_CORE_COUNT}
endif()

#
## Additional compile flag settings
#

if(EIGEN_VECTORIZE)
  set(BOLDHUMANOID_COMPILE_FLAGS "-std=c++11 -fPIC -pipe -Wall -Wno-unused-local-typedefs -Wno-multichar")
else(EIGEN_VECTORIZE)
  set(BOLDHUMANOID_COMPILE_FLAGS "-std=c++11 -fPIC -pipe -DEIGEN_DONT_ALIGN -DEIGEN_DONT_VECTORIZE -Wall -Wno-unused-local-typedefs -Wno-multichar")
endif(EIGEN_VECTORIZE)

if(TARGET_ATOM)
  set(BOLDHUMANOID_COMPILE_FLAGS "${BOLDHUMANOID_COMPILE_FLAGS} -march=atom -mtune=atom")
endif(TARGET_ATOM)

if(INCLUDE_ASSERTIONS)
  set(BOLDHUMANOID_COMPILE_FLAGS "${BOLDHUMANOID_COMPILE_FLAGS} -DINCLUDE_ASSERTIONS")
endif(INCLUDE_ASSERTIONS)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(BOLDHUMANOID_COMPILE_FLAGS "${BOLDHUMANOID_COMPILE_FLAGS} -Wno-deprecated -Wno-unused-private-field -Wno-unknown-warning-option")
endif()

#
## Class source files
#

add_class(BOLDHUMANOID
  ./Agent/agent.cc
  ./Agent/registerStateTypes.cc
  ./Agent/run.cc
  ./Agent/think.cc
)

add_class(BOLDHUMANOID
  ./AgentPosition/agentWorldTransform.cc
  ./AgentPosition/worldAgentTransform.cc
)

add_class(BOLDHUMANOID
  ./BehaviourControl/behaviourcontrol.cc
)

add_class(BOLDHUMANOID
  ./Balance/GyroBalance/gyrobalance.cc
  ./Balance/OrientationBalance/orientationbalance.cc
)

add_class(BOLDHUMANOID
  ./BodyControl/bodycontrol.cc
)

add_class(BOLDHUMANOID
  ./BodyModel/DarwinBodyModel/darwinbodymodel.cc
)

add_class(BOLDHUMANOID
  ./Camera/Camera.cc
  ./Camera/capture.cc
  ./Camera/createControls.cc
  ./Camera/createFormats.cc
  ./Camera/initMemoryMapping.cc
  ./Camera/logFrameIntervalDetails.cc
  ./Camera/open.cc
  ./Camera/pixelFormatRequestSize.cc
  ./Camera/startCapture.cc
  ./Camera/stopCapture.cc
)

add_class(BOLDHUMANOID
  ./CameraModel/CameraModel.cc
  ./CameraModel/directionForPixel.cc
  ./CameraModel/pixelForDirection.cc
)

add_class(BOLDHUMANOID
  ./Clock/clock.cc
)

add_class(BOLDHUMANOID
  ./CM730/cm730.cc
)

add_class(BOLDHUMANOID
  ./CM730CommsModule/MX28HealthChecker/mx28healthchecker.cc
)

add_class(BOLDHUMANOID
  ./CM730Platform/CM730Linux/cm730linux.cc
)

add_class(BOLDHUMANOID
  ./CM730Snapshot/CM730Snapshot.cc
)

add_class(BOLDHUMANOID
  ./Colour/bgr2hsv.cc
  ./Colour/hsv2bgr.cc
  ./Colour/hsvRange.cc
  ./Colour/knownColours.cc
  ./Colour/operator_ins.cc
  ./Colour/YCbCr.cc
)

add_class(BOLDHUMANOID
  ./Config/config.cc
)

add_class(BOLDHUMANOID
  ./Debugger/debugger.cc
)

add_class(BOLDHUMANOID
  ./DataStreamer/callbackCamera.cc
  ./DataStreamer/callbackControl.cc
  ./DataStreamer/callbackHttp.cc
  ./DataStreamer/callbackLog.cc
  ./DataStreamer/callbackState.cc
  ./DataStreamer/camerasession.cc
  ./DataStreamer/DataStreamer.cc
  ./DataStreamer/jsonsession.cc
  ./DataStreamer/processCommand.cc
  ./DataStreamer/run.cc
  ./DataStreamer/stop.cc
  ./DataStreamer/streamImage.cc
  ./DataStreamer/writeControlSyncJson.cc
  ./DataStreamer/writeSettingUpdateJson.cc
)

add_class(BOLDHUMANOID
  ./DrawBridgeComms/drawbridgecomms.cc
)

add_class(BOLDHUMANOID
  ./Drawing/drawing.cc
)

add_class(BOLDHUMANOID
  ./FieldMap/FieldMap.cc
)

add_class(BOLDHUMANOID
  ./GameStateReceiver/gamestatereceiver.cc
)

add_class(BOLDHUMANOID
  ./geometry/LineSegment/LineSegment2/LineSegment2i/linesegment2i.cc
)

add_class(BOLDHUMANOID
  ./HoughLineAccumulator/houghlineaccumulator.cc
)

add_class(BOLDHUMANOID
  ./HoughLineExtractor/findLines.cc
  ./HoughLineExtractor/findMaxLine.cc
)

add_class(BOLDHUMANOID
  ./ImageCodec/JpegCodec/jpegcodec.cc
  ./ImageCodec/PngCodec/pngcodec.cc
)

add_class(BOLDHUMANOID
  ./ImageLabeller/ImageLabeller.cc
)

add_class(BOLDHUMANOID
  ./ImagePassHandler/BlobDetectPass/detectBlobs.cc
  ./ImagePassHandler/BlobDetectPass/runSetToBlob.cc
  ./ImagePassHandler/BlobDetectPass/BlobDetectPass.cc
)

add_class(BOLDHUMANOID
  ./ImagePassHandler/CartoonPass/cartoonpass.cc
)

add_class(BOLDHUMANOID
  ./ImagePassHandler/FieldEdgePass/fieldedgepass.cc
)

add_class(BOLDHUMANOID
  ./ImagePassHandler/FieldEdgePass/CompleteFieldEdgePass/completefieldedgepass.cc
)

add_class(BOLDHUMANOID
  ./ImagePassHandler/FieldEdgePass/PeriodicFieldEdgePass/periodicfieldedgepass.cc
)

add_class(BOLDHUMANOID
  ./ImagePassHandler/LabelCountPass/labelcountpass.cc
)

add_class(BOLDHUMANOID
  ./ImagePassHandler/LineDotPass/linedotpass.cc
)

add_class(BOLDHUMANOID
  ./ImagePassRunner/imagepassrunner.cc
)

add_class(BOLDHUMANOID
  ./ImageSampleMap/imagesamplemap.cc
)

add_class(BOLDHUMANOID
  ./IntegralImage/integralimage.cc
)

add_class(BOLDHUMANOID
  ./JointId/jointid.cc
)

add_class(BOLDHUMANOID
  ./Kick/kick.cc
)

add_class(BOLDHUMANOID
  ./LabelTeacher/labelteacher.cc
)

add_class(BOLDHUMANOID
  ./LineFinder/RandomPairLineFinder/findLineSegments.cc
)

add_class(BOLDHUMANOID
  ./LineFinder/MaskWalkLineFinder/findLineSegments.cc
  ./LineFinder/MaskWalkLineFinder/MaskWalkLineFinder.cc
  ./LineFinder/MaskWalkLineFinder/rebuild.cc
  ./LineFinder/MaskWalkLineFinder/walkLine.cc
)

add_class(BOLDHUMANOID
  ./LineFinder/ScanningLineFinder/ScanningLineFinder.cc
  ./LineFinder/ScanningLineFinder/findLineSegments.cc
)

add_class(BOLDHUMANOID
  ./LineJunctionFinder/findLineJunctions.cc
  ./LineJunctionFinder/tryFindLineJunction.cc
)

add_class(BOLDHUMANOID
  ./Localiser/Localiser.cc
  ./Localiser/generateState.cc
  ./Localiser/predict.cc
  ./Localiser/update.cc
  ./Localiser/updateSmoothedPos.cc
  ./Localiser/updateStateObject.cc
)

add_class(BOLDHUMANOID
  ./LUTBuilder/lutbuilder.cc
)

add_class(BOLDHUMANOID
  ./Math/alignUp.cc
  ./Math/createRng.cc
  ./Math/findPerpendicularVector.cc
  ./Math/intersectRayWithGroundPlane.cc
  ./Math/intersectRayWithPlane.cc
  ./Math/linePointClosestToPoint.cc
  ./Math/smallestAngleBetween.cc
)

add_class(BOLDHUMANOID
  ./MessageCounter/messagecounter.cc
)

add_class(BOLDHUMANOID
  ./mitecom/mitecom-handler.cpp
  ./mitecom/mitecom-network.cpp
  ./mitecom/mitecom-roledecider.cpp
)

add_class(BOLDHUMANOID
  ./MotionLoop/motionloop.cc
)

add_class(BOLDHUMANOID
  ./MotionModule/HeadModule/headmodule.cc
)

add_class(BOLDHUMANOID
  ./MotionModule/MotionScriptModule/motionscriptmodule.cc
)

add_class(BOLDHUMANOID
  ./MotionModule/WalkModule/walkmodule.cc
)

add_class(BOLDHUMANOID
  ./MotionModule/motionmodule.cc
)

add_class(BOLDHUMANOID
  ./MotionScript/motionscript.cc
)

add_class(BOLDHUMANOID
  ./MotionScriptRunner/motionscriptrunner.cc
)

add_class(BOLDHUMANOID
  ./MotionTask/motiontask.cc
)

add_class(BOLDHUMANOID
  ./MotionTaskScheduler/motiontaskscheduler.cc
)

add_class(BOLDHUMANOID
  ./MX28Snapshot/MX28Snapshot.cc
)

add_class(BOLDHUMANOID
  ./MX28/conversions.cc
)

add_class(BOLDHUMANOID
  ./Option/ActionOption/actionoption.cc
)

add_class(BOLDHUMANOID
  ./Option/ApproachBall/approachball.cc
)

add_class(BOLDHUMANOID
  ./Option/AwaitTheirKickOff/awaittheirkickoff.cc
)

add_class(BOLDHUMANOID
  ./Option/AtBall/atball.cc
)

add_class(BOLDHUMANOID
  ./Option/CircleBall/circleball.cc
)

add_class(BOLDHUMANOID
  ./Option/FSMOption/fsmoption.cc
  ./Option/FSMOption/fsmstate.cc
  ./Option/FSMOption/fsmtransition.cc
  ./Option/FSMOption/runPolicy.cc
  ./Option/FSMOption/toDot.cc
)

add_class(BOLDHUMANOID
  ./Option/GameOver/gameover.cc
)

add_class(BOLDHUMANOID
  ./Option/GetUpOption/getupoption.cc
)

add_class(BOLDHUMANOID
  ./Option/KeepPosition/keepposition.cc
)

add_class(BOLDHUMANOID
  ./Option/LocateBall/locateball.cc
)

add_class(BOLDHUMANOID
  ./Option/LookAround/lookaround.cc
)

add_class(BOLDHUMANOID
  ./Option/LookAtBall/lookatball.cc
)

add_class(BOLDHUMANOID
  ./Option/LookAtFeet/lookatfeet.cc
)

add_class(BOLDHUMANOID
  ./Option/MotionScriptOption/motionscriptoption.cc
)

add_class(BOLDHUMANOID
  ./Option/OdoWalkTo/odowalkto.cc
)

add_class(BOLDHUMANOID
  ./Option/SearchBall/runPolicy.cc
)

add_class(BOLDHUMANOID
  ./Option/SequenceOption/sequenceoption.cc
)

add_class(BOLDHUMANOID
  ./Option/SquareDance/squaredance.cc
)

add_class(BOLDHUMANOID
  ./Option/StopWalking/stopwalking.cc
)

add_class(BOLDHUMANOID
  ./Option/TrackBall/trackball.cc
)

add_class(BOLDHUMANOID
  ./Option/Support/support.cc
)

add_class(BOLDHUMANOID
  ./Option/UntilShutdown/untilshutdown.cc
)

add_class(BOLDHUMANOID
  ./Option/WaitForWhistle/waitforwhistle.cc
)

add_class(BOLDHUMANOID
  ./Option/WalkTo/walkto.cc
)

add_class(BOLDHUMANOID
  ./OptionTree/optiontree.cc
)

add_class(BOLDHUMANOID
  ./OptionTreeBuilder/AdHocOptionTreeBuilder/buildBallCirclerFsm.cc
  ./OptionTreeBuilder/AdHocOptionTreeBuilder/buildKeeperFsm.cc
  ./OptionTreeBuilder/AdHocOptionTreeBuilder/buildKickLearnerFsm.cc
  ./OptionTreeBuilder/AdHocOptionTreeBuilder/buildPauseStateFsm.cc
  ./OptionTreeBuilder/AdHocOptionTreeBuilder/buildPenaltyKeeperFsm.cc
  ./OptionTreeBuilder/AdHocOptionTreeBuilder/buildPlayModeFsm.cc
  ./OptionTreeBuilder/AdHocOptionTreeBuilder/buildStayStandingFsm.cc
  ./OptionTreeBuilder/AdHocOptionTreeBuilder/buildStrikerFsm.cc
  ./OptionTreeBuilder/AdHocOptionTreeBuilder/buildSupporterFsm.cc
  ./OptionTreeBuilder/AdHocOptionTreeBuilder/buildTree.cc
  ./OptionTreeBuilder/AdHocOptionTreeBuilder/buildWhistleListenerFsm.cc
)

add_class(BOLDHUMANOID
  ./Painter/painter.cc
)

add_class(BOLDHUMANOID
  ./PixelFilterChain/applyFilters.cc
)

add_class(BOLDHUMANOID
  ./PixelLabel/pixellabel.cc
)

add_class(BOLDHUMANOID
  ./PixelLabel/HistogramPixelLabel/histogrampixellabel.cc
)

add_class(BOLDHUMANOID
  ./RemoteControl/remotecontrol.cc
)

add_class(BOLDHUMANOID
  ./RobotisMotionFile/RobotisMotionFile.cc
)

add_class(BOLDHUMANOID
  ./RoleDecider/roledecider.cc
)

add_class(BOLDHUMANOID
  ./Setting/setting.cc
  ./Setting/setting-implementations.cc
)

add_class(BOLDHUMANOID
  ./Spatialiser/Spatialiser.cc
  ./Spatialiser/updateZeroGroundPixelTransform.cc
  ./Spatialiser/findGroundPixelTransform.cc
  ./Spatialiser/findGroundPointForPixel.cc
  ./Spatialiser/findPixelForAgentPoint.cc
  ./Spatialiser/findHorizonForColumn.cc
  ./Spatialiser/updateCameraToAgent.cc
  ./Spatialiser/updateAgentToWorld.cc
)

add_class(BOLDHUMANOID
  ./State/state.cc
)

add_class(BOLDHUMANOID
  ./StateObject/AgentFrameState/agentframestate.cc
)

add_class(BOLDHUMANOID
  ./StateObject/AudioPowerSpectrumState/audiopowerspectrumstate.cc
)

add_class(BOLDHUMANOID
  ./StateObject/BodyControlState/bodycontrolstate.cc
)

add_class(BOLDHUMANOID
  ./StateObject/BodyState/bodystate.cc
  ./StateObject/BodyState/initialise.cc
)

add_class(BOLDHUMANOID
  ./StateObject/GameState/gamestate.cc
)

add_class(BOLDHUMANOID
  ./StateObject/OrientationState/orientationstate.cc
)

add_class(BOLDHUMANOID
  ./StateObject/StationaryMapState/stationarymapstate.cc
)

add_class(BOLDHUMANOID
  ./StateObject/TeamState/teamstate.cc
)

add_class(BOLDHUMANOID
  ./StateObject/WalkState/walkstate.cc
)

add_class(BOLDHUMANOID
  ./StateObserver/stateobserver.cc
)

add_class(BOLDHUMANOID
  ./StateObserver/ButtonObserver/buttonobserver.cc
)

add_class(BOLDHUMANOID
  ./StateObserver/FallDetector/falldetector.cc
  ./StateObserver/FallDetector/AccelerometerFallDetector/accelerometerfalldetector.cc
  ./StateObserver/FallDetector/OrientationFallDetector/orientationfalldetector.cc
)

add_class(BOLDHUMANOID
  ./StateObserver/GyroCalibrator/gyrocalibrator.cc
)

add_class(BOLDHUMANOID
  ./StateObserver/HandlerHelper/handlerhelper.cc
)

add_class(BOLDHUMANOID
  ./StateObserver/HealthAndSafety/healthandsafety.cc
)

add_class(BOLDHUMANOID
  ./StateObserver/JamDetector/jamdetector.cc
)

add_class(BOLDHUMANOID
  ./StateObserver/Odometer/odometer.cc
)

add_class(BOLDHUMANOID
  ./StateObserver/OpenTeamCommunicator/openteamcommunicator.cc
)

add_class(BOLDHUMANOID
  ./StateObserver/OrientationTracker/orientationtracker.cc
)

add_class(BOLDHUMANOID
  ./StateObserver/StationaryMapper/stationarymapper.cc
)

add_class(BOLDHUMANOID
  ./StateObserver/SuicidePill/suicidepill.cc
)

add_class(BOLDHUMANOID
  ./StateObserver/Vocaliser/vocaliser.cc
)

add_class(BOLDHUMANOID
  ./ThreadUtil/statics.cc
)

add_class(BOLDHUMANOID
  ./UDPSocket/udpsocket.cc
)

add_class(BOLDHUMANOID
  ./VisualCortex/canBlobBeBall.cc
  ./VisualCortex/canBlobBeGoal.cc
  ./VisualCortex/canBlobBePlayer.cc
  ./VisualCortex/detectBall.cc
  ./VisualCortex/detectGoal.cc
  ./VisualCortex/detectPlayers.cc
  ./VisualCortex/integrateImage.cc
  ./VisualCortex/shouldMergeBallBlobs.cc
  ./VisualCortex/saveImage.cc
  ./VisualCortex/streamDebugImage.cc
  ./VisualCortex/VisualCortex.cc
)

add_class(BOLDHUMANOID
  ./Voice/voice.cc
)

add_class(BOLDHUMANOID
  ./WalkAttitude/walkattitude.cc
)

add_class(BOLDHUMANOID
  ./WalkEngine/walkengine.cc
  ./WalkEngine/Matrix.cpp
  ./WalkEngine/Point.cpp
  ./WalkEngine/Vector.cpp
)

add_class(BOLDHUMANOID
  ./WhistleListener/whistlelistener.cc
)

add_class(BOLDHUMANOID
  ./util/ccolor.cc
  ./util/log.cc
  ./util/loop.cc
  ./util/windowfunctions.cc
)

#
## Reusable object files target
#
add_library(boldhumanoid_objects OBJECT
  ${BOLDHUMANOID_SOURCES}
)

foreach(pch_target ${BOLDHUMANOID_PCH_TARGETS})
  add_dependencies(boldhumanoid_objects ${pch_target})
endforeach(pch_target)

#
## Main executable
#
add_executable(boldhumanoid
  ./main.cc
  $<TARGET_OBJECTS:boldhumanoid_objects>
)

#
## Link executable to libraries
#
set(BOLDHUMANOID_LINK_LIBRARIES ${SigC++_LIBRARIES} turbojpeg opencv_core opencv_imgproc ssl crypto libwebsockets.a pthread rt asound fftw3f espeak z png)

target_link_libraries(boldhumanoid ${BOLDHUMANOID_LINK_LIBRARIES})

#
## Additional compile flags
# TODO: fix things so we can use Eigen vectorization
#
set_target_properties(boldhumanoid_objects PROPERTIES COMPILE_FLAGS ${BOLDHUMANOID_COMPILE_FLAGS})
set_target_properties(boldhumanoid PROPERTIES COMPILE_FLAGS ${BOLDHUMANOID_COMPILE_FLAGS})

#
## Target dependencies
#
add_dependencies(boldhumanoid libwebsockets_lib)
add_dependencies(boldhumanoid_objects version)

#
## Drawbridge
#

add_executable(drawbridge
  ./drawbridge.cc
  $<TARGET_OBJECTS:boldhumanoid_objects>
)

target_link_libraries(drawbridge ${BOLDHUMANOID_LINK_LIBRARIES})
set_target_properties(drawbridge PROPERTIES COMPILE_FLAGS ${BOLDHUMANOID_COMPILE_FLAGS})

#
## Deployment
#

set(BOLDHUMANOID_DEPLOY_FILES
  $<TARGET_FILE:boldhumanoid>
  ${CMAKE_SOURCE_DIR}/*.ini
  ${CMAKE_SOURCE_DIR}/www
  ${CMAKE_SOURCE_DIR}/motion_4096.bin
)

#
## CTAGS target (for emacs users)
#
set(_command find ${CMAKE_SOURCE_DIR} -regextype posix-egrep -regex ".*((hh)|(cc))$" -print | etags -o ${CMAKE_SOURCE_DIR}/TAGS -)
add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/TAGS
  COMMAND ${_command}
  IMPLICIT_DEPENDS ${BOLDHUMANOID_SORCES}
  VERBATIM
  )
add_custom_target(tags DEPENDS ${CMAKE_SOURCE_DIR}/TAGS)

#
## SWIG/Python
#
#if (BUILD_BOLDPY)
#  add_subdirectory(swig)
#endif (BUILD_BOLDPY)

#
## Build and run test
#
if (BUILDRUN_UNIT_TEST)
  add_subdirectory(test)
#  add_dependencies(boldhumanoid runtests)
endif (BUILDRUN_UNIT_TEST)

#
## Build tools
#
if (BUILD_TOOLS)
  add_subdirectory(tools)
endif (BUILD_TOOLS)

#
## Round Table target
#
add_custom_target(roundtable ALL gulp dist WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/www)

set(JSON_FILES
  ${CMAKE_SOURCE_DIR}/configuration-berlin.json
  ${CMAKE_SOURCE_DIR}/configuration-drews.json
  ${CMAKE_SOURCE_DIR}/configuration-german-open-2014.json
  ${CMAKE_SOURCE_DIR}/configuration-hamburg.json
  ${CMAKE_SOURCE_DIR}/configuration-hatfield.json
  ${CMAKE_SOURCE_DIR}/configuration-iran-open-2014.json
  ${CMAKE_SOURCE_DIR}/configuration-metadata.json
  ${CMAKE_SOURCE_DIR}/configuration-rc2014.json
  ${CMAKE_SOURCE_DIR}/configuration-team.json
)

foreach(json_file ${JSON_FILES})
  add_Custom_command(TARGET boldhumanoid PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${json_file} $<TARGET_FILE_DIR:boldhumanoid>)
endforeach(json_file)

#
## For CLion users. See https://youtrack.jetbrains.com/issue/CPP-270
#
file(GLOB_RECURSE clion_all_headers
  ${CMAKE_SOURCE_DIR}/*.hh
  ${CMAKE_SOURCE_DIR}/*.ih
  ${CMAKE_SOURCE_DIR}/www/scripts/*.ts
  ${CMAKE_SOURCE_DIR}/configuration-*.json
  ${CMAKE_SOURCE_DIR}/motionscripts/*.json
  ${CMAKE_SOURCE_DIR}/upstart.conf
  ${CMAKE_SOURCE_DIR}/bh.d/*)
add_custom_target(all_clion SOURCES ${clion_all_headers})
