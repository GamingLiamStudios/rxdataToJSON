include(FetchContent)

# Include zlib-ng
FetchContent_Declare(
	zlib_ng
	GIT_REPOSITORY https://github.com/zlib-ng/zlib-ng
	GIT_TAG        6539b769e677bc1e6f4954d4457aba0cd138468b
)
FetchContent_GetProperties(zlib_ng)
if(NOT zlib_ng_POPULATED)
	FetchContent_Populate(zlib_ng)

	# Set custom variables, policies, etc.
	set(ZLIB_ENABLE_TESTS OFF)
	set(ZLIB_COMPAT OFF)
	set(WITH_NATIVE_INSTRUCTIONS ON)

	add_subdirectory(${zlib_ng_SOURCE_DIR} ${zlib_ng_BINARY_DIR})
endif()
