cmake_minimum_required(VERSION 2.8.12)

project(FileClient)

# 在文件开头添加平台检测
if(WIN32)
    # Windows 特定设置保持不变
    set(CMAKE_PREFIX_PATH "D:/myinstall/Qt5.12.12/5.12.12/msvc2017_64")
    set(PROTOBUF_ROOT "D:/myinstall/protobuf/protobuf64")
    set(OPENSSL_ROOT "D:/myinstall/OpenSSL/OpenSSL-Win64")
    
    # Windows特定的Protobuf设置
    set(PROTOBUF_INCLUDE_DIR "${PROTOBUF_ROOT}/include")
    set(PROTOBUF_LIBRARY "${PROTOBUF_ROOT}/lib/libprotobufd.lib")
    set(PROTOBUF_LITE_LIBRARY "${PROTOBUF_ROOT}/lib/libprotobuf-lited.lib")
    set(PROTOBUF_DLL "${PROTOBUF_ROOT}/lib/libprotobufd.dll")
    
    # Windows特定的OpenSSL设置
    set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT}/include")
    set(OPENSSL_LIBRARIES "${OPENSSL_ROOT}/lib/libcrypto.lib")
    set(OPENSSL_DLL "${OPENSSL_ROOT}/bin/libcrypto-1_1-x64.dll")
else()
    # Linux 特定设置
    set(CMAKE_PREFIX_PATH "/usr/lib64/qt5")
    
    # 确保找到线程库
    find_package(Threads REQUIRED)
    
    # 使用find_package查找Protobuf
    find_package(Protobuf REQUIRED)
    if(NOT PROTOBUF_FOUND)
        message(FATAL_ERROR "Protobuf not found")
    endif()
    
    message(STATUS "Protobuf include dir: ${PROTOBUF_INCLUDE_DIRS}")
    message(STATUS "Protobuf libraries: ${PROTOBUF_LIBRARIES}")
    
    # 直接设置OpenSSL的库和包含目录，不使用find_package
    set(OPENSSL_INCLUDE_DIR "/usr/local/include")
    set(OPENSSL_SSL_LIB "/usr/local/lib64/libssl.so.1.1")
    set(OPENSSL_CRYPTO_LIB "/usr/local/lib64/libcrypto.so.1.1")
    set(OPENSSL_LIBRARIES ${OPENSSL_SSL_LIB} ${OPENSSL_CRYPTO_LIB})
    
    # 验证OpenSSL文件是否存在
    if(NOT EXISTS "${OPENSSL_SSL_LIB}")
        message(FATAL_ERROR "OpenSSL SSL library not found at ${OPENSSL_SSL_LIB}")
    endif()
    if(NOT EXISTS "${OPENSSL_CRYPTO_LIB}")
        message(FATAL_ERROR "OpenSSL Crypto library not found at ${OPENSSL_CRYPTO_LIB}")
    endif()
    if(NOT EXISTS "${OPENSSL_INCLUDE_DIR}/openssl/ssl.h")
        message(FATAL_ERROR "OpenSSL headers not found at ${OPENSSL_INCLUDE_DIR}")
    endif()
    
    message(STATUS "OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "OpenSSL SSL library: ${OPENSSL_SSL_LIB}")
    message(STATUS "OpenSSL Crypto library: ${OPENSSL_CRYPTO_LIB}")
    
    # Linux特定的编译选项
    add_compile_options(-fPIC)
    
    # Linux特定的定义
    add_definitions(
        -DLINUX
        -D_REENTRANT
        -D_THREAD_SAFE
    )
endif()

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

# 查找 Qt 包
find_package(Qt5 COMPONENTS Core Gui Widgets Network REQUIRED)

# 在 find_package(Qt5 ...) 之后添加
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/shared
    ${CMAKE_CURRENT_SOURCE_DIR}/protos
)

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

# 添加资源文件编译
qt5_add_resources(RESOURCES_RCC ${CLIENT_RESOURCES})

# 创建可执行文件
add_executable(${PROJECT_NAME}
    ${CLIENT_SOURCES}
    ${RESOURCES_RCC}
)

# 设置 OpenSSL 配置 - MSVC 版本
set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT}/include")
set(OPENSSL_LIBRARIES "${OPENSSL_ROOT}/lib/libcrypto.lib")
set(OPENSSL_DLL "${OPENSSL_ROOT}/bin/libcrypto-1_1-x64.dll")

# 设置包含目录
if(WIN32)
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${SRC_DIR}
        ${SHARED_DIR}
        ${PROTOS_DIR}
        ${PROTOBUF_INCLUDE_DIR}
        ${OPENSSL_INCLUDE_DIR}
    )
else()
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${SRC_DIR}
        ${SHARED_DIR}
        ${PROTOS_DIR}
        ${PROTOBUF_INCLUDE_DIRS}
        ${OPENSSL_INCLUDE_DIR}
    )
endif()

# 链接库
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${PROTOBUF_LIBRARY}
        ${PROTOBUF_LITE_LIBRARY}
        ${OPENSSL_LIBRARIES}
    )
else()
    # Linux下OpenSSL库的名称不同
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${PROTOBUF_LIBRARIES}
        ${OPENSSL_SSL_LIB}        # 使用前面定义的具体库路径
        ${OPENSSL_CRYPTO_LIB}     # 使用前面定义的具体库路径
        ${CMAKE_THREAD_LIBS_INIT}
    )
endif()

