cmake_minimum_required(VERSION 3.5)

project(FileClient LANGUAGES CXX)

# 设置通用编译选项
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 添加调试信息
set(CMAKE_BUILD_TYPE Debug)
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DEBUG")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG /SUBSYSTEM:CONSOLE")
endif()

#禁止自动扫描
set(CMAKE_FIND_PACKAGE_NO_PACKAGE_REGISTRY ON)

# 设置 Qt 路径 - MSVC 版本
set(CMAKE_PREFIX_PATH "D:/myinstall/Qt5.12.12/5.12.12/msvc2017_64")

# 查找 Qt 包
find_package(Qt5 COMPONENTS Core Gui Widgets Network REQUIRED)

# 设置 Protobuf 相关路径 - MSVC 版本
set(PROTOBUF_ROOT "D:/myinstall/protobuf/protobuf64")
set(PROTOBUF_INCLUDE_DIR "${PROTOBUF_ROOT}/include")
set(PROTOBUF_LIBRARY "${PROTOBUF_ROOT}/lib/libprotobufd.lib")
set(PROTOBUF_LITE_LIBRARY "${PROTOBUF_ROOT}/lib/libprotobuf-lited.lib")
set(PROTOBUF_DLL "${PROTOBUF_ROOT}/lib/libprotobufd.dll")

# 设置源文件目录
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(SHARED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shared")
set(PROTOS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/protos")
set(RESOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")

# 添加源文件
set(SOURCES
    src/main.cpp
    src/FileClient.cpp
    src/FileListView.cpp
    src/AddressBar.cpp
    src/LogWidget.cpp
    src/ProgressWidget.cpp
    src/Net_Tool.cpp
    src/RemoteFileSystemModel.cpp
    src/FileTabPage.cpp
    src/LocalFileSystemModel.cpp
)

# 添加头文件
set(HEADERS
    src/FileClient.h
    src/FileListView.h
    src/AddressBar.h
    src/LogWidget.h
    src/ProgressWidget.h
    src/Net_Tool.h
    src/RemoteFileSystemModel.h
    src/FileTabPage.h
    src/LocalFileSystemModel.h
    src/CommHead.h
)

# 收集源文件
file(GLOB_RECURSE CLIENT_SOURCES 
    "${SRC_DIR}/*.cpp"
    "${SRC_DIR}/*.h"
    "${SHARED_DIR}/*.cpp"
    "${SHARED_DIR}/*.h"
    "${PROTOS_DIR}/*.cc"
    "${PROTOS_DIR}/*.h"
)

# 添加资源文件
set(CLIENT_RESOURCES
    "${RESOURCES_DIR}/icons.qrc"
    "${RESOURCES_DIR}/help.qrc"
    "${RESOURCES_DIR}/themes.qrc"
)

# 创建可执行文件
add_executable(${PROJECT_NAME}
    ${CLIENT_SOURCES}
    ${CLIENT_RESOURCES}
)

# 设置 OpenSSL 配置 - MSVC 版本
set(OPENSSL_ROOT "D:/myinstall/OpenSSL/OpenSSL-Win64")
set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT}/include")
set(OPENSSL_LIBRARIES "${OPENSSL_ROOT}/lib/libcrypto.lib")
set(OPENSSL_DLL "${OPENSSL_ROOT}/bin/libcrypto-1_1-x64.dll")

# 设置包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${SRC_DIR}
    ${SHARED_DIR}
    ${PROTOBUF_INCLUDE_DIR}
    ${PROTOS_DIR}
    ${OPENSSL_INCLUDE_DIR}
)

# 链接库
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
    ${PROTOBUF_LIBRARY}
    ${PROTOBUF_LITE_LIBRARY}
    ${OPENSSL_LIBRARIES}
)

# 添加 Qt 的 DLL 目录到运行时路径
set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 复制 Qt DLLs
get_target_property(QT_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
set(QT_DLLS
    Qt5Core.dll
    Qt5Gui.dll
    Qt5Widgets.dll
    Qt5Network.dll
)

foreach(DLL ${QT_DLLS})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${QT_BIN_DIR}/${DLL}"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endforeach()

# 验证文件
if(NOT EXISTS ${PROTOBUF_LIBRARY})
    message(FATAL_ERROR "Protobuf library not found at ${PROTOBUF_LIBRARY}")
endif()

if(NOT EXISTS ${PROTOBUF_DLL})
    message(FATAL_ERROR "Protobuf DLL not found at ${PROTOBUF_DLL}")
endif()

# 复制 protobuf DLL 到输出目录
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROTOBUF_DLL}"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

# 复制 OpenSSL DLL
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${OPENSSL_DLL}"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

# 在项目声明之前添加
set(CMAKE_C_COMPILER "D:/myinstall/VS2022/Visual_Studio_IDE/Community/VC/Tools/MSVC/14.16.27023/bin/HostX64/x64/cl.exe")
set(CMAKE_CXX_COMPILER "D:/myinstall/VS2022/Visual_Studio_IDE/Community/VC/Tools/MSVC/14.16.27023/bin/HostX64/x64/cl.exe")

# 在项目声明之后添加
if(MSVC)
    set(CMAKE_VS_PLATFORM_TOOLSET "v141")
    set(CMAKE_VS_PLATFORM_TOOLSET_VERSION "14.16.27023")
    
    # 基本编译器选项
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
        /MP \
        /W3 \
        /GR \
        /EHsc \
        /bigobj \
        /utf-8")
    
    # 禁用特定警告
    add_compile_options(
        /wd4819  # 禁用代码页警告
        /wd4251  # 禁用 DLL 接口警告
        /wd4996  # 禁用不安全函数警告
        /wd4275  # 禁用 DLL 导出警告
        /wd4267  # 禁用大小类型转换警告
        /wd4244  # 禁用数值转换警告
    )
    
    # 基本定义
    add_definitions(
        -D_UNICODE 
        -DUNICODE
        -D_CRT_SECURE_NO_WARNINGS
        -D_SCL_SECURE_NO_WARNINGS
        -DWIN32_LEAN_AND_MEAN
        -DNOMINMAX
        -DPROTOBUF_USE_DLLS
        -DGOOGLE_PROTOBUF_CMAKE_BUILD
    )
    
    # 设置运行时库
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    
    # 强制使用旧版本的 STL
    add_definitions(-D_HAS_AUTO_PTR_ETC=1)
endif()

# 添加 protobuf 包含目录
include_directories(${PROTOBUF_INCLUDE_DIR})

# 添加 protobuf 库
target_link_libraries(${PROJECT_NAME} 
    PRIVATE
    ${PROTOBUF_LIBRARY}
    # ... 其他库
)