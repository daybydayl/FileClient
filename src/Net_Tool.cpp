#include "Net_Tool.h"
#include <fstream>
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>
#include <openssl/md5.h>

// CRC32表
static uint32_t crc32_table[256];
static bool crc32_table_computed = false;

static void make_crc32_table() {
    uint32_t c;
    int n, k;
    for (n = 0; n < 256; n++) {
        c = (uint32_t)n;
        for (k = 0; k < 8; k++) {
            if (c & 1)
                c = 0xedb88320L ^ (c >> 1);
            else
                c = c >> 1;
        }
        crc32_table[n] = c;
    }
    crc32_table_computed = true;
}

// 构造函数：初始化网络环境
Net_Tool::Net_Tool() : sock(INVALID_SOCK), isConnected(false) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        if (errorCallback) {
            errorCallback("Failed to initialize Winsock");
        }
    }
#endif
}

// 析构函数：清理网络连接和所有传输任务
Net_Tool::~Net_Tool() {
    disconnect();
#ifdef _WIN32
    WSACleanup();
#endif

    // 清理所有传输任务
    std::lock_guard<std::mutex> lock(tasksMutex);
    for (auto& task : transferTasks) {
        task.second->isCancelled = true;
        if (task.second->transferThread.joinable()) {
            task.second->transferThread.join();
        }
    }
    transferTasks.clear();
}

// 连接到指定服务器
bool Net_Tool::connectToServer(const std::string& serverIP, uint16_t port) {
    std::lock_guard<std::mutex> lock(sockMutex);
    
    if (isConnected) {
        disconnect();
    }

    // 创建socket
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCK) {
        if (errorCallback) {
            errorCallback("Failed to create socket");
        }
        return false;
    }

    // 设置服务器地址
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    if (inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr) != 1) {
        if (errorCallback) {
            errorCallback("Invalid IP address");
        }
        CLOSE_SOCKET(sock);
        return false;
    }

    // 连接服务器
    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCK_ERROR) {
        if (errorCallback) {
#ifdef _WIN32
            errorCallback("Failed to connect to server");
#else
            errorCallback(std::string("Failed to connect to server: ") + strerror(errno));
#endif
        }
        CLOSE_SOCKET(sock);
        return false;
    }

    isConnected = true;
    return true;
}

// 断开服务器连接
void Net_Tool::disconnect() {
    std::lock_guard<std::mutex> lock(sockMutex);
    if (sock != INVALID_SOCK) {
        CLOSE_SOCKET(sock);
        sock = INVALID_SOCK;
    }
    isConnected = false;
}

