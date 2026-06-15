-- okinawa.cpp - xmake build
--
-- Build:        xmake
-- Run tests:    xmake run okinawa_test   (or: xmake test)
-- Coverage:     xmake coverage           (custom task, llvm source-based)
--
-- Third-party dependencies are fetched and built by xrepo (xmake's package
-- manager); there is no separate "install dependencies" step.

set_project("okinawa")
set_version("0.1.0")

set_languages("cxx17")
set_warnings("all")                 -- -Wall
add_cxflags("-Wundef", "-Wmacro-redefined", "-Wextra-semi", {tools = {"clang", "gcc"}})
set_symbols("debug")                -- always emit debug info (-g)

-- Keep compile_commands.json in sync for clangd / tooling.
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".", lsp = "clangd"})

-- Build modes. Debug is the default; `xmake f -m release` for release.
add_rules("mode.debug", "mode.release")
set_defaultmode("debug")

-- Compile-time toggle for the in-engine MCP server. Default: on in debug,
-- off in release. Force either way with `xmake f --mcp=y` / `--mcp=n`.
-- When off, the src/okinawa/mcp sources and their extra dependencies
-- (cpp-httplib, nlohmann_json) are excluded entirely.
option("mcp")
    set_default(is_mode("debug"))
    set_showmenu(true)
    set_description("Compile the in-engine MCP server")
option_end()

-- Third-party dependencies (resolved from xrepo). Must live in the root scope.
add_requires("glm")
add_requires("glfw")
add_requires("stb")
add_requires("opengl")
add_requires("catch2")              -- only used by the test target

-- MCP server dependencies, only fetched when the feature is enabled.
if has_config("mcp") then
    add_requires("cpp-httplib")
    add_requires("nlohmann_json")
end

-- =========================================================================
-- Engine static library
-- =========================================================================
target("okinawa")
    set_kind("static")
    add_files("src/**.cpp")
    -- Headers live under src/okinawa/, so consumers include them with the
    -- "okinawa/" prefix (e.g. #include "okinawa/core/core.hpp") via the
    -- public "src" root. The private "src/okinawa" root lets the engine's
    -- own sources keep their internal, unprefixed includes (e.g.
    -- #include "math/point.hpp").
    add_includedirs("src/okinawa")             -- private: engine-internal includes
    add_includedirs("src", {public = true})    -- public: consumers use okinawa/ prefix
    add_packages("glm", "glfw", "stb", "opengl", {public = true})

    -- In-engine MCP server: compile its sources + deps only when enabled,
    -- otherwise exclude them from the build entirely.
    if has_config("mcp") then
        add_defines("OKINAWA_WITH_MCP")
        add_packages("cpp-httplib", "nlohmann_json")
    else
        remove_files("src/okinawa/mcp/**.cpp")
    end

    -- macOS windowing/runtime frameworks required by GLFW.
    if is_plat("macosx") then
        add_frameworks("Cocoa", "IOKit", "CoreVideo", "OpenGL")
    end

-- =========================================================================
-- Test suite (Catch2)
-- =========================================================================
target("okinawa_test")
    set_kind("binary")
    set_default(false)              -- not built by a bare `xmake`; build explicitly
    add_files("tests/*.cpp")
    add_includedirs("tests")
    add_deps("okinawa")
    add_packages("catch2")
    -- Tests read data files by project-relative paths (e.g. tests/test-file.txt).
    set_rundir("$(projectdir)")
    add_tests("default")

-- =========================================================================
-- Coverage task: llvm source-based coverage over the test run.
--   xmake coverage   ->  HTML report in ./coverage/index.html
-- =========================================================================
task("coverage")
    set_menu({usage = "xmake coverage", description = "Run tests and generate an llvm-cov HTML report"})
    on_run(function ()
        import("core.project.project")
        os.exec("xmake build okinawa_test")

        local profraw  = "build/okinawa.profraw"
        local profdata = "build/okinawa.profdata"
        local testbin  = project.target("okinawa_test"):targetfile()
        local libbin   = project.target("okinawa"):targetfile()

        os.tryrm(profraw)
        os.tryrm(profdata)
        os.mkdir("coverage")

        os.execv(testbin, {"--reporter=console"}, {envs = {LLVM_PROFILE_FILE = profraw}})
        os.exec("llvm-profdata merge -sparse %s -o %s", profraw, profdata)
        os.exec("llvm-cov show %s -instr-profile=%s -format=html " ..
                "-show-line-counts-or-regions -show-instantiations -show-expansions " ..
                "-use-color -output-dir=coverage -ignore-filename-regex=.*tests/.* ",
                libbin, profdata)
        print("Coverage report generated in coverage/index.html")
    end)
