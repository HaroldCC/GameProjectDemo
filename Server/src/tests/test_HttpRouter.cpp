#include <iostream>
#include <format>
#include <functional>
#include <doctest/doctest.h>
#include "Common/http/HttpRouter.h"
#include "Common/include/Log.hpp"

using namespace std::literals;

void test(const Http::HttpRequest &, const Http::HttpResponse &)
{
    Log::Info("test");
}

// TEST_CASE("Trie tests")
// {
//     auto lambda = [](const Http::HttpRequest &req, const Http::HttpResponse &res)
//     {
//         Log::info("lambda");
//     };

//     Http::HttpHandlerFunc func = [](const Http::HttpRequest &) -> std::optional<Http::HttpResponse>
//     { Log::info("httpHandlerFunc"); };

//     Http::HttpRouteTrie<Http::HttpHandlerFunc> trie;
//     trie.AddNode("/", lambda);
//     trie.AddNode("/", test);
//     trie.AddNode("/home", test);
//     trie.AddNode("/login", test);
//     trie.AddNode("/index", test);
//     trie.AddNode("/login/123", func);

//     if (auto ret = trie.FindNode("/"); ret.has_value())
//     {
//         auto func = ret.value();
//     }

//     if (auto ret = trie.FindNode("/login"); ret.has_value())
//     {
//         auto func = ret.value();
//     }

//     if (auto ret = trie.FindNode("/login/123"); ret.has_value())
//     {
//         auto func = ret.value();
//     }

//     if (auto ret = trie.FindNode("/login/456"); ret.has_value())
//     {
//         auto func = ret.value();
//     }

//     if (auto ret = trie.FindNode("/login/1223/456"); ret.has_value())
//     {
//         auto func = ret.value();
//     }
// }
