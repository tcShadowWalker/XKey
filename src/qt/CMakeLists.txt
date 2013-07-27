
find_package(Qt4 REQUIRED)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(XKey_Qt_SRCS KeyListModel.cpp FolderListModel.cpp KeyEditDialog.cpp XKeyApplication.cpp qt_main.cpp)

qt4_wrap_cpp(XKey_MOC XKeyApplication.h KeyListModel.h FolderListModel.h KeyEditDialog.h )
qt4_wrap_ui(XKey_UI ${CMAKE_PROJECT_ROOT}/ui/Main.ui ${CMAKE_PROJECT_ROOT}/ui/OpenKeystorePassphrase.ui ${CMAKE_PROJECT_ROOT}/ui/Entry.ui )

add_executable(XKey_qt ${XKey_SRCS} ${XKey_Qt_SRCS} ${XKey_MOC} ${XKey_UI}  ${TP_Json_SRCS} )
target_link_libraries(XKey_qt ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${XKeyLibraries} )
install(TARGETS XKey_qt RUNTIME DESTINATION bin)
