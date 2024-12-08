#ifndef TRANSFERPROTOCOL_H
#define TRANSFERPROTOCOL_H

#include <cstdint>
#include <string>

// 消息类型
enum class MessageType : uint8_t {
    // 连接相关
    CONNECT_REQUEST = 0x01,
    CONNECT_RESPONSE = 0x02,
    DISCONNECT_REQUEST = 0x03,
    DISCONNECT_RESPONSE = 0x04,
    
    // 目录操作相关
    DIRECTORY_REQUEST = 0x10,
    DIRECTORY_RESPONSE = 0x11,
    
    // 文件传输相关
    UPLOAD_REQUEST = 0x20,
    UPLOAD_RESPONSE = 0x21,
    UPLOAD_DATA = 0x22,
    UPLOAD_COMPLETE = 0x23,
    
    DOWNLOAD_REQUEST = 0x30, 
    DOWNLOAD_RESPONSE = 0x31,
    DOWNLOAD_DATA = 0x32,
    DOWNLOAD_COMPLETE = 0x33,
    
    // 传输控制相关
    TRANSFER_CONTROL_REQUEST = 0x40,
    TRANSFER_CONTROL_RESPONSE = 0x41,
    
    // 传输进度相关
    TRANSFER_PROGRESS_REQUEST = 0x50,
    TRANSFER_PROGRESS_RESPONSE = 0x51,
    
    // 错误响应
    ERROR_RESPONSE = 0xFF
};

// 传输控制类型
enum class ControlType : uint8_t {
    PAUSE = 0,
    RESUME = 1,
    CANCEL = 2
};

// 消息头部
struct MessageHeader {
    MessageType type;      // 消息类型
    uint32_t length;       // 消息体长度
    uint32_t sequence;     // 序列号
    char checksum[16];     // 校验和
};

// 定义最大分片大小(1MB)
const uint32_t MAX_CHUNK_SIZE = 1024 * 1024;

#endif // TRANSFERPROTOCOL_H
