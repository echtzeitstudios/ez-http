from conans import ConanFile, CMake
from conans.tools import download, unzip, replace_in_file
import os
import shutil

class EzHttpConan(ConanFile):
    name = "ez-http"
    version = "0.0.3"
    url = "https://github.com/0x7f/ez-http"
    license = "https://github.com/0x7f/ez-http/blob/master/LICENSE"
    export = "*"
    settings = "os", "compiler", "build_type", "arch"
    requires = "OpenSSL/1.0.2h@lasote/stable", "Boost/1.60.0@lasote/stable", "http-parser/2.6.0@aptakhin/stable"
    generators = "cmake"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = "shared=False", "fPIC=True"

    def source(self):
        zip_name = "v%s.zip" % self.version
        download("https://github.com/0x7f/ez-http/archive/%s" % zip_name, zip_name)
        unzip(zip_name)
        os.unlink(zip_name)
        shutil("%s-%s/*" % (self.name, self.version), ".")

    def imports(self):
        self.copy("*.dll", dst="bin", src="bin") # From bin to bin
        self.copy("*.dylib*", dst="bin", src="lib") # From lib to bin

    def build(self):
        cmake = CMake(self.settings)
        self.run('cmake "%s" %s' % (self.conanfile_directory, cmake.command_line))
        self.run('cmake --build . %s' % cmake.build_config)

    def package(self):
        self.copy("*.h", dst="src")
        self.copy("*.lib", dst="lib", src="lib")
        self.copy("*.a", dst="lib", src="lib")

    def package_info(self):
        self.cpp_info.libs = ["ez-http"]
