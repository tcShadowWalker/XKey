
find_package(Qt4 REQUIRED)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(XKey_Qt_SRCS KeyListModel.cpp FileDialog.cpp FolderListModel.cpp KeyEditDialog.cpp XKeyApplication.cpp SettingsDialog.cpp qt_main.cpp)

qt4_wrap_cpp(XKey_MOC XKeyApplication.h KeyListModel.h FolderListModel.h KeyEditDialog.h FileDialog.h SettingsDialog.h )
set(UiPath "${CMAKE_PROJECT_ROOT}/ui")
qt4_wrap_ui(XKey_UI ${UiPath}/Main.ui ${UiPath}/Entry.ui ${UiPath}/Settings.ui
	${UiPath}/SaveKeystorePassphrase.ui ${UiPath}/OpenKeystorePassphrase.ui ${UiPath}/About.ui )

QT4_ADD_RESOURCES(XKey_RCC_SRCS res.qrc )

add_executable(XKey_qt ${XKey_SRCS} ${XKey_Qt_SRCS} ${XKey_MOC} ${XKey_UI} ${XKey_RCC_SRCS} ${TP_Json_SRCS} )
target_link_libraries(XKey_qt ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${XKeyLibraries} )
install(TARGETS XKey_qt RUNTIME DESTINATION bin)
