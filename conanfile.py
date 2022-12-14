from conan import ConanFile
from conan.tools.cmake import CMake
from conan.tools import microsoft, apple


class AdventOfCodeProjectConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "fPIC": [True, False],
        "shared": [True, False],
    }
    default_options = {
        "fPIC": True,
        "shared": False
    }
    generators = "CMakeToolchain", "CMakeDeps"

    def layout(self):
        self.folders.generators = "conan"
        self.folders.imports = "bin"

    def imports(self):
        if microsoft.is_msvc(self):
            self.copy("*.dll", src="bin")
        if apple.is_apple_os(self):
            self.copy("*.dylib", src="lib")

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def requirements(self):
        self.requires("fmt/9.1.0")
        self.requires("range-v3/0.12.0")
        self.requires("opencl-clhpp/2022.09.30")
        self.requires("spdlog/1.11.0")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
