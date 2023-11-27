target("Common")
    set_kind("static")
    add_headerfiles("**.h", "**.hpp")
    add_files("**.cpp", "**.c", "./Net/NetMessageProto/*.proto")

    add_packages("protobuf-cpp")
    add_rules("CommonRule", "protobuf.cpp")

    add_includedirs(
                    "$(projectdir)/3rdParty/asio/asio/include",
                    "$(projectdir)/3rdParty/spdlog/include",
                    "$(projectdir)/3rdParty/magic_enum/include",
                    "$(projectdir)/Src",
                     {public=true}
                     )
    add_deps("asio", "spdlog")

    add_includedirs("$(projectdir)/3rdParty/mysql/include")
    if is_plat("windows") then
        add_linkdirs("$(projectdir)/3rdParty/mysql/lib/windows")
        add_links("libmysql")
    end

    after_build(function (target) 
        if is_plat("windows") then
            os.cp("$(projectdir)/3rdParty/mysql/lib/windows/*.dll", target:targetdir())
        end
    end)
target_end()