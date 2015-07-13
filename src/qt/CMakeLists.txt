
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)

include_directories(SYSTEM ${Qt5Widgets_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} )

set(XKey_Qt_SRCS KeyListModel.cpp FileDialog.cpp FolderListModel.cpp KeyEditDialog.cpp XKeyApplication.cpp SettingsDialog.cpp qt_main.cpp)

QT_WRAP_CPP(XKey_MOC XKeyApplication.h KeyListModel.h FolderListModel.h KeyEditDialog.h FileDialog.h SettingsDialog.h )
set(UiPath "${CMAKE_PROJECT_ROOT}/ui")
QT5_WRAP_UI(XKey_UI ${UiPath}/Main.ui ${UiPath}/Entry.ui ${UiPath}/Settings.ui
	${UiPath}/SaveKeystorePassphrase.ui ${UiPath}/OpenKeystorePassphrase.ui ${UiPath}/About.ui )

QT5_ADD_RESOURCES(XKey_RCC_SRCS res.qrc)

set(Libraries Qt5::Widgets )

add_executable(XKey_qt ${XKey_Qt_SRCS} ${XKey_MOC} ${XKey_UI} ${XKey_RCC_SRCS} )
target_link_libraries(XKey_qt ${Libraries} ${XKeyLibraries} )

install(TARGETS XKey_qt RUNTIME DESTINATION bin)
