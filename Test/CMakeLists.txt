include_directories(${PROJECT_SOURCE_DIR}/MyTinySTL)
set(APP_SRC test.cpp)
# 设置可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# 创建可执行文件
add_executable(stltest ${APP_SRC})