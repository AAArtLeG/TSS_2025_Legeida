#Tu nastavit verziu Qt6
set(Qt6_Version 6.9.3)

cmake_minimum_required(VERSION 3.16.0)

PROJECT(TSS_App VERSION 1.0.0 LANGUAGES CXX)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
	add_definitions( "/D_CRT_SECURE_NO_WARNINGS /MP /openmp" )
endif(MSVC)

find_package(Exiv2 CONFIG QUIET)
if (Exiv2_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE Exiv2::exiv2)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_EXIV2=1)
else()
    find_package(Exiv2 QUIET)
    if (Exiv2_FOUND)
        target_link_libraries(${PROJECT_NAME} PRIVATE exiv2lib)
        target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_EXIV2=1)
    endif()
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(Qt6_DIR "C:/Qt/${Qt6_Version}/msvc2022_64/lib/cmake/Qt6")
set(Qt6WidgetsTools_DIR "C:/Qt/${Qt6_Version}/msvc2022_64/lib/cmake/Qt6WidgetsTools")
set(Qt6CoreTools_DIR "C:/Qt/${Qt6_Version}/msvc2022_64/lib/cmake/Qt6CoreTools")
set(Qt6GuiTools_DIR "C:/Qt/${Qt6_Version}/msvc2022_64/lib/cmake/Qt6GuiTools")

find_package(Qt6 COMPONENTS Widgets Core Gui Test REQUIRED)

file(GLOB_RECURSE UI_FILES  CONFIGURE_DEPENDS "src/*.ui")
file(GLOB_RECURSE H_FILES   CONFIGURE_DEPENDS "src/*.h")
file(GLOB_RECURSE CPP_FILES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE QRC_FILES CONFIGURE_DEPENDS "src/*.qrc")

set(SOURCE_LIST ${CPP_FILES} ${UI_FILES} ${H_FILES} ${QRC_FILES})

add_executable(${PROJECT_NAME} ${SOURCE_LIST})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Core Qt6::Gui)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_LIST})

add_custom_command(TARGET ${PROJECT_NAME}
 				POST_BUILD COMMAND "C:/Qt/${Qt6_Version}/msvc2022_64/bin/windeployqt.exe" "$(OutDir)$(TargetName)$(TargetExt)" 
 				COMMENT "Win deploy qt")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# Unit Test: ImageEditor - rotation

set(Qt6_BIN "C:/Qt/${Qt6_Version}/msvc2022_64/bin")

add_executable(tst_ImageEditorRotateUnit
    tests/tst_ImageEditorRotateUnit.cpp
    src/ImageEditor.cpp
)

target_include_directories(tst_ImageEditorRotateUnit
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(tst_ImageEditorRotateUnit 
    PRIVATE Qt6::Test Qt6::Core Qt6::Gui Qt6::Widgets
)

add_test(NAME unit_REQ-UNIT-ROTATE COMMAND tst_ImageEditorRotateUnit)

add_custom_command(TARGET tst_ImageEditorRotateUnit
    POST_BUILD
    COMMAND "${Qt6_BIN}/windeployqt.exe" "$<TARGET_FILE:tst_ImageEditorRotateUnit>"
    COMMENT "Win deploy qt (unit test)"
)

# Integration Test: Persistence of meta data

set(Qt6_BIN "C:/Qt/${Qt6_Version}/msvc2022_64/bin")

add_executable(tst_PersistenceIntegration
    tests/tst_PersistenceIntegration.cpp
    src/MetaDataStorage.cpp
)

target_include_directories(tst_PersistenceIntegration
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(tst_PersistenceIntegration 
    PRIVATE Qt6::Test Qt6::Core Qt6::Gui Qt6::Widgets
)

add_test(NAME integration_REQ-NFR2-PERSISTENCE COMMAND tst_PersistenceIntegration)

add_custom_command(TARGET tst_PersistenceIntegration
    POST_BUILD
    COMMAND "${Qt6_BIN}/windeployqt.exe" "$<TARGET_FILE:tst_PersistenceIntegration>"
    COMMENT "Win deploy qt (integration test)"
)