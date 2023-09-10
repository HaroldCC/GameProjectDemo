/*************************************************************************
> File Name       : HttpServer.cpp
> Brief           : Http服务器
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月21日  15时53分38秒
************************************************************************/
#include "HttpServer.h"
#include "Common/include/Log.hpp"
#include "Common/include/Assert.h"

using boost::beast::http::field;
using boost::beast::http::status;
using boost::beast::http::verb;

int WideCharFromUtf8(unsigned int *out_char, const char *in_text, const char *in_text_end)
{
    const char     lengths[32] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0};
    const int      masks[]     = {0x00, 0x7f, 0x1f, 0x0f, 0x07};
    const uint32_t mins[]      = {0x400000, 0, 0x80, 0x800, 0x10000};
    const int      shiftc[]    = {0, 18, 12, 6, 0};
    const int      shifte[]    = {0, 6, 4, 2, 0};
    int            len         = lengths[*(const unsigned char *)in_text >> 3];
    int            wanted      = len + (!len ? 1 : 0);

    if (in_text_end == NULL)
        in_text_end = in_text + wanted; // Max length, nulls will be taken into account.

    // Copy at most 'len' bytes, stop copying at 0 or past in_text_end. Branch predictor does a good job here,
    // so it is fast even with excessive branching.
    unsigned char s[4];
    s[0] = in_text + 0 < in_text_end ? in_text[0] : 0;
    s[1] = in_text + 1 < in_text_end ? in_text[1] : 0;
    s[2] = in_text + 2 < in_text_end ? in_text[2] : 0;
    s[3] = in_text + 3 < in_text_end ? in_text[3] : 0;

    // Assume a four-byte character and load four bytes. Unused bits are shifted out.
    *out_char = (uint32_t)(s[0] & masks[len]) << 18;
    *out_char |= (uint32_t)(s[1] & 0x3f) << 12;
    *out_char |= (uint32_t)(s[2] & 0x3f) << 6;
    *out_char |= (uint32_t)(s[3] & 0x3f) << 0;
    *out_char >>= shiftc[len];

    // Accumulate the various error conditions.
    int e = 0;
    e     = (*out_char < mins[len]) << 6;  // non-canonical encoding
    e |= ((*out_char >> 11) == 0x1b) << 7; // surrogate half?
    e |= (*out_char > 0x10FFFF) << 8;      // out of range?
    e |= (s[1] & 0xc0) >> 2;
    e |= (s[2] & 0xc0) >> 4;
    e |= (s[3]) >> 6;
    e ^= 0x2a; // top two bits of each tail byte correct?
    e >>= shifte[len];

    if (e)
    {
        // No bytes are consumed when *in_text == 0 || in_text == in_text_end.
        // One byte is consumed in case of invalid first byte of in_text.
        // All available bytes (at most `len` bytes) are consumed on incomplete/invalid second to last bytes.
        // Invalid or incomplete input may consume less bytes than wanted, therefore every byte has to be inspected in s.
        int cmp   = !!s[0] + !!s[1] + !!s[2] + !!s[3];
        wanted    = wanted < cmp ? wanted : cmp;
        *out_char = 0xFFFD;
    }

    return wanted;
}

std::string FormatUtf8(const char *text, size_t size)
{
    const char hex_digits[] = "0123456789ABCDEF";

    std::string output;
    output.reserve(size * 3);

    for (const char *cursor = text; cursor < text + size; cursor++)
    {
        unsigned char chr = *cursor;
        output.push_back('%');
        output.push_back(hex_digits[chr >> 4]);
        output.push_back(hex_digits[chr & 15]);
    }

    return output;
}

std::string FormatToUTF8(std::string_view str)
{
    std::string result;

    result.reserve(str.size());

    const char *text_begin = str.data();
    const char *text_end   = text_begin + str.size();

    for (const char *cursor = text_begin; cursor < text_end;)
    {
        unsigned int wide_char = *cursor;

        // this is an ascii char
        if (wide_char < 0x80)
        {
            result.push_back(*cursor);
            cursor += 1;
        }
        // this is an utf-8 char
        else
        {
            size_t utf8_len = WideCharFromUtf8(&wide_char, cursor, text_end);

            if (wide_char == 0) // Malformed UTF-8?
                break;

            result += FormatUtf8(cursor, utf8_len);
            cursor += utf8_len;
        }
    }

    return result;
}

void HttpServer::DoAccept()
{
    _accepter.async_accept(
        [this](const std::error_code &errcode, asio::ip::tcp::socket socket)
        {
            if (errcode)
            {
                Log::Error("接受连接失败：{}", errcode.message());
                return;
            }

            auto pSession = std::make_shared<Http::HttpSession>(std::move(socket));
            InitHttpRouter(pSession);
            pSession->StartSession();

            DoAccept();
        });
}

void HttpServer::InitHttpRouter(const std::shared_ptr<Http::HttpSession> &pSession)
{
    Assert(nullptr != pSession);

    pSession->AddRouter(verb::get, "/test", [](const Http::HttpRequest &request) -> std::optional<Http::HttpResponse>
                        {
                            Http::HttpResponse resp;
                            resp.SetStatusCode(status::ok);
                            resp.SetHeader(field::content_type, "text/html;charset=utf-8");
                            std::string_view body = "<!DOCTYPE html><html><head><title>Title</title></head><body><h1>This is an awesome HTML测试成功</h1></body></html>";
                            resp.SetContent(FormatToUTF8(body));
                            // resp.SetHeader(field::body, body);
                            // resp.SetHeader(field::content_length, std::to_string(body.size()));
                            // body = std::format("{}\r\n\r\n{}", body.size(), body);
                            // time_t rawTime;
                            // struct tm*timeInfo;
                            // time(&rawTime);
                            // timeInfo = gmtime(&rawTime);
                            // char  strTime[1024]={0};
                            // strftime(strTime, 1024, "%a, %d %b %Y %H:%M:%S GMT", timeInfo);
                            // // resp.SetHeader(field::date, strTime);
                            // resp.SetHeader(field::content_length, body);
                            // resp.SetBody(body);
                            return resp; });
}