cmake_minimum_required(VERSION 3.22.1)
project(ReactNativeMdict)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 20)
set(BUILD_SHARED_LIBS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


# Compile sources
add_library(
        react-native-mdict
        SHARED
        ../cpp/Promise.cpp
        ../cpp/JSMdict.cpp
        ../cpp/NativeMdictModule.cpp
        ../cpp/CancellableThreadPool.cpp
)

# Add headers search paths
target_include_directories(react-native-mdict PUBLIC ../cpp)

# Add mdict dependency
add_library(mdict STATIC IMPORTED)
set_target_properties(mdict PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/libs/${ANDROID_ABI}/libmdict_core.a)

# Add android/log dependency
find_library(log-lib log)

target_link_libraries(
        react-native-mdict
        mdict                        # <-- mdict
        ${log-lib}                  # <-- Logcat logger
        android                     # <-- Android JNI core
        react_codegen_MdictSpec    # <-- Generated Specs from CodeGen
)
