#include <doctest/doctest.h>
import common;
import net;

using net::Buffer;

TEST_CASE("Test Buffer Write Read")
{
    {
        net::Buffer buffer;
        CHECK_EQ(buffer.ReadableBytes(), 0);

        const std::string str(200, 'x');
        buffer.Write(str.data(), str.size());
        CHECK_EQ(buffer.ReadableBytes(), str.size());
        CHECK_EQ(buffer.WritableBytes(), net::Buffer::INITIAL_BUFFER_SIZE - str.size());
        CHECK_EQ(buffer.PrependableBytes(), net::Buffer::CHEAP_PREPEND);

        const std::string str2 = buffer.ReadAsString(50);
        CHECK_EQ(str2.size(), 50);
        CHECK_EQ(buffer.ReadableBytes(), str.size() - str2.size());
        CHECK_EQ(buffer.WritableBytes(), Buffer::INITIAL_BUFFER_SIZE - str.size());
        CHECK_EQ(buffer.PrependableBytes(), Buffer::CHEAP_PREPEND + str2.size());
        CHECK_EQ(str2, std::string(50, 'x'));

        buffer.Write(str);
        CHECK_EQ(buffer.ReadableBytes(), 2 * str.size() - str2.size());
        CHECK_EQ(buffer.WritableBytes(), net::Buffer::INITIAL_BUFFER_SIZE - 2 * str.size());
        CHECK_EQ(buffer.PrependableBytes(), net::Buffer::CHEAP_PREPEND + str2.size());

        const std::string str3 = buffer.ReadAllAsString();
        CHECK_EQ(str3.size(), 350);
        CHECK_EQ(buffer.ReadableBytes(), 0);
        CHECK_EQ(buffer.WritableBytes(), net::Buffer::INITIAL_BUFFER_SIZE);
        CHECK_EQ(buffer.PrependableBytes(), net::Buffer::CHEAP_PREPEND);
        CHECK_EQ(str3, std::string(350, 'x'));
    }

    Buffer            bufferVec;
    std::vector<char> vecChar(50, 'a');
    bufferVec.Write(vecChar.data(), vecChar.size());
    CHECK_EQ(vecChar.size(), 50);
    CHECK_EQ(bufferVec.ReadableBytes(), vecChar.size());
    CHECK_EQ(bufferVec.WritableBytes(), Buffer::INITIAL_BUFFER_SIZE - vecChar.size());
    CHECK_EQ(bufferVec.PrependableBytes(), Buffer::CHEAP_PREPEND);
    CHECK_EQ(vecChar, std::vector<char>(50, 'a'));

    // 测试中文
    net::Buffer       buffer;
    const std::string str("你好，缓冲区，hello world");
    buffer.Write(str.data(), str.size());
    CHECK_EQ(buffer.ReadableBytes(), str.size());
    CHECK_EQ(buffer.WritableBytes(), net::Buffer::INITIAL_BUFFER_SIZE - str.size());
    CHECK_EQ(buffer.PrependableBytes(), net::Buffer::CHEAP_PREPEND);

    const std::string str2 = buffer.ReadAllAsString();
    CHECK_EQ(str2, "你好，缓冲区，hello world");

    net::Buffer buffer2;
    buffer2.Write(1000);
    buffer2.Write(3.14);
    buffer2.Write(1.2f);
    size_t size = sizeof(int) + sizeof(double) + sizeof(float);
    CHECK_EQ(buffer2.ReadableBytes(), size);
    CHECK_EQ(buffer2.WritableBytes(), net::Buffer::INITIAL_BUFFER_SIZE - size);
    CHECK_EQ(buffer2.PrependableBytes(), net::Buffer::CHEAP_PREPEND);

    CHECK_EQ(buffer2.Read<int>(), 1000);
    CHECK_EQ(buffer2.Read<double>(), 3.14);
    CHECK_EQ(buffer2.Read<float>(), 1.2f);
    CHECK_EQ(buffer2.ReadableBytes(), 0);
    CHECK_EQ(buffer2.WritableBytes(), net::Buffer::INITIAL_BUFFER_SIZE);

    struct UserData
    {
        int    a;
        int    b;
        double c;
        float  d;

        // UserData()
        //     : a(0), b(b), c(c), d(0)
        // {
        // }
    };

    UserData data{1, 2, 3.14159, 2.2f};
    buffer2.Write(data);
    UserData data2 = buffer2.Read<UserData>();
    CHECK_EQ(data2.a, data.a);
    CHECK_EQ(data2.b, data.b);
    CHECK_EQ(data2.c, data.c);
    CHECK_EQ(data2.d, data.d);

    net::Buffer buffer3;
    int         a = 1;
    double      b = 2.55;
    float       c = 3.2f;
    buffer3 << data << a << b << c;
    UserData data3;
    int      a1 = 0;
    double   b1 = 0;
    float    c1 = 0.0f;
    buffer3 >> data3 >> a1 >> b1 >> c1;
    CHECK_EQ(data3.a, data.a);
    CHECK_EQ(data3.b, data.b);
    CHECK_EQ(data3.c, data.c);
    CHECK_EQ(data3.d, data.d);
    CHECK_EQ(a, a1);
    CHECK_EQ(b, b1);
    CHECK_EQ(c, c1);

    CHECK_EQ(buffer3.ReadableBytes(), 0);
    CHECK_EQ(buffer3.WritableBytes(), net::Buffer::INITIAL_BUFFER_SIZE);
}

TEST_CASE("Test Buffer Grow Shrink")
{
    Buffer buf;
    buf.Write(std::string(800, 'y'));
    CHECK_EQ(buf.ReadableBytes(), 800);
    CHECK_EQ(buf.WritableBytes(), Buffer::INITIAL_BUFFER_SIZE - 800);

    buf.Write(std::string(500, 'x'));
    buf << 1;
    CHECK_EQ(buf.ReadableBytes(), 1304);
    CHECK(buf.WritableBytes() + buf.ReadableBytes() > Buffer::INITIAL_BUFFER_SIZE + Buffer::CHEAP_PREPEND);
    CHECK_EQ(buf.PrependableBytes(), Buffer::CHEAP_PREPEND);

    std::string str = buf.ReadAsString(800);
    CHECK_EQ(str.size(), 800);

    buf.Shrink(0);
    CHECK_EQ(buf.ReadableBytes(), 504);
    str = buf.ReadAsString(500);
    CHECK_EQ(str, std::string(500, 'x'));

    int o = 0;
    buf >> o;
    CHECK_EQ(1, o);

    CHECK_EQ(buf.ReadableBytes(), 0);
}

TEST_CASE("Test Buffer Prepend")
{
    Buffer buffer;
    buffer.Write(std::string(200, 'y'));
    CHECK_EQ(buffer.ReadableBytes(), 200);
    CHECK_EQ(buffer.WritableBytes(), Buffer::INITIAL_BUFFER_SIZE - 200);
    CHECK_EQ(buffer.PrependableBytes(), Buffer::CHEAP_PREPEND);

    int head = 123;
    buffer.Prepend(&head, sizeof(head));
    CHECK_EQ(buffer.ReadableBytes(), 204);
    CHECK_EQ(buffer.WritableBytes(), Buffer::INITIAL_BUFFER_SIZE - 200);
    CHECK_EQ(buffer.PrependableBytes(), Buffer::CHEAP_PREPEND - 4);
}