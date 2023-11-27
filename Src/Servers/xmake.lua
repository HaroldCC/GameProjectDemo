target("HttpServer")
    set_kind("binary")
    add_headerfiles("**.h")
    add_files("**.cpp")
    
    add_rules("CommonRule")

    add_deps("asio", "spdlog", "Common")

target_end()