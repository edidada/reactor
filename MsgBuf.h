#pragma once

#include <bits/types/struct_iovec.h>
#include <cerrno>
#include <cstdint>
#include <vector>
#include <cstring>
#include <algorithm>
#include <unistd.h>
#include <sys/uio.h>

class MsgBuf {
public:
    MsgBuf() : rpos_(0), wpos_(0) {}

    explicit MsgBuf(std::size_t size) : buffer_(size), rpos_(0), wpos_(0) {
        buffer_.resize(size);
    }

    // 不允许拷贝
    MsgBuf(const MsgBuf&) = delete;
    MsgBuf& operator=(const MsgBuf&) = delete;

    // 允许移动
    MsgBuf(MsgBuf&& other) noexcept
        : buffer_(std::move(other.buffer_)), rpos_(other.rpos_), wpos_(other.wpos_) {
        other.rpos_ = 0;
        other.wpos_ = 0;
    }
    MsgBuf& operator=(MsgBuf&& other) noexcept {
        if (this != &other) {
            buffer_ = std::move(other.buffer_);
            rpos_ = other.rpos_;
            wpos_ = other.wpos_;
            other.rpos_ = 0;
            other.wpos_ = 0;
        }
        return *this;
    }

    // 动态数组的起始地址
    uint8_t* GetBasePointer() {
        return buffer_.data();
    }

    // 消费者
    uint8_t* GetReadPointer() {
        return buffer_.data() + rpos_;
    }

    // 生产者
    uint8_t* GetWritePointer() {
        return buffer_.data() + wpos_;
    }

    // 消费数据
    void ReadCompleted(std::size_t len) {
        rpos_ += len;
    }

    // 生产数据
    void WriteCompleted(std::size_t len) {
        wpos_ += len;
    }

    std::size_t GetActiveSize() const {
        return wpos_ - rpos_;
    }

    std::size_t GetFreeSize() const {
        return buffer_.size() - wpos_;
    }

    std::size_t GetBufferSize() const {
        return buffer_.size();
    }

    // 腾挪数据
    void Normalize() {
        if (rpos_ > 0) {
            std::memmove(buffer_.data(), GetReadPointer(), GetActiveSize());
            wpos_ -= rpos_;
            rpos_ = 0;
        }
    }

    void EnsureFreeSpace(std::size_t size) {
        if (GetBufferSize() - GetActiveSize() < size) {
            Normalize();
            buffer_.resize(buffer_.size() + std::max(size, buffer_.size() / 2));
        } else if (GetFreeSize() < size) {
            Normalize();
        }
    }

    void Write(const uint8_t *data, std::size_t size) {
        if (size > 0) {
            EnsureFreeSpace(size);
            std::memcpy(GetWritePointer(), data, size);
            WriteCompleted(size);
        }
    }

    int Recv(int fd, int *err) {
        char extra[65535];
        struct iovec iov[2];

        iov[0].iov_base = GetWritePointer();
        iov[0].iov_len = GetFreeSize();
        iov[1].iov_base = extra;
        iov[1].iov_len = sizeof(extra);
        std::size_t n = readv(fd, iov, 2);
        if (n < 0) {
            *err = errno;
            return -1;
        } else if (n == 0) {
            *err = ECONNRESET;
            return 0;
        } else if (n <= GetFreeSize()) {
            WriteCompleted(n);
            return n;
        } else {
            std::size_t extra_size = n - GetFreeSize();
            WriteCompleted(GetFreeSize());
            Write(reinterpret_cast<uint8_t*>(extra), extra_size);
            return n;
        }
    }
private:
    std::vector<uint8_t> buffer_;
    std::size_t rpos_;
    std::size_t wpos_;
};