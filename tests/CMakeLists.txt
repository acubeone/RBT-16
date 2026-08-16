enable_testing()

include(FetchContent)
FetchContent_Declare(
	unity
	GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
	GIT_TAG        v2.6.1
)
FetchContent_MakeAvailable(unity)

add_subdirectory(vendor/Moira)

include("../cmake/base.cmake")
include("../cmake/warnings.cmake")

function(add_test_executable test)
	cmake_parse_arguments(
		PARSE_ARGV 1 TEST
		""
		""
		"SOURCES;INCLUDE_DIRS"
	)

	if(NOT TEST_SOURCES OR TEST_SOURCES STREQUAL "")
		message(WARNING "Not sources for test target ${test}")
	endif()

	add_executable(${test})
	set_default_warnings(${test})
	enable_tools(${test})

	target_compile_features(${test} PRIVATE c_std_23)

	if(TEST_INCLUDE_DIRS)
		target_include_directories(${test} PRIVATE ${TEST_INCLUDE_DIRS})
	endif()
	target_include_directories(${test} PRIVATE "../src")

	target_sources(${test} PRIVATE ${TEST_SOURCES})

	# Enable code sanitizers
	if(ENABLE_SANITIZERS)
		target_compile_options(${test} PRIVATE ${ASAN_SANITIZER_FLAGS})
		target_link_options(${test} PRIVATE ${ASAN_SANITIZER_FLAGS})
	endif()

	target_link_libraries(
		${test}
		PRIVATE
			rbt-core
			unity::framework
	)

	add_test(RBT_TESTING ${test})
endfunction()

add_test_executable(
	test_ea
	SOURCES
		"src/cpu/test_ea.c"
)

add_test_executable(
	test_bus
	SOURCES
		"src/cpu/test_bus.c"
)

add_test_executable(
	test_decode
	SOURCES
		"src/cpu/test_decode.c"
)

add_test_executable(
	test_opcodes
	SOURCES
		"src/cpu/test_opcodes.c"
	INCLUDE_DIRS
		"src/"
)

add_library(moira_wrapper STATIC "src/moira_wrapper.cpp")
target_compile_features(moira_wrapper PRIVATE cxx_std_23)
target_include_directories(moira_wrapper PUBLIC "vendor/Moira")
add_test_executable(
	test_runner
	SOURCES
		"src/cpu/test_runner.c"
	INCLUDE_DIRS
		"src/"
		"vendor/"
)
target_link_libraries(test_runner PRIVATE moira_wrapper)
