project(XKey)
cmake_minimum_required(VERSION 2.6)


set(CMAKE_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

add_definitions( -std=c++0x )

find_package(OpenSSL)

include_directories( ${OPENSSL_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/ )

set(XKeyLibraries ${OPENSSL_CRYPTO_LIBRARIES} boost_program_options )

set(JsonDir ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/json/lib_json)
set(TP_Json_SRCS ${JsonDir}/json_reader.cpp ${JsonDir}/json_value.cpp ${JsonDir}/json_writer.cpp) 

set(SrcDir ${CMAKE_CURRENT_SOURCE_DIR}/src )
set(XKey_SRCS ${SrcDir}/XKey.cpp ${SrcDir}/CryptStream.cpp )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )

add_executable(XKey ${SrcDir}/main.cpp ${XKey_SRCS} ${TP_Json_SRCS} )
target_link_libraries(XKey ${XKeyLibraries} )
install(TARGETS XKey RUNTIME DESTINATION bin)

add_subdirectory(src/qt)
