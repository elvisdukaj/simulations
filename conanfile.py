from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps


class SimulaionRecipe(ConanFile):
    name = "simulations"
    version = "1"
    package_type = "application"

    # Optional metadata
    license = "MIT"
    author = "Elvis Dukaj"
    url = "<Package recipe repository url here, for issues about the package>"
    description = "Simple Phisical simulations"
    topics = ("phisic", "c++", "sdl", "opengl")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = (
        "CMakeLists.txt",
        "pre_01_callbacks/src/*",
        "pre_01_callbacks/CMakeLists.txt",
    )

    def requirements(self):
        self.requires("sdl/3.2.2")
        self.requires("entt/3.14.0")
        self.requires("glm/1.0.1")
        self.requires("box2d/3.0.0")
        self.requires("glew/2.2.0")

    def layout(self):
        cmake_layout(self)

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