// 发送数据的底层实现
bool Net_Tool::sendData(const void* data, size_t length) {
    std::lock_guard<std::mutex> lock(sockMutex);
    if (!isConnected) {
        if (errorCallback) {
            errorCallback("Not connected to server");
        }
        return false;
    }

    const char* buffer = static_cast<const char*>(data);
    size_t totalSent = 0;
    while (totalSent < length) {
#ifdef _WIN32
        int sent = send(sock, buffer + totalSent, static_cast<int>(length - totalSent), 0);
        if (sent == SOCK_ERROR) {
#else
        ssize_t sent = send(sock, buffer + totalSent, length - totalSent, 0);
        if (sent < 0) {
#endif
            if (errorCallback) {
#ifdef _WIN32
                errorCallback("Failed to send data");
#else
                errorCallback(std::string("Failed to send data: ") + strerror(errno));
#endif
            }
            return false;
        }
        totalSent += sent;
    }
    return true;
}

// 接收数据的底层实现
bool Net_Tool::receiveData(void* buffer, size_t length) {
    std::lock_guard<std::mutex> lock(sockMutex);
    if (!isConnected) {
        if (errorCallback) {
            errorCallback("Not connected to server");
        }
        return false;
    }

    // 分块接收数据，确保完整性
    char* buf = static_cast<char*>(buffer);
    size_t totalReceived = 0;
    while (totalReceived < length) {
        int received = recv(sock, buf + totalReceived, static_cast<int>(length - totalReceived), 0);
        if (received <= 0) {
            if (errorCallback) {
                errorCallback("Failed to receive data");
            }
            return false;
        }
        totalReceived += received;
    }
    return true;
}

template<typename T>
bool Net_Tool::sendMessage(const T& message, char type) {
    // 直接序列化消息并发送
    std::string serialized;
    if (!message.SerializeToString(&serialized)) {
        if (errorCallback) {
            errorCallback("Failed to serialize message");
        }
        return false;
    }

    //再包个底层收发的长度和类型头
    char* buf = new char[sizeof(int)+sizeof(char)+serialized.size()];
    int data_len = serialized.size();
    char type_char = type;
    memcpy((char*)buf,(char*)&data_len,sizeof(int));//序列化数据长度
    memcpy(buf+sizeof(int),&type_char,sizeof(char));//类型
    memcpy(buf+sizeof(int)+sizeof(char),serialized.data(),serialized.size());//序列化数据

    sendData(buf, sizeof(int)+sizeof(char)+serialized.size());
    free(buf);
    return true;
}

template<typename T>
bool Net_Tool::receiveMessage(T& message) {
    // 接收序列化的消息
    std::string serialized;
    // 这里需要根据你的协议来确定如何接收消息
    // ...

    // 反序列化消息
    if (!message.ParseFromString(serialized)) {
        if (errorCallback) {
            errorCallback("Failed to parse message");
        }
        return false;
    }

    return true;
}

transfer::DirectoryResponse Net_Tool::sendDirectoryRequest(const transfer::DirectoryRequest& request) {
    transfer::DirectoryResponse response;
    if (!sendMessage(request, DIRECTORY_TYPE) || !receiveMessage(response)) {
        if (errorCallback) {
            errorCallback("Failed to send directory request");
        }
    }
    return response;
}

transfer::RequestHeader Net_Tool::createRequestHeader(transfer::MessageType type) {
    transfer::RequestHeader header;
    header.set_session_id(generateTaskId());  // 使用任务ID生成器来生成会话ID
    header.set_type(type);
    header.set_timestamp(std::chrono::system_clock::now().time_since_epoch().count());
    static uint64_t sequence = 0;
    header.set_sequence(++sequence);
    return header;
}

// 计算文件的MD5值
std::string Net_Tool::calculateFileMD5(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        return "";
    }

    // 初始化MD5上下文
    MD5_CTX md5Context;
    MD5_Init(&md5Context);

    // 分块读取文件并更新MD5
    char buffer[8192];
    while (file.read(buffer, sizeof(buffer))) {
        MD5_Update(&md5Context, buffer, file.gcount());
    }
    if (file.gcount() > 0) {
        MD5_Update(&md5Context, buffer, file.gcount());
    }

    // 获取最终的MD5值
    unsigned char result[MD5_DIGEST_LENGTH];
    MD5_Final(result, &md5Context);

    // 转换为十六进制字符串
    std::stringstream ss;
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)result[i];
    }
    return ss.str();
}

uint32_t Net_Tool::calculateCRC32(const void* data, size_t length) {
    if (!crc32_table_computed) {
        make_crc32_table();
    }

    const uint8_t* buf = static_cast<const uint8_t*>(data);
    uint32_t crc = 0xffffffffL;

    for (size_t i = 0; i < length; i++) {
        crc = crc32_table[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);
    }
    return crc ^ 0xffffffffL;
}

// 生成唯一的任务ID
std::string Net_Tool::generateTaskId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static const char* hex = "0123456789abcdef";

    // 生成32位的随机十六进制字符串
    std::string uuid;
    uuid.reserve(32);
    for (int i = 0; i < 32; ++i) {
        uuid += hex[dis(gen)];
    }
    return uuid;
}

// 创建目录请求
transfer::DirectoryRequest Net_Tool::createDirectoryRequest(
    const std::string& currentPath, const std::string& dirName, bool isParent) {
    transfer::DirectoryRequest request;
    // 设置请求头和参数
    request.set_allocated_header(new transfer::RequestHeader(createRequestHeader(transfer::DIRECTORY)));
    request.set_current_path(currentPath);
    request.set_dir_name(dirName);
    request.set_is_parent(isParent);
    return request;
}

