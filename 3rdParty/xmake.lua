rule("3rdPartyRule")
    on_load(function (target) 
        target:set("targetdir", target:targetdir().."/3rdParty")
    end)
rule_end()

target("asio")
    set_kind("static")
    add_rules("CommonRule", "3rdPartyRule")
    set_group("3rdParty")
    add_headerfiles("./asio/asio/include/*.hpp", {prefixdir = "asio"})
    add_defines("ASIO_SEPARATE_COMPILATION")
    add_includedirs("./asio/asio/include")
    add_files("./asio/asio/src/asio.cpp")
target_end()

target("spdlog")
    set_kind("static")
    add_rules("3rdPartyRule")
    set_group("3rdParty")
    add_headerfiles("./spdlog/include/spdlog/**.h", {prefixdir="spdlog"})
    add_includedirs("./spdlog/include")
    add_defines("SPDLOG_COMPILED_LIB")
    add_files("./spdlog/src/*.cpp")
target_end()

target("lualib")
    add_rules("3rdPartyRule")
    set_kind("static")
    set_group("3rdParty/lua")
    set_basename("lua")
    add_headerfiles("./lua/*.h", {prefixdir="lua"})
    add_files("./lua/*.c|lua.c|luac.c|onelua.c")
    add_defines("LUA_COMPAT_5_2", "LUA_COMPAT_5_1")
    if is_plat("linux", "bsd", "cross") then
        add_defines("LUA_USE_LINUX")
        add_defines("LUA_DL_DLOPEN")
    elseif is_plat("macosx", "iphoneos") then
        add_defines("LUA_USE_MACOSX")
        add_defines("LUA_DL_DYLD")
    elseif is_plat("windows", "mingw") then
        -- Lua already detects Windows and sets according defines
        if is_kind("shared") then
            add_defines("LUA_BUILD_AS_DLL", {public = true})
        end
    end
target_end()

target("lua")
    add_rules("3rdPartyRule")
    set_group("3rdParty/lua")
    set_enabled(enabled)
    set_kind("binary")
    add_files("./lua/lua.c")
    add_deps("lualib")
    if not is_plat("windows", "mingw") then
        add_syslinks("dl")
    end
target_end()