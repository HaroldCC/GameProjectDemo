/*************************************************************************
> File Name       : HttpRouter.h
> Brief           : Http路由功能
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月17日  20时28分46秒
************************************************************************/
#pragma once
#include <functional>
#include <utility>
#include "HttpRequest.h"
#include "HttpResponse.h"
#include <map>
#include <optional>

namespace Http
{
    using HttpHandlerFunc = std::function<std::optional<HttpResponse>(const HttpRequest &)>;

    template <typename HandlerFunc>
    concept http_handler_type = std::is_invocable_r_v<std::optional<HttpResponse>, HandlerFunc, const HttpRequest &>;

    template <http_handler_type HandlerFunc>
    class HttpRouteTrie final
    {
    private:
        struct TrieNode
        {
            std::unordered_map<std::string_view, std::unique_ptr<TrieNode>> _children;
            HandlerFunc                                                     _handler;
            bool                                                            _isLeaf{false};
        };

    public:
        void AddNode(std::string_view path, HandlerFunc handler)
        {
            if (path.empty())
            {
                return;
            }

            if (_root == nullptr)
            {
                _root = std::make_unique<TrieNode>();
            }

            // 跳过第一个字符'/'，从根节点的子节点开始添加
            size_t    delimiterPos = 1;
            TrieNode *pCurrentNode = _root.get();
            while (delimiterPos < path.size())
            {
                std::size_t      nextDelimiterPos = path.find('/', delimiterPos);
                std::string_view currentNodeName  = path.substr(delimiterPos, nextDelimiterPos - delimiterPos);

                if (!currentNodeName.empty())
                {
                    if (pCurrentNode->_children.find(currentNodeName) == pCurrentNode->_children.end())
                    {
                        pCurrentNode->_children[currentNodeName] = std::make_unique<TrieNode>();
                    }
                    pCurrentNode = pCurrentNode->_children[currentNodeName].get();
                }

                if (nextDelimiterPos == std::string_view::npos)
                {
                    break;
                }

                delimiterPos = nextDelimiterPos + 1;
            }

            pCurrentNode->_isLeaf  = true;
            pCurrentNode->_handler = handler;
        }

        [[nodiscard]] std::optional<HandlerFunc> FindNode(std::string_view path) const
        {
            if (path.empty())
            {
                return std::nullopt;
            }

            const TrieNode *pCurrentNode = _root.get();
            // 跳过第一个字符'/'，从根节点的子节点开始添加
            size_t delimiterPos = 1;
            while (delimiterPos < path.size())
            {
                size_t           nextDelimiterPos = path.find('/', delimiterPos);
                std::string_view currentNodeName  = path.substr(delimiterPos, nextDelimiterPos - delimiterPos);
                if (!currentNodeName.empty())
                {
                    auto iter = pCurrentNode->_children.find(currentNodeName);
                    if (iter == pCurrentNode->_children.end())
                    {
                        return std::nullopt;
                    }

                    pCurrentNode = iter->second.get();
                }

                if (nextDelimiterPos == std::string_view::npos)
                {
                    break;
                }

                delimiterPos = nextDelimiterPos + 1;
            }

            return pCurrentNode->_isLeaf ? std::make_optional(pCurrentNode->_handler) : std::nullopt;
        }

    private:
        std::unique_ptr<TrieNode> _root;
    };

    class HttpRouter final
    {
    public:
        std::optional<HttpResponse> Handle(const HttpRequest &req);

        void AddRouter(Verb method, std::string_view path, HttpHandlerFunc handler);
        void AddRouter(std::string_view method, std::string_view path, HttpHandlerFunc handler);

    private:
        std::map<Verb, HttpRouteTrie<Http::HttpHandlerFunc>> _tree;
    };
} // namespace Http