// 创建上传请求
transfer::UploadRequest Net_Tool::createUploadRequest(
    const std::string& filePath, const std::string& targetPath, uint32_t chunkSize) {
    transfer::UploadRequest request;
    request.set_allocated_header(new transfer::RequestHeader(createRequestHeader(transfer::UPLOAD)));

    // 获取文件信息
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file) {
        return request;
    }

    auto fileSize = file.tellg();
    file.seekg(0);

    // 设置文件信息
    auto fileInfo = request.add_files();
    fileInfo->set_file_name(filePath.substr(filePath.find_last_of("/\\") + 1));
    fileInfo->set_target_path(targetPath);
    fileInfo->set_file_size(fileSize);
    fileInfo->set_md5(calculateFileMD5(filePath));
    fileInfo->set_need_chunk(fileSize > chunkSize);
    fileInfo->set_chunk_size(chunkSize);
    fileInfo->set_task_id(generateTaskId());
    fileInfo->set_status(transfer::INIT);

    return request;
}

transfer::DownloadRequest Net_Tool::createDownloadRequest(
    const std::string& fileName, const std::string& targetPath, uint32_t chunkSize) {
    transfer::DownloadRequest request;
    request.set_allocated_header(new transfer::RequestHeader(createRequestHeader(transfer::DOWNLOAD)));

    auto fileInfo = request.add_files();
    fileInfo->set_file_name(fileName);
    fileInfo->set_target_path(targetPath);
    fileInfo->set_chunk_size(chunkSize);
    fileInfo->set_offset(0);  // 初始下载从0开始
    fileInfo->set_download_id(generateTaskId());

    return request;
}

transfer::TransferControlRequest Net_Tool::createTransferControlRequest(
    const std::string& taskId, transfer::TransferControlRequest_ControlType controlType) {
    transfer::TransferControlRequest request;
    request.set_allocated_header(new transfer::RequestHeader(createRequestHeader(transfer::TRANSFER_CONTROL)));
    request.set_task_id(taskId);
    request.set_control_type(controlType);
    return request;
}

transfer::TransferProgressRequest Net_Tool::createTransferProgressRequest(const std::string& taskId) {
    transfer::TransferProgressRequest request;
    request.set_allocated_header(new transfer::RequestHeader(createRequestHeader(transfer::TRANSFER_PROGRESS)));
    request.set_task_id(taskId);
    return request;
}

void Net_Tool::startUploadTask(const std::string& filePath, const std::string& targetPath,
    std::function<void(const transfer::TransferProgressResponse&)> progressCallback) {
    auto task = std::make_shared<TransferTask>();
    task->taskId = generateTaskId();
    task->filePath = filePath;
    task->targetPath = targetPath;
    task->progressCallback = progressCallback;
    task->isPaused = false;
    task->isCancelled = false;

    {
        std::lock_guard<std::mutex> lock(tasksMutex);
        transferTasks[task->taskId] = task;
    }

    task->transferThread = std::thread([this, task]() {
        std::ifstream file(task->filePath, std::ios::binary | std::ios::ate);
        if (!file) {
            if (errorCallback) {
                errorCallback("Failed to open file: " + task->filePath);
            }
            return;
        }

        task->fileSize = file.tellg();
        file.seekg(0);

        // 创建并发送上传请求
        auto request = createUploadRequest(task->filePath, task->targetPath);
        if (!sendMessage(request, UPLOAD_TYPE)) {
            if (errorCallback) {
                errorCallback("Failed to send upload request");
            }
            return;
        }

        // 接收上传响应
        transfer::UploadResponse response;
        if (!receiveMessage(response)) {
            if (errorCallback) {
                errorCallback("Failed to receive upload response");
            }
            return;
        }

        const uint32_t chunkSize = 1024 * 1024;  // 1MB chunks
        std::vector<char> buffer(chunkSize);
        uint64_t totalRead = 0;

        while (totalRead < task->fileSize && !task->isCancelled) {
            while (task->isPaused && !task->isCancelled) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            if (task->isCancelled) break;

            auto bytesToRead = std::min(chunkSize, static_cast<uint32_t>(task->fileSize - totalRead));
            file.read(buffer.data(), bytesToRead);
            auto bytesRead = file.gcount();

            if (bytesRead <= 0) break;

            // 发送数据块
            if (!sendData(buffer.data(), bytesRead)) {
                if (errorCallback) {
                    errorCallback("Failed to send file data");
                }
                break;
            }

            totalRead += bytesRead;
            task->transferredSize = totalRead;

            // 更新进度
            if (task->progressCallback) {
                transfer::TransferProgressResponse progress;
                progress.set_task_id(task->taskId);
                progress.set_status(transfer::TRANSFERRING);
                progress.set_transferred_size(totalRead);
                progress.set_total_size(task->fileSize);
                progress.set_progress(static_cast<uint32_t>((totalRead * 100) / task->fileSize));
                task->progressCallback(progress);
            }
        }

        // 完成或取消后清理任务
        std::lock_guard<std::mutex> lock(tasksMutex);
        transferTasks.erase(task->taskId);
    });
}

