# This builds the dependencies, then builds all backends
cmake_minimum_required(VERSION 3.13.4)
project(feathergui)

if (MSVC)
  set(BIN_DIR "${CMAKE_SOURCE_DIR}/bin-x64/MinSizeRel")
else()
  set(BIN_DIR "${CMAKE_SOURCE_DIR}/bin-x64/deps")
endif()

set(DEPS_DIR "${CMAKE_SOURCE_DIR}/deps")
file(MAKE_DIRECTORY ${BIN_DIR})

macro(buildfix target type)
  file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/${target}")
  if (MSVC)
    execute_process(COMMAND_ECHO STDOUT WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/${target}" COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" ${DEPS_DIR}/${target} ${ARGN})
    set(ENV{_CL_} "/MT")
    execute_process(WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/${target}" COMMAND ${CMAKE_COMMAND} --build . --config MinSizeRel)
    execute_process(WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/${target}" COMMAND ${CMAKE_COMMAND} --install . --config MinSizeRel --prefix "${BIN_DIR}/${target}")
    set(ENV{_CL_} "/MTd")
    execute_process(WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/${target}" COMMAND ${CMAKE_COMMAND} --build . --config Debug)
    execute_process(WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/${target}" COMMAND ${CMAKE_COMMAND} --install . --config Debug --prefix "${CMAKE_SOURCE_DIR}/bin-x64/Debug/${target}")
    unset(ENV{_CL_})
  else()
    execute_process(COMMAND_ECHO STDOUT WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/${target}" COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" -DCMAKE_BUILD_TYPE=MinSizeRel ${DEPS_DIR}/${target} ${ARGN})
    execute_process(WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/${target}" COMMAND ${CMAKE_COMMAND} --build .)
    execute_process(WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/${target}" COMMAND ${CMAKE_COMMAND} --install . --prefix "${BIN_DIR}/${target}")
  endif()
endmacro()

buildfix("glfw" -DGLFW_BUILD_DOCS=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_EXAMPLES=OFF "-DCMAKE_CXX_STANDARD=17" "-DCMAKE_CXX_FLAGS=/D _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS /D _CRT_SECURE_NO_WARNINGS")
buildfix("freetype2" "-DCMAKE_C_FLAGS=/D _CRT_SECURE_NO_WARNINGS /D FT_CONFIG_OPTION_SUBPIXEL_RENDERING")
buildfix("SOIL" ${SOIL_FLAGS} -DSOIL_BUILD_TESTS=OFF "-DCMAKE_C_FLAGS=/D _CRT_SECURE_NO_WARNINGS")
buildfix("harfbuzz" ${HARFBUZZ_FLAGS} "-DCMAKE_CXX_STANDARD=17" "-DCMAKE_CXX_FLAGS=/D _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS /D _CRT_SECURE_NO_WARNINGS" -DHB_HAVE_FREETYPE=ON -DCMAKE_PREFIX_PATH=${BIN_DIR}/freetype2)

list(APPEND CMAKE_MODULE_PATH ${DEPS_DIR})
list(APPEND CMAKE_PREFIX_PATH ${BIN_DIR}/glfw ${BIN_DIR}/freetype2 ${BIN_DIR}/SOIL ${BIN_DIR}/harfbuzz)

add_subdirectory("${CMAKE_SOURCE_DIR}/fgOpenGL")