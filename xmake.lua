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

-- Compile-time toggle for the in-engine MCP server. Compiled in by default;
-- pass `xmake f --mcp=n` to exclude it (e.g. for lean release builds), or
-- `--mcp=y` to be explicit. The option carries the OKINAWA_WITH_MCP define
-- and is wired to the engine target with add_options below; mcp-server.cpp is
-- guarded by that define, so when the option is off it compiles to nothing
-- and no MCP/HTTP code (or its header-only deps) lands in the binary.
option("mcp")
    set_default(true)
    set_showmenu(true)
    set_description("Compile the in-engine MCP server (default: on; --mcp=n to exclude)")
    add_defines("OKINAWA_WITH_MCP")
option_end()

-- Third-party dependencies (resolved from xrepo). Must live in the root scope.
add_requires("glm")
add_requires("glfw")
add_requires("stb")
add_requires("opengl")
add_requires("catch2")              -- only used by the test target
add_requires("cpp-httplib")         -- MCP server (header-only)
add_requires("nlohmann_json")       -- MCP server (header-only)

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

    -- In-engine MCP server: add_options applies the "mcp" option's
    -- OKINAWA_WITH_MCP define when it is enabled. mcp-server.cpp is guarded by
    -- that define, so it compiles to nothing when the option is off.
    add_options("mcp")
    add_packages("cpp-httplib", "nlohmann_json")

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
