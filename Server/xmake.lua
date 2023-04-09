add_requires("spdlog", "doctest", "toml++", "asio", "protobuf-cpp")

target("Common")
    set_kind("static")
    add_packages("spdlog", "doctest", "toml++", "asio", "protobuf-cpp")
    add_includedirs("src")
    -- add_rules("protobuf.cpp")
    -- add_files("src/proto/*.proto", {rules="protobuf.cpp"})
    set_pcxxheader("src/Common/pch.h")
    add_files("src/Common/*.cpp")
    add_rules("protobuf.cpp")
    add_headerfiles("src/Common/**.hpp", "src/Common/**.h")
    add_headerfiles("src/net/**.hpp")
    add_files("src/net/MessageProto/**.proto", "src/net/**.cpp")
    ExterRule()

target("tests")
    set_kind("binary")
    add_packages("spdlog", "doctest", "toml++", "asio", "protobuf-cpp")
    add_deps("Common")
    add_includedirs("src")
    add_headerfiles("src/tests/*.h")
    add_files("src/tests/test.cpp")
    ExterRule()
    after_build(function (target) 
        os.cp("$(curdir)/conf/", target:targetdir())
    end)

target("ServerDemo")
    set_kind("binary")
    add_packages("spdlog", "doctest", "toml++", "asio","protobuf-cpp")
    add_deps("Common")
    add_includedirs("src")
    add_files("src/tests/ServerDemo.cpp")
    add_rules("protobuf.cpp")
    add_files("src/net/MessageProto/**.proto")
    ExterRule()
target("ClientDemo")
    set_kind("binary")
    add_packages("spdlog", "doctest", "toml++", "asio", "protobuf-cpp")
    add_deps("Common")
    add_includedirs("src")
    add_rules("protobuf.cpp")
    add_files("src/tests/ClientDemo.cpp")
    add_files("src/net/MessageProto/**.proto")
    ExterRule()

target("testClient")
    set_kind("binary")
    add_packages("spdlog", "doctest", "toml++", "asio", "protobuf-cpp")
    add_deps("Common")
    add_includedirs("src/")
    -- add_rules("protobuf.cpp")
    add_files("src/proto/*.proto", {rules="protobuf.cpp"})
    add_files("src/tests/test_sessionClient.cpp")
    ExterRule()
