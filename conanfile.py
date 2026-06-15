from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import copy

class OkinawaConan(ConanFile):
    name = "okinawa"
    version = "0.1.0"
    
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}
    
    exports_sources = "CMakeLists.txt", "src/*"
    
    def requirements(self):
        self.requires("glm/0.9.9.8")
        self.requires("glfw/3.4")
        self.requires("stb/cci.20240531")
        self.requires("opengl/system")
        
    def build_requirements(self):
        self.test_requires("catch2/3.8.0")
        
    def layout(self):
        # Editable mode: consumers (wadviewer, heist) read headers and the
        # freshly built static lib straight from this source tree, so we can
        # edit the engine and just rebuild the consumer (no conan create).
        # Headers are exposed under an "okinawa/" prefix via the committed
        # symlink include/okinawa -> ../src, matching the packaged layout
        # (install(DIRECTORY src/ DESTINATION include/okinawa)).
        # Folders are left untouched so the engine's own
        # `conan install . --output-folder=build` + `cmake --preset` flow
        # keeps writing to build/ exactly as before.
        self.cpp.source.includedirs = ["include"]
        self.cpp.build.libdirs = ["build"]

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()
        
    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        
    def package(self):
        cmake = CMake(self)
        cmake.install()
        # Copying additional resources (shader files for the engine)
        copy(self, "*.glsl", src=self.source_folder, dst=self.package_folder, keep_path=True)
        
    def package_info(self):
        self.cpp_info.libs = ["okinawa"]
        self.cpp_info.includedirs = ["include"]