# 添加 Qt 的 DLL 目录到运行时路径
set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 只在 Windows 下设置 MSVC 特定选项
if(MSVC)
    set(CMAKE_C_COMPILER "D:/myinstall/VS2022/Visual_Studio_IDE/Community/VC/Tools/MSVC/14.16.27023/bin/HostX64/x64/cl.exe")
    set(CMAKE_CXX_COMPILER "D:/myinstall/VS2022/Visual_Studio_IDE/Community/VC/Tools/MSVC/14.16.27023/bin/HostX64/x64/cl.exe")
    
    set(CMAKE_VS_PLATFORM_TOOLSET "v141")
    set(CMAKE_VS_PLATFORM_TOOLSET_VERSION "14.16.27023")
    
    # MSVC 特定的编译选项
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /W3 /GR /EHsc /bigobj /utf-8")
    
    # MSVC 特定的警告禁用
    add_compile_options(
        /wd4819
        /wd4251
        /wd4996
        /wd4275
        /wd4267
        /wd4244
    )
    
    # MSVC 特定的定义
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
    
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    add_definitions(-D_HAS_AUTO_PTR_ETC=1)
endif()

# 在设置通用编译选项部分修改
if(WIN32)
    # Windows 设置保持不变
    if(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DEBUG")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG /SUBSYSTEM:CONSOLE")
    endif()
else()
    # Linux 特定的编译选项
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")  # 添加 C++11 支持
    add_compile_options(-fPIC)
endif()

if(NOT WIN32)
    # 添加运行时资源路径
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    
    # 确保Qt插件路径正确
    set(QT_PLUGIN_PATH "${CMAKE_PREFIX_PATH}/plugins")
    add_definitions(-DQT_PLUGIN_PATH="${QT_PLUGIN_PATH}")

    # 只在非Windows环境下生成compile_commands.json
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    
    # Linux 特定设置
    add_definitions(
        -DGOOGLE_PROTOBUF_CMAKE_BUILD
        -DPROTOBUF_USE_DLLS
    )
    
    # 添加调试信息
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()

# 定义复制函数
function(copy_file src dst)
    if(EXISTS "${src}")
        if(WIN32)
            file(COPY "${src}" DESTINATION "${dst}")
        else()
            # Linux下使用cp命令处理符号链接
            execute_process(
                COMMAND cp -P "${src}" "${dst}"
                RESULT_VARIABLE copy_result
            )
            if(copy_result)
                message(WARNING "Failed to copy ${src} to ${dst}")
            endif()
        endif()
    else()
        message(WARNING "File not found: ${src}")
    endif()
endfunction()

# 复制依赖库到目标目录
if(WIN32)
    # 设置目标目录
    set(TARGET_DIR ${CMAKE_BINARY_DIR}/bin/Debug)
    # Windows下复制Qt DLL
    get_target_property(QT_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
    
    set(QT_DLLS
        "${QT_BIN_DIR}/Qt5Core.dll"
        "${QT_BIN_DIR}/Qt5Gui.dll"
        "${QT_BIN_DIR}/Qt5Widgets.dll"
        "${QT_BIN_DIR}/Qt5Network.dll"
    )
    
    # 复制Qt DLL
    foreach(QT_DLL ${QT_DLLS})
        copy_file("${QT_DLL}" "${TARGET_DIR}/")
    endforeach()
    
    # 复制Protobuf DLL
    copy_file("${PROTOBUF_DLL}" "${TARGET_DIR}/")
    
    # 复制OpenSSL DLL
    copy_file("${OPENSSL_DLL}" "${TARGET_DIR}/")
    
    # 复制Qt平台插件到同级目录的platforms子目录
    file(MAKE_DIRECTORY "${TARGET_DIR}/platforms")
    copy_file("${QT_BIN_DIR}/../plugins/platforms/qwindows.dll" "${TARGET_DIR}/platforms/")
    
else()
    # Linux下复制共享库
    # 设置目标目录
    set(TARGET_DIR ${CMAKE_BINARY_DIR}/bin)
    
    # 复制Qt库
    execute_process(
        COMMAND qmake -query QT_INSTALL_LIBS
        OUTPUT_VARIABLE QT_LIB_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    file(GLOB QT_LIBS
        "${QT_LIB_DIR}/libQt5Core.so*"
        "${QT_LIB_DIR}/libQt5Gui.so*"
        "${QT_LIB_DIR}/libQt5Widgets.so*"
        "${QT_LIB_DIR}/libQt5Network.so*"
    )
    
    # 复制Qt库到同级目录
    foreach(LIB ${QT_LIBS})
        copy_file("${LIB}" "${TARGET_DIR}")
    endforeach()
    
    # 复制Qt平台插件到同级目录的platforms子目录
    execute_process(
        COMMAND qmake -query QT_INSTALL_PLUGINS
        OUTPUT_VARIABLE QT_PLUGIN_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    file(MAKE_DIRECTORY "${TARGET_DIR}/platforms")
    file(GLOB QT_PLATFORM_PLUGINS "${QT_PLUGIN_DIR}/platforms/libqxcb.so*")
    foreach(PLUGIN ${QT_PLATFORM_PLUGINS})
        copy_file("${PLUGIN}" "${TARGET_DIR}/platforms")
    endforeach()
    
    # 复制Protobuf库到同级目录
    file(GLOB PROTOBUF_LIBS "/usr/local/lib/libprotobuf*.so*")
    foreach(LIB ${PROTOBUF_LIBS})
        copy_file("${LIB}" "${TARGET_DIR}")
    endforeach()
    
    # 复制OpenSSL库
    copy_file("${OPENSSL_SSL_LIB}" "${TARGET_DIR}")
    copy_file("${OPENSSL_CRYPTO_LIB}" "${TARGET_DIR}")
endif()

# 添加自定义命令,确保在构建完成后执行复制
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Copying dependencies to ${TARGET_DIR}"
)