
set(TestDir ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable(XTest EXCLUDE_FROM_ALL ${TestDir}/test.cpp ${SrcDir}/UtilFunctions.cpp )
target_link_libraries(XTest ${XKeyLibraries} ${Boost_PROGRAM_OPTIONS_LIBRARY}  )

add_executable(WriteTest EXCLUDE_FROM_ALL ${TestDir}/write_test.cpp ${SrcDir}/UtilFunctions.cpp )
target_link_libraries(WriteTest ${XKeyLibraries} ${Boost_PROGRAM_OPTIONS_LIBRARY}  )
