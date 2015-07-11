
set(TestDir ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable(XTest ${TestDir}/test.cpp ${SrcDir}/UtilFunctions.cpp )
target_link_libraries(XTest ${XKeyLibraries} ${Boost_PROGRAM_OPTIONS_LIBRARY}  )

add_executable(WriteTest ${TestDir}/write_test.cpp ${SrcDir}/UtilFunctions.cpp )
target_link_libraries(WriteTest ${XKeyLibraries} ${Boost_PROGRAM_OPTIONS_LIBRARY}  )

add_executable(XMigrate ${TestDir}/XMigrate.cpp ${SrcDir}/UtilFunctions.cpp )
target_link_libraries(XMigrate ${XKeyLibraries} ${Boost_PROGRAM_OPTIONS_LIBRARY}  )

