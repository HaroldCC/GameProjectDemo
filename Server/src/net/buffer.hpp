/*************************************************************************
> File Name       : buffer.h
> Brief           : 缓冲区
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年03月15日  15时33分30秒
************************************************************************/
#pragma once

#include <vector>
#include <cassert>
#include <cstdint>
#include <string_view>
#include <concepts>

namespace net
{
    class MessageBuffer
    {
    public:
        static constexpr size_t INITIAL_BUFFER_SIZE = 1024;

        explicit MessageBuffer(size_t initialSize = INITIAL_BUFFER_SIZE)
            : _buffer(initialSize)
        {
        }

        MessageBuffer(const MessageBuffer &buffer) = default;

        MessageBuffer &operator=(MessageBuffer buffer)
        {
            swap(buffer);
            return *this;
        }

        MessageBuffer(MessageBuffer &&buffer) noexcept
            : _buffer(std::move(buffer._buffer)),
              _readIndex(buffer._readIndex),
              _writeIndex(buffer._writeIndex)
        {
            buffer._readIndex  = 0;
            buffer._writeIndex = 0;
        }

        MessageBuffer &operator=(MessageBuffer &&buffer) noexcept
        {
            buffer.swap(*this);
            return *this;
        }

        virtual ~MessageBuffer() = default;

        void swap(MessageBuffer &buffer) noexcept
        {
            _buffer.swap(buffer._buffer);
            std::swap(_readIndex, buffer._readIndex);
            std::swap(_writeIndex, buffer._writeIndex);
        }

        /**
         * @brief 可以读取的字节数
         *
         * @return 字节数
         */
        [[nodiscard]] size_t ReadableBytes() const
        {
            return _writeIndex - _readIndex;
        }

        /**
         * @brief 可以写入的字节数
         *
         * @return 字节数
         */
        [[nodiscard]] size_t WritableBytes() const
        {
            return _buffer.size() - _writeIndex;
        }

        /**
         * @brief 确保内容可写
         *
         * @param len 需要长度
         */
        void EnsureWritableBytes(size_t len)
        {
            if (WritableBytes() < len)
            {
                MakeSpace(len);
            }

            assert(WritableBytes() >= 0);
        }

        uint8_t *GetBasePointer()
        {
            return _buffer.data();
        }

        uint8_t *GetReadPointer()
        {
            return GetBasePointer() + _readIndex;
        }

        uint8_t *GetWriterPointer()
        {
            return GetBasePointer() + _writeIndex;
        }

        /**
         * @brief 写入pod类型数据
         *
         * @param data 数据
         */
        template <typename PODType>
            requires std::is_standard_layout_v<PODType> && std::is_trivial_v<PODType>
        void Write(const PODType &data)
        {
            EnsureWritableBytes(sizeof(data));
            std::memcpy(GetWriterPointer(), &data, sizeof(data));
            _writeIndex += sizeof(data);
        }

        /**
         * @brief 流方式写入pod类型数据
         *
         * @param buffer 缓冲区
         * @param data 数据
         */
        template <typename PODType>
            requires std::is_standard_layout_v<PODType> && std::is_trivial_v<PODType>
        friend MessageBuffer &operator<<(MessageBuffer &buffer, const PODType &data)
        {
            buffer.Write(data);
            return buffer;
        }

        /**
         * @brief 写入数据
         *
         * @param data 数据
         * @param len 长度
         */
        void Write(std::string_view str)
        {
            Write(str.data(), str.size());
        }

        /**
         * @brief 写入数据
         *
         * @param data 数据
         * @param len 长度
         */
        void Write(const uint8_t *data, size_t len)
        {
            EnsureWritableBytes(len);
            std::copy(data, data + len, GetWriterPointer());
            _writeIndex += len;
        }

        /**
         * @brief 写入数据
         *
         * @param data 数据
         * @param len 长度
         */
        void Write(const void *data, size_t len)
        {
            const uint8_t *byteData = static_cast<const uint8_t *>(data);
            Write(byteData, len);
        }

        void WriteDone(size_t len)
        {
            _writeIndex += len;
        }

        /**
         * @brief 读取数据
         *
         * @return 数据
         */
        template <typename PODType>
            requires std::is_standard_layout_v<PODType> && std::is_trivial_v<PODType>
        PODType Read()
        {
            PODType result;
            std::memcpy(&result, GetReadPointer(), sizeof(result));
            Adjustment(sizeof(PODType));
            return result;
        }

        /**
         * @brief 流方式读取数据
         *
         * @param buffer 缓冲区
         * @param data 数据
         */
        template <typename PODType>
            requires std::is_standard_layout_v<PODType> && std::is_trivial_v<PODType>
        friend MessageBuffer &operator>>(MessageBuffer &buffer, PODType &data)
        {
            data = buffer.Read<PODType>();
            return buffer;
        }

        /**
         * @brief 以字符串形式读取数据
         *
         * @param len 要读取的长度
         * @return 结果
         */
        std::string ReadAsString(size_t len)
        {
            assert(len <= ReadableBytes());
            std::string result(reinterpret_cast<const char *>(GetReadPointer()), len);
            Adjustment(len);
            return result;
        }

        /**
         * @brief 讲当前的缓冲区所有内容以字符串形式读出
         *
         * @return 结果
         */
        std::string ReadAllAsString()
        {
            return ReadAsString(ReadableBytes());
        }

        void ReadDone(size_t len)
        {
            _readIndex += len;
        }

        /**
         * @brief 裁剪大小
         *
         * @param reserve
         */
        void Shrink(size_t reserve)
        {
            MessageBuffer other;
            other.EnsureWritableBytes(ReadableBytes() + reserve);
            other.Write(GetWriterPointer(), ReadableBytes());
            swap(other);
        }

    private:
        /**
         * @brief 调整存取位置和大小
         * @param len 需要的大小
         */
        void Adjustment(size_t len)
        {
            assert(len <= ReadableBytes());
            if (len < ReadableBytes())
            {
                _readIndex += len;
            }
            else
            {
                AdjustmentAll();
            }
        }

        /**
         * @brief 全部重置
         */
        void AdjustmentAll()
        {
            _readIndex  = 0;
            _writeIndex = 0;
        }

        /**
         * @brief 调整空间
         *
         * @param len 需要的大小
         */
        void MakeSpace(size_t len)
        {
            if (WritableBytes() < len)
            {
                // 可写的空间不足，扩容
                _buffer.resize(_writeIndex + len);
            }
            else
            {
                // 前缀空余过多，调整
                const size_t readableBytes = ReadableBytes();
                std::copy(GetReadPointer(), GetWriterPointer(), _buffer.begin());
                _readIndex  = 0;
                _writeIndex = _readIndex + readableBytes;
                assert(readableBytes == ReadableBytes());
            }
        }

    private:
        std::vector<uint8_t> _buffer;
        size_t               _readIndex{};
        size_t               _writeIndex{};
    };

    void swap(MessageBuffer &lhs, MessageBuffer &rhs) noexcept
    {
        lhs.swap(rhs);
    }

} // namespace net