cmake_minimum_required (VERSION 2.6)
Project (awesome-wav)

#version number
set (awesome-wav_VERSION_MAJOR 1)
set (awesome-wav_VERSION_MINOR 0)
set (awesome-wav_VERSION_SUB_MINOR 0)

#output directory
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

#flags
set(CMAKE_CXX_FLAGS " -Wall -Wextra")
set(CMAKE_C_FLAGS " -Wall -Wextra")

#Platform specifics
IF(UNIX AND NOT APPLE)
	FIND_PACKAGE(ZLIB REQUIRED)
	INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -D_FILE_OFFSET_BITS=64")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -D_FILE_OFFSET_BITS=64")
ENDIF(UNIX AND NOT APPLE)

IF(APPLE)
	SET(ZLIB /usr)
	SET(ZLIB_INC "${ZLIB}/include")
	SET(ZLIB_LIBRARIES z)
ENDIF(APPLE)

IF(WIN32)
	SET(ZLIB ${LIBDIR}/src/win32/lib)
	SET(ZLIB_INC ${LIBDIR}/src/win32/include)
ENDIF(WIN32)

ADD_EXECUTABLE(awesome-wav 
${CMAKE_SOURCE_DIR}/src/compression/compress_util.cpp
${CMAKE_SOURCE_DIR}/src/compression/compress_util2.cpp
${CMAKE_SOURCE_DIR}/src/util.cpp
${CMAKE_SOURCE_DIR}/src/main.cpp
${CMAKE_SOURCE_DIR}/src/arg_processor.cpp
${CMAKE_SOURCE_DIR}/src/flac.cpp
${CMAKE_SOURCE_DIR}/src/wav.cpp
${CMAKE_SOURCE_DIR}/src/logger.cpp
${CMAKE_SOURCE_DIR}/src/crypt/sha2_util.cpp
${CMAKE_SOURCE_DIR}/src/crypt/aes_util.cpp
${CMAKE_SOURCE_DIR}/src/compression/file_compression.c
${CMAKE_SOURCE_DIR}/src/compression/quicklz.c
${CMAKE_SOURCE_DIR}/src/crypt/aes.c
${CMAKE_SOURCE_DIR}/src/crypt/sha2.c
)

target_link_libraries (awesome-wav z)


