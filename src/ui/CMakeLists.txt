set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt5
    REQUIRED
    COMPONENTS
        Core
        Gui
        Multimedia
        Widgets
)

set(UI_SRC
    LogWindow.cpp
    MainWindow.cpp
    SettingsConstants.cpp
)

add_library(zlsnes_ui
    ${UI_SRC}
)
target_link_libraries(zlsnes_ui
    Qt5::Core
    Qt5::Gui
    Qt5::Multimedia
    Qt5::Widgets
    zlsnes_core
)
target_compile_definitions(zlsnes_ui
    PRIVATE QT_DEPRECATED_WARNINGS
)