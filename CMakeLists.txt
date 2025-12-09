cmake_minimum_required(VERSION 3.20)
project(LifeGame LANGUAGES CXX)

# C++ 标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows Unicode 支持
add_compile_definitions(UNICODE _UNICODE)

# MSVC 特定设置
if(MSVC)
    # UTF-8 源文件编码
    add_compile_options(/utf-8)
    # 警告级别 3
    add_compile_options(/W3)
    # SDL 检查
    add_compile_options(/sdl)
    # 一致性模式
    add_compile_options(/permissive-)
endif()

# 源文件
set(SOURCES
    LifeGame/Application.cpp
    LifeGame/CommandHistory.cpp
    LifeGame/FileManager.cpp
    LifeGame/Game.cpp
    LifeGame/HelpWindow.cpp
    LifeGame/Main.cpp
    LifeGame/PatternLibrary.cpp
    LifeGame/PatternPreview.cpp
    LifeGame/PlacePatternCommand.cpp
    LifeGame/Renderer.cpp
    LifeGame/RuleEngine.cpp
    LifeGame/SetCellCommand.cpp
    LifeGame/SettingsDialog.cpp
    LifeGame/SplashWindow.cpp
    LifeGame/Statistics.cpp
    LifeGame/UI.cpp
)

# 头文件
set(HEADERS
    LifeGame/Application.h
    LifeGame/Command.h
    LifeGame/CommandHistory.h
    LifeGame/FileManager.h
    LifeGame/Game.h
    LifeGame/HelpWindow.h
    LifeGame/PatternLibrary.h
    LifeGame/PatternPreview.h
    LifeGame/PlacePatternCommand.h
    LifeGame/Renderer.h
    LifeGame/Resource.h
    LifeGame/RuleEngine.h
    LifeGame/SetCellCommand.h
    LifeGame/Settings.h
    LifeGame/SettingsDialog.h
    LifeGame/SplashWindow.h
    LifeGame/Statistics.h
    LifeGame/UI.h
)

# 创建可执行文件 (WIN32 表示 Windows GUI 应用程序)
add_executable(LifeGame WIN32 ${SOURCES} ${HEADERS})

# 包含目录
target_include_directories(LifeGame PRIVATE ${CMAKE_SOURCE_DIR}/LifeGame)

# 配置特定的预处理器定义
target_compile_definitions(LifeGame PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    _CONSOLE
)

# Release 配置优化
if(MSVC)
    target_compile_options(LifeGame PRIVATE
        $<$<CONFIG:Release>:/Gy>  # 函数级链接
        $<$<CONFIG:Release>:/Oi>  # 内在函数
    )
endif()

# 输出目录配置 (与 VS 项目保持一致)
set_target_properties(LifeGame PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/x64/$<CONFIG>"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/x64/$<CONFIG>"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/x64/$<CONFIG>"
)

# 在 IDE 中组织源文件
source_group("Source Files" FILES ${SOURCES})
source_group("Header Files" FILES ${HEADERS})

