project(XKey)
cmake_minimum_required(VERSION 2.6)

set(CMAKE_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

add_definitions( -std=c++11 )

find_package(OpenSSL)

set(Boost_USE_STATIC_LIBS True)
find_package(Boost REQUIRED COMPONENTS program_options)

# Boost_program_options_LIBRARY

include_directories(SYSTEM ${OPENSSL_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/ ${Boost_INCLUDE_DIR} )

set(XKeyLibraries ${OPENSSL_CRYPTO_LIBRARIES} )

set(JsonDir ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/json/lib_json)
set(TP_Json_SRCS ${JsonDir}/json_reader.cpp ${JsonDir}/json_value.cpp ${JsonDir}/json_writer.cpp) 

set(SrcDir ${CMAKE_CURRENT_SOURCE_DIR}/src )
set(XKey_SRCS ${SrcDir}/XKey.cpp ${SrcDir}/CryptStream.cpp
	${SrcDir}/XKeyGenerator.cpp ${SrcDir}/XKeyJsonSerialization.cpp )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( include )

add_executable(XKey ${SrcDir}/main.cpp ${XKey_SRCS} ${TP_Json_SRCS} )
target_link_libraries(XKey ${XKeyLibraries} ${Boost_PROGRAM_OPTIONS_LIBRARY}  )
install(TARGETS XKey RUNTIME DESTINATION bin)

add_executable(XTest EXCLUDE_FROM_ALL ${SrcDir}/test.cpp ${XKey_SRCS} ${TP_Json_SRCS} )
target_link_libraries(XTest ${XKeyLibraries} ${Boost_PROGRAM_OPTIONS_LIBRARY}  )

add_subdirectory(src/qt)
