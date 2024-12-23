#pragma once
#ifndef NET_TOOL_H
#define NET_TOOL_H


#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <queue>
#include <functional>
#include <openssl/md5.h>
#include "../protos/transfer.pb.h"

#ifdef _WIN32
    #include <WinSock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET socket_t;
    #define INVALID_SOCK INVALID_SOCKET
    #define SOCK_ERROR SOCKET_ERROR
    #define CLOSE_SOCKET closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <errno.h>
    typedef int socket_t;
    #define INVALID_SOCK (-1)
    #define SOCK_ERROR (-1)
    #define CLOSE_SOCKET close
#endif

//用于区分底层收发请求类型
#define DIRECTORY_TYPE 1
#define UPLOAD_TYPE 2
#define DOWNLOAD_TYPE 3
#define TRANSFER_CONTROL_TYPE 4
#define TRANSFER_PROGRESS_TYPE 5


#define CHUNK_SIZE 1024 * 1024

class Net_Tool {
public:
    static Net_Tool* getInstance() {
        static Net_Tool instance;
        return &instance;
    }

    // 连接到服务器
    bool connectToServer(const std::string& serverIP, uint16_t port);
    
    // 断开连接
    void disconnect();

    // 生成目录请求
    transfer::DirectoryRequest createDirectoryRequest(const std::string& currentPath, 
        const std::string& dirName, bool isParent = false);

    // 生成文件上传请求
    transfer::UploadRequest createUploadRequest(const std::string& fileName, 
        const std::string& targetPath, uint32_t chunkSize = CHUNK_SIZE);

    // 生成文件下载请求
    transfer::DownloadRequest createDownloadRequest(const std::string& fileName,
        const std::string& targetPath, uint32_t chunkSize = CHUNK_SIZE);

    // 生成传输控制请求
    transfer::TransferControlRequest createTransferControlRequest(const std::string& taskId,
        transfer::TransferControlRequest_ControlType controlType);

    // 生成传输进度查询请求
    transfer::TransferProgressRequest createTransferProgressRequest(const std::string& taskId);

    // 开始文件上传任务
    void startUploadTask(const std::string& fileName, const std::string& targetPath,
        std::function<void(const transfer::TransferProgressResponse&)> progressCallback);

    // 开始文件下载任务
    void startDownloadTask(const std::string& fileName, const std::string& targetPath,
        std::function<void(const transfer::TransferProgressResponse&)> progressCallback);

    // 暂停传输任务
    void pauseTransfer(const std::string& taskId);

    // 恢复传输任务
    void resumeTransfer(const std::string& taskId);

    // 取消传输任务
    void cancelTransfer(const std::string& taskId);

    // 发送目录请求并获取响应
    transfer::DirectoryResponse sendDirectoryRequest(const transfer::DirectoryRequest& request);

    // 设置错误回调
    void setErrorCallback(std::function<void(const std::string&)> callback) {
        m_errorCallback = callback;
    }

private:
    Net_Tool();
    ~Net_Tool();
    Net_Tool(const Net_Tool&) = delete;
    Net_Tool& operator=(const Net_Tool&) = delete;

public:
    // 生成文件的MD5值
    std::string calculateFileMD5(const std::string& filePath);

    // 生成数据的CRC32校验和
    uint32_t calculateCRC32(const void* data, size_t length);

    // 生成请求头
    transfer::RequestHeader createRequestHeader(transfer::MessageType type);

    // 发送数据
    bool sendData(const void* data, size_t length);

    // 接收数据
    bool receiveData(void* buffer, size_t length);

    // 发送protobuf消息
    template<typename T>
    bool sendMessage(const T& message, char type);

    // 接收protobuf消息
    template<typename T>
    bool receiveMessage(T& message, char type);

    //查看数据，返回本次查看的数据长度
    int peek_read(char *buf, int len);

    // 传输任务结构
    struct TransferTask {
        std::string taskId;
        std::string fileName;
        std::string targetPath;
        uint64_t fileSize;
        uint64_t transferredSize;
        bool isPaused;
        bool isCancelled;
        std::thread transferThread;
        std::function<void(const transfer::TransferProgressResponse&)> progressCallback;
    };

    socket_t m_sock;
    bool m_isConnected;
    std::mutex m_sockMutex;
    std::mutex m_tasksMutex;
    std::map<std::string, TransferTask*> m_transferTasks;
    std::function<void(const std::string&)> m_errorCallback;

    // 生成唯一的任务ID
    std::string generateTaskId();

    // 添加上传任务处理函数
    void handleUploadTask(TransferTask* task);
    
    // 添加下载任务处理函数 
    void handleDownloadTask(TransferTask* task);
};

#endif  // NET_TOOL_H