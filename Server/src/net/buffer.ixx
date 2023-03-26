/*************************************************************************
> File Name       : buffer.ixx
> Brief           : 缓冲区
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年03月15日  15时33分30秒
************************************************************************/

module;
#include <vector>
#include <cassert>
#include <cstdint>
#include <string_view>
#include <concepts>

export module net:buffer;

export namespace net
{
    class Buffer
    {
    public:
        static constexpr size_t CHEAP_PREPEND       = 8;
        static constexpr size_t INITIAL_BUFFER_SIZE = 1024;

        explicit Buffer(size_t initialSize = INITIAL_BUFFER_SIZE)
            : _buffer(CHEAP_PREPEND + initialSize), _readIndex(CHEAP_PREPEND), _writeIndex(CHEAP_PREPEND)
        {
            assert(PrependableBytes() == CHEAP_PREPEND);
            assert(WritableBytes() == initialSize);
            assert(ReadableBytes() == 0);
        }

        Buffer(const Buffer &buffer) = default;

        Buffer &operator=(Buffer buffer)
        {
            swap(buffer);
            return *this;
        }

        Buffer(Buffer &&buffer) noexcept
            : _buffer(std::move(buffer._buffer)),
              _readIndex(buffer._readIndex),
              _writeIndex(buffer._writeIndex)
        {
            buffer._readIndex  = 0;
            buffer._writeIndex = 0;
        }

        Buffer &operator=(Buffer &&buffer) noexcept
        {
            buffer.swap(*this);
            return *this;
        }

        virtual ~Buffer() = default;

        void swap(Buffer &buffer) noexcept
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
         * @brief 可以在前面附加的字节数
         *
         * @return 字节数
         */
        [[nodiscard]] size_t PrependableBytes() const
        {
            return _readIndex;
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
            std::memcpy(Tail(), &data, sizeof(data));
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
        friend Buffer &operator<<(Buffer &buffer, const PODType &data)
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
            std::copy(data, data + len, Tail());
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

        /**
         * @brief 在前面追加数据
         *
         * @param data 数据
         * @param len 长度
         */
        void Prepend(const uint8_t *data, size_t len)
        {
            assert(len <= PrependableBytes());
            _readIndex -= len;
            std::copy(data, data + len, Peek());
        }

        /**
         * @brief 在前面追加数据
         *
         * @param data 数据
         * @param len 长度
         */
        void Prepend(const void *data, size_t len)
        {
            Prepend(static_cast<const uint8_t *>(data), len);
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
            std::memcpy(&result, Peek(), sizeof(result));
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
        friend Buffer &operator>>(Buffer &buffer, PODType &data)
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
            std::string result(reinterpret_cast<const char *>(Peek()), len);
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

        /**
         * @brief 裁剪大小
         *
         * @param reserve
         */
        void Shrink(size_t reserve)
        {
            Buffer other;
            other.EnsureWritableBytes(ReadableBytes() + reserve);
            other.Write(Peek(), ReadableBytes());
            swap(other);
        }

    private:
        /**
         * @brief 头部
         */
        uint8_t *Peek()
        {
            uint8_t *head = &(*_buffer.begin());
            return head + _readIndex;
        }

        /**
         * @brief 尾部
         */
        uint8_t *Tail()
        {
            uint8_t *head = &(*_buffer.begin());
            return head + _writeIndex;
        }

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
            _readIndex  = CHEAP_PREPEND;
            _writeIndex = CHEAP_PREPEND;
        }

        /**
         * @brief 调整空间
         *
         * @param len 需要的大小
         */
        void MakeSpace(size_t len)
        {
            if (WritableBytes() + PrependableBytes() < len + CHEAP_PREPEND)
            {
                // 可写的空间不足，扩容
                _buffer.resize(_writeIndex + len);
            }
            else
            {
                // 前缀空余过多，调整
                assert(CHEAP_PREPEND < _readIndex);
                const size_t readableBytes = ReadableBytes();
                std::copy(Peek(), Tail(), _buffer.begin() + CHEAP_PREPEND);
                _readIndex  = CHEAP_PREPEND;
                _writeIndex = _readIndex + readableBytes;
                assert(readableBytes == ReadableBytes());
            }
        }

    private:
        std::vector<uint8_t> _buffer;
        size_t               _readIndex{};
        size_t               _writeIndex{};
    };

    void swap(Buffer &lhs, Buffer &rhs) noexcept
    {
        lhs.swap(rhs);
    }

} // namespace net