#include "Net_Tool.h"
#include <fstream>
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>
#include <openssl/md5.h>
#include "FileClient.h"

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
Net_Tool::Net_Tool() : m_sock(INVALID_SOCK), m_isConnected(false) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        if (m_errorCallback) {
            m_errorCallback("Failed to initialize Winsock");
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
    std::lock_guard<std::mutex> lock(m_tasksMutex);
    for (auto& pair : m_transferTasks) {
        TransferTask* task = pair.second;
        task->isCancelled = true;
        if (task->transferThread.joinable()) {
            task->transferThread.join();
        }
        delete task;  // 释放内存
    }
    m_transferTasks.clear();
}

// 连接到指定服务器
bool Net_Tool::connectToServer(const std::string& serverIP, uint16_t port) {
    std::lock_guard<std::mutex> lock(m_sockMutex);
    
    if (m_isConnected) {
        disconnect();
    }

    // 创建socket
    m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_sock == INVALID_SOCK) {
        if (m_errorCallback) {
            m_errorCallback("Failed to create socket");
        }
        return false;
    }

    //linux和window下非阻塞设置不一样
// #ifdef _WIN32
//     u_long iMode = 1;
//     ioctlsocket(m_sock, FIONBIO, &iMode);
// #else
//     int flags = fcntl(m_sock, F_GETFL, 0);
//     fcntl(m_sock, F_SETFL, flags | O_NONBLOCK);
// #endif

    // 设置服务器地址
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    if (inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr) != 1) {
        if (m_errorCallback) {
            m_errorCallback("Invalid IP address");
        }
        CLOSE_SOCKET(m_sock);
        return false;
    }

    // 连接服务器
    if (connect(m_sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCK_ERROR) {
        if (m_errorCallback) {
#ifdef _WIN32
            m_errorCallback("Failed to connect to server");
#else
            m_errorCallback(std::string("Failed to connect to server: ") + strerror(errno));
#endif
        }
        CLOSE_SOCKET(m_sock);
        return false;
    }

    m_isConnected = true;
    return true;
}

// 断开服务器连接
void Net_Tool::disconnect() {
    std::lock_guard<std::mutex> lock(m_sockMutex);
    if (m_sock != INVALID_SOCK) {
        CLOSE_SOCKET(m_sock);
        m_sock = INVALID_SOCK;
    }
    m_isConnected = false;
}

