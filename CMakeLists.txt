cmake_minimum_required(VERSION 3.29)
project(
	rbt
	VERSION 0.1.0
	LANGUAGES C CXX
)

option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors." ON)
option(ENABLE_SANITIZERS "" ON)
option(BUILD_TESTS "Build unit tests" OFF)

include("cmake/base.cmake")
include("cmake/warnings.cmake")

set(RBT_LIBCORE ${PROJECT_NAME}-core)

add_library(${RBT_LIBCORE} STATIC)
set_default_warnings(${RBT_LIBCORE})
enable_tools(${RBT_LIBCORE})

# Enable code sanitizers
if(ENABLE_SANITIZERS)
	target_compile_options(${RBT_LIBCORE} PRIVATE ${ASAN_SANITIZER_FLAGS})
	target_link_options(${RBT_LIBCORE} PRIVATE ${ASAN_SANITIZER_FLAGS})
endif()

target_compile_features(
	${RBT_LIBCORE}
	PRIVATE
		c_std_23
)

target_sources(
	${RBT_LIBCORE}
	PRIVATE
		"src/cpu/bus.c"
		"src/cpu/cpu.c"
		"src/cpu/decode.c"
		"src/cpu/effective_address.c"
		"src/cpu/timing.c"
		"src/error.c"
		"src/helpers.c"
)

target_include_directories(
	${RBT_LIBCORE}
	PUBLIC
		"${CMAKE_SOURCE_DIR}/include"
	PRIVATE
		"${CMAKE_SOURCE_DIR}/src"
)

if(BUILD_TESTS)
	add_subdirectory("${CMAKE_SOURCE_DIR}/tests")
endif()
