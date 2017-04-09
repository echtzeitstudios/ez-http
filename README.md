# ez-http

```
mkdir build
cd build
conan install .. --build=missing
cmake ..
```

if you experience problems with the clang version detected by cmake and the one used with conan
change the accoring variable in build/conanbuildinfo.cmake
