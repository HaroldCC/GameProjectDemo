
#include <format>
#include <string_view>

#include "HttpCommon.h"
#include "Common/Util/Util.hpp"

#include "magic_enum/magic_enum.hpp"

#if defined(DELETE)
    #undef DELETE
#endif

using namespace std::string_view_literals;

namespace Http
{
    inline std::string StatusToResponseHead(Status status)
    {
        return std::format("HTTP/1.1 {} {}\r\n", Util::ToUnderlying(status), magic_enum::enum_name(status));
    }

    inline std::string_view ContentTypeString(ContentType type)
    {
        switch (type)
        {
            case ContentType::String:
                return "Content-Type: text/plain; charset=UTF-8\r\n"sv;
            case ContentType::Html:
                return "Content-Type: text/html; charset=UTF-8\r\n"sv;
            case ContentType::Json:
                return "Content-Type: application/json; charset=UTF-8\r\n"sv;
                break;
        }

        return {};
    }

    inline std::string StatusToResponseContent(Status status)
    {
        return std::format(R"(
                <html>
                <head><title>{}</title></head>
                <body><h1>{} {}</h1></body>
                </html>)",
                           magic_enum::enum_name(status),
                           Util::ToUnderlying(status),
                           magic_enum::enum_name(status));
    }

    inline constexpr std::string_view HttpMethodToString(HttpMethod mtd)
    {
        switch (mtd)
        {
            case HttpMethod::UNKNOWN:
                return "UNKNOWN"sv;
            case HttpMethod::DELETE:
                return "DELETE"sv;
            case HttpMethod::GET:
                return "GET"sv;
            case HttpMethod::HEAD:
                return "HEAD"sv;
            case HttpMethod::POST:
                return "POST"sv;
            case HttpMethod::PUT:
                return "PUT"sv;
            case HttpMethod::PATCH:
                return "PATCH"sv;
            case HttpMethod::CONNECT:
                return "CONNECT"sv;
            case HttpMethod::OPTIONS:
                return "OPTIONS"sv;
        };

        return {};
    }

    inline constexpr HttpMethod StringToHttpMethod(std::string_view mtd)
    {
        if (mtd.size() < 3)
        {
            return HttpMethod::UNKNOWN;
        }

        char c = mtd[0];
        mtd.remove_prefix(1);
        switch (c)
        {
            case 'C':
            {
                if (mtd == "ONNECT"sv)
                {
                    return HttpMethod::CONNECT;
                }
            }
            break;
            case 'D':
            {
                if (mtd == "ELETE"sv)
                {
                    return HttpMethod::DELETE;
                }
            }
            break;
            case 'G':
            {
                if (mtd == "ET"sv)
                {
                    return HttpMethod::GET;
                }
            }
            break;
            case 'H':
            {
                if (mtd == "EAD"sv)
                {
                    return HttpMethod::HEAD;
                }
            }
            break;
            case 'O':
            {
                if (mtd == "PTIONS"sv)
                {
                    return HttpMethod::OPTIONS;
                }
            }
            break;
            case 'P':
            {
                c = mtd[0];
                mtd.remove_prefix(1);
                switch (c)
                {
                    case 'A':
                    {
                        if (mtd == "TCH"sv)
                        {
                            return HttpMethod::PATCH;
                        }
                    }
                    break;

                    case 'O':
                    {
                        if (mtd == "ST"sv)
                        {
                            return HttpMethod::POST;
                        }
                    }
                    break;
                    case 'U':
                    {
                        if (mtd == "T"sv)
                        {
                            return HttpMethod::PUT;
                        }
                    }
                    break;
                    default:
                        break;
                }
            }
            break;
        }

        return HttpMethod::UNKNOWN;
    }
} // namespace Http