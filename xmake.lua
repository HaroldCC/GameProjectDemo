set_version("0.0.1", {build = "%Y%m%d%H%m"})

set_warnings("all", "error")
set_languages("c++20")

function ExterRule() 
    if is_mode("debug") then
        add_defines("DEBUG")
        set_suffixname("_d")
    end

    set_targetdir("$(projectdir)/Bin/" .. "$(plat)-$(arch)-$(mode)")
end

add_rules("mode.release", "mode.debug")


includes("Server")