// 发送数据的底层实现
bool Net_Tool::sendData(const void* data, size_t length) {
    std::lock_guard<std::mutex> lock(m_sockMutex);
    if (!m_isConnected) {
        if (m_errorCallback) {
            m_errorCallback("Not connected to server");
        }
        return false;
    }

    const char* buffer = static_cast<const char*>(data);
    size_t totalSent = 0;
    while (totalSent < length) {
#ifdef _WIN32
        int sent = send(m_sock, buffer + totalSent, static_cast<int>(length - totalSent), 0);
        if (sent == SOCK_ERROR) {
#else
        ssize_t sent = send(m_sock, buffer + totalSent, length - totalSent, 0);
        if (sent < 0) {
#endif
            if (m_errorCallback) {
#ifdef _WIN32
                m_errorCallback("Failed to send data");
#else
                m_errorCallback(std::string("Failed to send data: ") + strerror(errno));
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
    std::lock_guard<std::mutex> lock(m_sockMutex);
    if (!m_isConnected) {
        if (m_errorCallback) {
            m_errorCallback("Not connected to server");
        }
        return false;
    }

    // 分块接收数据，确保完整性
    char* buf = static_cast<char*>(buffer);
    size_t totalReceived = 0;
    while (totalReceived < length) {
        int received = recv(m_sock, buf + totalReceived, static_cast<int>(length - totalReceived), 0);
        if (received <= 0) {
            if (m_errorCallback) {
                m_errorCallback("Failed to receive data");
            }
            return false;
        }
        totalReceived += received;
    }
    return true;
}

template<typename T>
bool Net_Tool::sendMessage(const T& message, char type) {
    // // 直接序列化消息并发送
    std::string serialized;
    bool retcode = message.SerializeToString(&serialized);
    if (!retcode) {
        if (m_errorCallback) {
            m_errorCallback("Failed to serialize message");
        }
        return false;
    }

    //再包个底层收发的长度和类型头
    char* buf = new char[sizeof(uint64_t)+sizeof(char)+serialized.size()];
    uint64_t data_len = serialized.size();
    char type_char = type;
    memcpy((char*)buf,(char*)&data_len,sizeof(uint64_t));//序列化数据长度
    memcpy(buf+sizeof(uint64_t),&type_char,sizeof(char));//类型
    memcpy(buf+sizeof(uint64_t)+sizeof(char),serialized.c_str(),serialized.size());//序列化数据

    sendData(buf, sizeof(uint64_t)+sizeof(char)+serialized.size());
    free(buf);
    return true;
}

template<typename T>
bool Net_Tool::receiveMessage(T& message, char type) {
    // 接收序列化的消息
    std::string serialized;
    char buff[65535] = {0};//一包长
    int ret=0;
    uint64_t data_len=0;
    char type_char=0;
    bool res = true;

    //先看下数据有多少，在接收
    ret = peek_read(buff, sizeof(buff));
    if(ret <= 0) {
        return false;
    }
    //接收数据
    res = receiveData(buff, ret);
    if(!res) {
        return false;
    }
    //解析底层收发头
    memcpy((char*)&data_len,buff,sizeof(uint64_t));
    memcpy((char*)&type_char,buff+sizeof(uint64_t),sizeof(char));
    if(type_char != type) {
        //类型不对，返回false
        return false;
    }
    //把序列化部分数据拿到
    serialized += std::string(buff+sizeof(uint64_t)+sizeof(char), ret-sizeof(uint64_t)-sizeof(char));

    //一包拿不完，继续拿(整理逻辑不太好，有空再优化)
    uint64_t total_len = ret;
    while(data_len+sizeof(uint64_t)+sizeof(char) > total_len)
    {
        ret = peek_read(buff, sizeof(buff));
        if(ret <= 0) {
            return false;
        }
        //接收数据
        res = receiveData(buff, ret);
        if(!res) {
            return false;
        }
        serialized += std::string(buff, ret);
        total_len += ret;
    }

    // 反序列化消息
    if (!message.ParseFromString(serialized)) {
        if (m_errorCallback) {
            m_errorCallback("Failed to parse message");
        }
        return false;
    }

    return true;
}

int Net_Tool::peek_read(char *buf, int len) {
#ifndef _WIN32
    // 使用临时缓冲区进行MSG_PEEK
    ret = recv(m_sock, peek_buf, sizeof(peek_buf), MSG_PEEK);
    if(-1 == ret)
    {
        perror("readLine error -1");
        return -1;
    }
    return ret;
#else
    int ret = recv(m_sock, buf, len, MSG_PEEK);
    if (ret == SOCKET_ERROR) {
            int errorCode = WSAGetLastError();
            if (errorCode != WSAEINTR) {
                printf("recv error: %d\n", errorCode);
                return -1;
            }
    }
    return ret;
#endif
}

transfer::DirectoryResponse Net_Tool::sendDirectoryRequest(const transfer::DirectoryRequest& request) {
    transfer::DirectoryResponse response;
    if (!sendMessage(request, DIRECTORY_TYPE) || !receiveMessage(response, DIRECTORY_TYPE)) {
        if (m_errorCallback) {
            m_errorCallback("Failed to send directory request");
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
    const std::string& fileName, const std::string& targetPath, uint32_t chunkSize) {
    
    transfer::UploadRequest request;
    request.set_allocated_header(new transfer::RequestHeader(createRequestHeader(transfer::UPLOAD)));
    // 获取文件信息
    std::ifstream file(fileName, std::ios::binary | std::ios::ate);
    if (!file) {
        return request;
    }
    auto fileSize = file.tellg();
    file.seekg(0);

    // 设置文件信息
    auto fileInfo = request.add_files();
    fileInfo->set_file_name(fileName.substr(fileName.find_last_of("/\\") + 1));//文件名
    fileInfo->set_target_path(targetPath);//目标路径
    fileInfo->set_file_size(fileSize);//文件大小
    fileInfo->set_md5(calculateFileMD5(fileName));//文件MD5
    fileInfo->set_need_chunk(fileSize > chunkSize);//是否分片
    fileInfo->set_chunk_size(chunkSize);//分片大小
    fileInfo->set_chunk_sequence(0);//分片序号
    uint64_t read_size = fileSize>chunkSize?chunkSize:fileSize;

    //file.seekg(read_size,std::ios::beg);//跳过已经读取的数据
    std::vector<char> chunk_data(read_size);
    file.read(chunk_data.data(),read_size);
    fileInfo->set_data(chunk_data.data(),read_size);//读取数据

    fileInfo->set_checksum(calculateCRC32(chunk_data.data(),read_size));//校验和，分片情况下才有用
    fileInfo->set_task_id(generateTaskId());//任务ID
    fileInfo->set_status(transfer::INIT);//传输状态
    fileInfo->set_offset(0);//断点续传的起始位置
    fileInfo->set_upload_id(generateTaskId());//上传会话ID(用于断点续传)

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

void Net_Tool::startUploadTask(const std::string& fileName, const std::string& targetPath,
    std::function<void(const transfer::TransferProgressResponse&)> progressCallback) {
    std::string taskId = generateTaskId();
    
    // 使用普通指针
    TransferTask* task = new TransferTask();
    task->taskId = taskId;
    task->fileName = fileName;
    task->targetPath = targetPath;
    task->progressCallback = progressCallback;
    task->isPaused = false;
    task->isCancelled = false;
    
    {
        // 使用互斥锁保护对任务列表的访问
        std::lock_guard<std::mutex> lock(m_tasksMutex);
        // 将新的传输任务添加到任务映射表中
        m_transferTasks[taskId] = task;
    }

    // 使用成员函数替代lambda表达式
    task->transferThread = std::thread(&Net_Tool::handleUploadTask, this, task);
    task->transferThread.detach();//线程分离
}

void Net_Tool::startDownloadTask(const std::string& fileName, const std::string& targetPath,
    std::function<void(const transfer::TransferProgressResponse&)> progressCallback) {
    TransferTask* task = new TransferTask();
    task->taskId = generateTaskId();
    task->fileName = fileName;
    task->targetPath = targetPath;
    task->progressCallback = progressCallback;
    task->isPaused = false;
    task->isCancelled = false;

    {
        std::lock_guard<std::mutex> lock(m_tasksMutex);
        m_transferTasks[task->taskId] = task;
    }

    // 使用成员函数替代lambda表达式
    task->transferThread = std::thread(&Net_Tool::handleDownloadTask, this, task);
    task->transferThread.detach();//线程分离
}

// 上传任务处理函数
void Net_Tool::handleUploadTask(TransferTask* task)
{
    std::ifstream file(task->fileName, std::ios::binary | std::ios::ate);
    if (!file) {
        if (m_errorCallback) {
            m_errorCallback("Failed to open file: " + task->fileName);
        }
        return;
    }

    // 获取文件大小
    task->fileSize = file.tellg();
    // 将文件指针重置到开头
    file.seekg(0);

    // 创建并发送上传请求
    auto request = createUploadRequest(task->fileName, task->targetPath);
    if (!sendMessage(request, UPLOAD_TYPE)) {
        if (m_errorCallback) {
            m_errorCallback("Failed to send upload request");
        }
        file.close();
        return;
    }
    
    transfer::UploadResponse response;
    do
    {
        if (!receiveMessage(response, UPLOAD_TYPE)) {
            if (m_errorCallback) {
                m_errorCallback("Failed to receive upload response");
            }
            file.close();
            return;
        }
        if(response.results().size() > 0
        && response.header().session_id() == request.header().session_id())
        {
            auto fileInfo = response.results(0);
            if(fileInfo.success() == true)
            {//上传成功
                int next_sequence = fileInfo.next_sequence();
                // 更新进度
                if (task->progressCallback) {
                    transfer::TransferProgressResponse progress;
                    progress.set_task_id(task->taskId);
                    progress.set_status(next_sequence > 0 ? transfer::TRANSFERRING : transfer::COMPLETED);
                    progress.set_transferred_size(next_sequence*CHUNK_SIZE);
                    progress.set_total_size(task->fileSize);
                    progress.set_progress(static_cast<uint32_t>((next_sequence * CHUNK_SIZE * 100) / task->fileSize));
                    task->progressCallback(progress);
                }
                if(next_sequence > 0)
                {//分片，还未传完
                    //分片需要更新的参数有：分片序号、数据、校验和、传输状态、断点续传的起始位置
                    //auto req_info = request.mutable_files(0);//可修改
                    for (int i = 0; i < request.files_size(); ++i) 
                    {
                        auto req_info = request.mutable_files(i);//可修改
                        req_info->set_chunk_sequence(next_sequence);//分片序号
                        file.seekg(next_sequence*CHUNK_SIZE,std::ios::beg);//偏移到待取的位置
                        uint64_t next_size = task->fileSize-next_sequence*CHUNK_SIZE;
                        if(next_size > CHUNK_SIZE)
                            next_size = CHUNK_SIZE;
                        std::vector<char> chunk_data(next_size);
                        file.read(chunk_data.data(),next_size);
                        req_info->set_data(chunk_data.data(),next_size);//读取数据
                        req_info->set_checksum(calculateCRC32(chunk_data.data(),next_size));//校验和
                        req_info->set_status(transfer::TRANSFERRING);//传输状态
                        req_info->set_offset(next_sequence*CHUNK_SIZE);//断点续传的起始位置
                        if (!sendMessage(request, UPLOAD_TYPE)) {
                            if (m_errorCallback) {
                                m_errorCallback("Failed to send upload request");
                            }
                            file.close();
                            return;
                        }
                    }
                }
                else
                {//文件上传完成
                    // 发送目录请求以刷新远端目录显示
                    transfer::DirectoryRequest dirRequest;
                    dirRequest.mutable_header()->set_type(transfer::DIRECTORY);
                    dirRequest.set_current_path("");
                    dirRequest.set_dir_name(request.files(0).target_path());
                    dirRequest.set_is_parent(false);
                    transfer::DirectoryResponse response = sendDirectoryRequest(dirRequest);
                    FileClient::instance()->getRemoteView()->dispatchRemoteResponse(response);

                    break;//终止while
                }
            }
            else
            {//上个没成功，重新上传
                if (!sendMessage(request, UPLOAD_TYPE)) {
                    if (m_errorCallback) {
                        m_errorCallback("Failed to send upload request");
                    }
                    file.close();
                    return;
                }
            }
        }
    } while (1);

    file.close();
    
    {   // 使用花括号创建局部作用域,确保互斥锁的RAII特性,锁会在作用域结束时自动释放
        std::lock_guard<std::mutex> lock(m_tasksMutex);  // 加锁保护共享资源m_transferTasks
        auto it = m_transferTasks.find(task->taskId);    // 查找当前任务
        if (it != m_transferTasks.end()) {
            delete it->second;  // 释放任务对象占用的内存
            m_transferTasks.erase(it);  // 从任务映射中移除该任务
        }
    }   // 作用域结束,锁自动释放
}

// 下载任务处理函数
void Net_Tool::handleDownloadTask(TransferTask* task) {
    // 创建并发送下载请求
    auto request = createDownloadRequest(task->fileName, task->targetPath);
    if (!sendMessage(request, DOWNLOAD_TYPE)) {
        if (m_errorCallback) {
            m_errorCallback("Failed to send download request");
        }
        return;
    }

    // 接收下载响应
    transfer::DownloadResponse response;
    if (!receiveMessage(response, DOWNLOAD_TYPE)) {
        if (m_errorCallback) {
            m_errorCallback("Failed to receive download response");
        }
        return;
    }

    // 检查响应结果
    if (response.results().empty() || !response.results(0).exists()) {
        if (m_errorCallback) {
            m_errorCallback("File not found on server");
        }
        return;
    }

    const auto& fileInfo = response.results(0);//响应的数据
    task->fileSize = fileInfo.file_size();

    // 创建输出文件
    std::string target_file = fileInfo.target_path() + "/" + fileInfo.file_name();

    if(fileInfo.need_chunk())
    {
        std::ofstream file(target_file,fileInfo.chunk_sequence()==0?
                std::ios::binary | std::ios::trunc : 
                std::ios::binary | std::ios::app);
        if (!file) {
                //request.files(0)->set_error_message("Failed to open file for writing");
        }
        uint64_t file_total_len = 0;//以获取文件数据长度

        //分片校验
        uint32_t calculated_crc = calculateCRC32(fileInfo.data().c_str(), fileInfo.data().length());
        if(calculated_crc == fileInfo.checksum())
        {
            file.seekp(file_total_len);
            file.write(fileInfo.data().c_str(), fileInfo.data().length());
            file_total_len += fileInfo.data().length();
            //file.close();
            
            auto req_info = request.mutable_files(0);
            req_info->set_offset(file_total_len);
            // 更新进度回调
            if (task->progressCallback) {
                transfer::TransferProgressResponse progress;
                progress.set_task_id(task->taskId);
                progress.set_status(file_total_len < task->fileSize ? 
                                    transfer::TRANSFERRING : transfer::COMPLETED);
                progress.set_transferred_size(file_total_len);
                progress.set_total_size(task->fileSize);
                progress.set_progress(static_cast<uint32_t>((file_total_len * 100) / task->fileSize));
                task->progressCallback(progress);
            }
        }
        else
        {
            if (m_errorCallback) {
                m_errorCallback("Chunk checksum verification failed");
            }
        }
        
        if (!sendMessage(request, DOWNLOAD_TYPE)) {
        if (m_errorCallback) {
                m_errorCallback("Failed to send download request");
            }
            return;
        }
        do
        {
            // 检查任务是否被取消
            if (task->isCancelled) {
                file.close();
                std::remove(target_file.c_str());
                break;
            }
            // 处理暂停状态
            while (task->isPaused && !task->isCancelled) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            if (!receiveMessage(response, DOWNLOAD_TYPE)) {
                if (m_errorCallback) {
                    m_errorCallback("Failed to receive download response");
                }
                return;
            }
            if (response.results().empty() || !response.results(0).exists()) {
                if (m_errorCallback) {
                    m_errorCallback("File not found on server");
                }
                return;
            }

            auto fileInfo = response.results(0);
            //分片校验
            uint32_t calculated_crc = calculateCRC32(fileInfo.data().c_str(), fileInfo.data().length());
            if(calculated_crc == fileInfo.checksum())
            {
                file.seekp(file_total_len);
                file.write(fileInfo.data().c_str(), fileInfo.data().length());
                file_total_len += fileInfo.data().length();
                //file.close();
                auto req_info = request.mutable_files(0);
                req_info->set_offset(file_total_len);
                // 更新进度回调
                if (task->progressCallback) {
                    transfer::TransferProgressResponse progress;
                    progress.set_task_id(task->taskId);
                    progress.set_status(file_total_len < task->fileSize ? 
                                        transfer::TRANSFERRING : transfer::COMPLETED);
                    progress.set_transferred_size(file_total_len);
                    progress.set_total_size(task->fileSize);
                    progress.set_progress(static_cast<uint32_t>((file_total_len * 100) / task->fileSize));
                    task->progressCallback(progress);
                }
            }
            else
            {
                if (m_errorCallback) {
                    m_errorCallback("Chunk checksum verification failed");
                }
            }
            if(fileInfo.is_last() == true)
            {
                file.close();
                //验证文件md5
                std::string downloaded_md5 = calculateFileMD5(target_file);
                if (downloaded_md5 != fileInfo.md5()) {
                    if (m_errorCallback) {
                        m_errorCallback("File MD5 verification failed");
                    }
                    std::remove(target_file.c_str());
                }
                break;
            }
            else
            {
                if (!sendMessage(request, DOWNLOAD_TYPE)) {
                    if (m_errorCallback) {
                        m_errorCallback("Failed to send download request");
                    }
                    return;
                }
            }
        } while (1);
    }
    else
    {
        std::ofstream file(target_file,fileInfo.chunk_sequence()==0?
                std::ios::binary | std::ios::trunc : 
                std::ios::binary | std::ios::app);
        if (!file) {
                //request.files(0)->set_error_message("Failed to open file for writing");
        }
        file.seekp(fileInfo.chunk_size()*fileInfo.chunk_sequence());
        file.write(fileInfo.data().c_str(), fileInfo.data().length());
        file.close();
        std::string downloaded_md5 = calculateFileMD5(target_file);
        if (downloaded_md5 != fileInfo.md5()) {
            if (m_errorCallback) {
                m_errorCallback("File MD5 verification failed");
            }
            std::remove(target_file.c_str());
        }

        // 更新进度回调
        if (task->progressCallback) {
            transfer::TransferProgressResponse progress;
            progress.set_task_id(task->taskId);
            progress.set_status(fileInfo.chunk_size()*fileInfo.chunk_sequence() < task->fileSize ? 
                                transfer::TRANSFERRING : transfer::COMPLETED);
            progress.set_transferred_size(fileInfo.chunk_size()*fileInfo.chunk_sequence());
            progress.set_total_size(task->fileSize);
            progress.set_progress(static_cast<uint32_t>((fileInfo.chunk_size()*fileInfo.chunk_sequence() * 100) / task->fileSize));
            task->progressCallback(progress);
        }

    }

    // 清理任务资源
    {
        std::lock_guard<std::mutex> lock(m_tasksMutex);
        auto it = m_transferTasks.find(task->taskId);
        if (it != m_transferTasks.end()) {
            delete it->second;
            m_transferTasks.erase(it);
        }
    }
}

// 暂停传输任务
void Net_Tool::pauseTransfer(const std::string& taskId) {
    std::lock_guard<std::mutex> lock(m_tasksMutex);
    auto it = m_transferTasks.find(taskId);
    if (it != m_transferTasks.end()) {
        it->second->isPaused = true;
    }
}

// 恢复传输任务
void Net_Tool::resumeTransfer(const std::string& taskId) {
    std::lock_guard<std::mutex> lock(m_tasksMutex);
    auto it = m_transferTasks.find(taskId);
    if (it != m_transferTasks.end()) {
        it->second->isPaused = false;
    }
}

// 取消传输任务
void Net_Tool::cancelTransfer(const std::string& taskId) {
    std::lock_guard<std::mutex> lock(m_tasksMutex);
    auto it = m_transferTasks.find(taskId);
    if (it != m_transferTasks.end()) {
        TransferTask* task = it->second;
        task->isCancelled = true;
        if (task->transferThread.joinable()) {
            task->transferThread.join();
        }
        delete task;  // 释放内存
        m_transferTasks.erase(it);
    }
}
