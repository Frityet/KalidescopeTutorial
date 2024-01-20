--Config:
local packages = {
    "llvm"
}

local sanitizers = { "address", "leak", "undefined" }

local cxxflags = {
    release = {},
    debug = {
        "-Wno-unused-function", "-Wno-unused-parameter", "-Wno-unused-variable"
    },
    regular = {
        "-Wall", "-Wextra", "-Werror",
        "-stdlib=libc++",
        "-Wno-c99-designator"
    }
}

local ldflags = {
    release = {},
    debug = {},
    regular = {}
}

set_languages("gnuxxlatest", "gnulatest")

add_rules("mode.debug", "mode.release")

add_requires(packages)

target("kalidescope")
do
    set_kind("binary")
    add_packages(packages)

    add_files("src/**.cpp", "src/**.c")
    add_headerfiles("src/**.hpp")
    --precompile utilities.hpp
    set_pcxxheader("src/utilities.hpp")

    add_includedirs("src/")

    add_cxxflags(cxxflags.regular)
    add_ldflags(ldflags.regular)

    if is_mode("debug") then
        add_cxxflags(cxxflags.debug)
        add_ldflags(ldflags.debug)

        for _, v in ipairs(sanitizers) do
            add_cxflags("-fsanitize=" .. v)
            add_ldflags("-fsanitize=" .. v)
        end

        add_defines("PROJECT_DEBUG")
    elseif is_mode("release") then
        add_cxxflags(cxxflags.release)
        add_ldflags(ldflags.release)
    end
end
target_end()
