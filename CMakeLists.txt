# 最低版本
cmake_minimum_required(VERSION 3.10...3.25)
# 项目名称
project(learn_tiny_STL)

# version
set(MyTinySTL_VERSION_MAJOR 2)
set(MyTinySTL_VERSION_MINOR 0)
set(MyTinySTL_VERSION_PATCH 0)
set(MyTinySTL_VERSION "${MyTinySTL_VERSION_MAJOR}.${MyTinySTL_VERSION_MINOR}.${MyTinySTL_VERSION_PATCH}")
message(STATUS "The version of this project is: ${MyTinySTL_VERSION}")

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 构建类型设置（仅影响单配置生成器）
#set(CMAKE_BUILD_TYPE release)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")


# GNU编译器（GCC）设置：
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "5.0.0")
		message(FATAL_ERROR "required GCC 5.0 or later")
	endif()
	# 通用编译选项
    add_compile_options(
        -Wall      # 开启所有警告
        -Wextra    # 开启额外警告
        -Wno-sign-compare  # 禁用对符号的比较警告
        -Wno-unused-but-set-variable  # 禁用未使用但被设置的变量警告
        -Wno-array-bounds  # 禁用数组边界警告
    )

# Clang编译器设置：
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")

	if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "3.5.0")
		message(FATAL_ERROR "required Clang 3.5 or later")
	endif()
	 # 通用编译选项
    add_compile_options(
        -Wall
        -Wextra
        -Wno-sign-compare
    )
endif()

# 构建类型特定选项
add_compile_options(
    "$<$<CONFIG:Debug>:-g -O0>"#Debug：启用调试信息（-g）并关闭优化（-O0）。
    "$<$<CONFIG:Release>:-O2>"#Release：启用优化（-O2）。
    "$<$<CONFIG:RelWithDebInfo>:-O2 -g>"#RelWithDebInfo：启用优化（-O2）和调试信息（-g）。
	"$<$<CONFIG:MinSizeRel>:-Os>"
)

# 打印最终编译标志（需要CMake 3.15+）
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.15)
    message(STATUS "Final CXX flags: $<$<BOOL:${CMAKE_CXX_FLAGS}>:${CMAKE_CXX_FLAGS} >$<JOIN:$<TARGET_PROPERTY:COMPILE_OPTIONS>, >")
else()
    message(STATUS "Final CXX flags: ${CMAKE_CXX_FLAGS}")
endif()

# 包含项目中的Test子目录，该目录应包含测试代码和独立的CMakeLists.txt
add_subdirectory(${PROJECT_SOURCE_DIR}/Test)