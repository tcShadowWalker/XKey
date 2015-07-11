project(XKey)
cmake_minimum_required(VERSION 2.6)

set(CMAKE_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall")

find_package(OpenSSL)

set(Boost_USE_STATIC_LIBS True)
find_package(Boost REQUIRED COMPONENTS program_options)

include_directories(SYSTEM ${OPENSSL_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/ ${Boost_INCLUDE_DIR} )

set(XKeyLibraries ${OPENSSL_CRYPTO_LIBRARIES} XKeyLib )

set(JsonDir ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/json/lib_json)
set(TP_Json_SRCS ${JsonDir}/json_reader.cpp ${JsonDir}/json_value.cpp ${JsonDir}/json_writer.cpp) 

set(SrcDir ${CMAKE_CURRENT_SOURCE_DIR}/src )
set(CoreDir ${CMAKE_CURRENT_SOURCE_DIR}/src/core )
set(XKey_SRCS ${CoreDir}/XKey.cpp ${CoreDir}/CryptStream.cpp
	${CoreDir}/XKeyGenerator.cpp ${CoreDir}/XKeyJsonSerialization.cpp )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/include )

add_library(XKeyLib ${XKey_SRCS} ${TP_Json_SRCS} )

add_executable(XKey ${SrcDir}/main.cpp ${SrcDir}/UtilFunctions.cpp )
target_link_libraries(XKey ${XKeyLibraries} ${Boost_PROGRAM_OPTIONS_LIBRARY}  )
install(TARGETS XKey RUNTIME DESTINATION bin)

add_subdirectory(src/qt)

if(TEST)
	add_subdirectory(test)
endif()
