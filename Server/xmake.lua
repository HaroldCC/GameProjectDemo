add_requires("spdlog", "doctest", "toml++")

target("Common")
    set_kind("binary")
    add_packages("spdlog", "doctest", "toml++")
    add_files("src/Common/*.cpp", "src/Common/*.ixx")
    ExterRule()