void Net_Tool::startDownloadTask(const std::string& fileName, const std::string& targetPath,
    std::function<void(const transfer::TransferProgressResponse&)> progressCallback) {
    auto task = std::make_shared<TransferTask>();
    task->taskId = generateTaskId();
    task->filePath = fileName;
    task->targetPath = targetPath;
    task->progressCallback = progressCallback;
    task->isPaused = false;
    task->isCancelled = false;

    {
        std::lock_guard<std::mutex> lock(tasksMutex);
        transferTasks[task->taskId] = task;
    }

    task->transferThread = std::thread([this, task]() {
        // 创建并发送下载请求
        auto request = createDownloadRequest(task->filePath, task->targetPath);
        if (!sendMessage(request, DOWNLOAD_TYPE)) {
            if (errorCallback) {
                errorCallback("Failed to send download request");
            }
            return;
        }

        // 接收下载响应
        transfer::DownloadResponse response;
        if (!receiveMessage(response)) {
            if (errorCallback) {
                errorCallback("Failed to receive download response");
            }
            return;
        }

        if (response.results().empty() || !response.results(0).exists()) {
            if (errorCallback) {
                errorCallback("File not found on server");
            }
            return;
        }

        // 创建输出文件
        std::ofstream file(task->targetPath, std::ios::binary);
        if (!file) {
            if (errorCallback) {
                errorCallback("Failed to create output file: " + task->targetPath);
            }
            return;
        }

        const auto& fileResult = response.results(0);
        task->fileSize = fileResult.file_size();
        task->transferredSize = 0;

        std::vector<char> buffer(fileResult.chunk_size());
        while (task->transferredSize < task->fileSize && !task->isCancelled) {
            while (task->isPaused && !task->isCancelled) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            if (task->isCancelled) break;

            // 接收数据块
            if (!receiveData(buffer.data(), buffer.size())) {
                if (errorCallback) {
                    errorCallback("Failed to receive file data");
                }
                break;
            }

            file.write(buffer.data(), buffer.size());
            task->transferredSize += buffer.size();

            // 更新进度
            if (task->progressCallback) {
                transfer::TransferProgressResponse progress;
                progress.set_task_id(task->taskId);
                progress.set_status(transfer::TRANSFERRING);
                progress.set_transferred_size(task->transferredSize);
                progress.set_total_size(task->fileSize);
                progress.set_progress(static_cast<uint32_t>((task->transferredSize * 100) / task->fileSize));
                task->progressCallback(progress);
            }
        }

        file.close();

        // 验证MD5
        std::string downloadedMD5 = calculateFileMD5(task->targetPath);
        if (downloadedMD5 != fileResult.md5()) {
            if (errorCallback) {
                errorCallback("MD5 verification failed");
            }
            // 删除损坏的文件
            std::remove(task->targetPath.c_str());
        }

        // 完成或取消后清理任务
        std::lock_guard<std::mutex> lock(tasksMutex);
        transferTasks.erase(task->taskId);
    });
}

// 暂停传输任务
void Net_Tool::pauseTransfer(const std::string& taskId) {
    std::lock_guard<std::mutex> lock(tasksMutex);
    auto it = transferTasks.find(taskId);
    if (it != transferTasks.end()) {
        it->second->isPaused = true;
    }
}

// 恢复传输任务
void Net_Tool::resumeTransfer(const std::string& taskId) {
    std::lock_guard<std::mutex> lock(tasksMutex);
    auto it = transferTasks.find(taskId);
    if (it != transferTasks.end()) {
        it->second->isPaused = false;
    }
}

// 取消传输任务
void Net_Tool::cancelTransfer(const std::string& taskId) {
    std::lock_guard<std::mutex> lock(tasksMutex);
    auto it = transferTasks.find(taskId);
    if (it != transferTasks.end()) {
        it->second->isCancelled = true;
        if (it->second->transferThread.joinable()) {
            it->second->transferThread.join();
        }
        transferTasks.erase(it);
    }
}
