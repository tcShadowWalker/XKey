project(XKey)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)

add_definitions( -std=c++0x )

FIND_PACKAGE(OpenSSL)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

include_directories( ${OPENSSL_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/ )


set(JsonDir ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/json/lib_json)
set(TP_Json_SRCS ${JsonDir}/json_reader.cpp ${JsonDir}/json_value.cpp ${JsonDir}/json_writer.cpp) 

set(XKey_Qt_SRCS src/KeyListModel.cpp src/XKey.cpp src/qt_main.cpp)

set(XKey_SRCS src/XKey.cpp src/CryptStream.cpp )

QT4_WRAP_CPP(XKey_MOC  src/XKey.h src/KeyListModel.h)
qt4_wrap_ui(XKey_UI ui/Main.ui)

#add_executable(XKey_qt ${XKey_Qt_SRCS} ${XKey_MOC} ${XKey_UI} )
#target_link_libraries(XKey_qt ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
#install(TARGETS XKey_qt RUNTIME DESTINATION bin)

add_executable(XKey src/main.cpp ${XKey_SRCS} ${TP_Json_SRCS} )
target_link_libraries(XKey ${OPENSSL_CRYPTO_LIBRARIES} boost_program_options )
install(TARGETS XKey RUNTIME DESTINATION bin)

add_executable(XTest src/test.cpp ${XKey_SRCS} ${TP_Json_SRCS} )
target_link_libraries(XTest ${OPENSSL_CRYPTO_LIBRARIES} )
install(TARGETS XTest RUNTIME DESTINATION bin)

add_subdirectory(src/qt)