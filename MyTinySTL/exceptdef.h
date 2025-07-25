#pragma once

#include <stdexcept>
// 定义了一组标准异常类，用于处理运行时错误。
// 与断言不同，异常处理允许程序在错误发生时继续执行，而不是立即终止。
// 通过使用异常处理机制，程序员可以定义错误处理的代码块（catch块），以便在发生特定类型的错误时执行相应的操作。
// tdexcept头文件中的异常类都是std::exception类的派生类,都有一些共同接口，如what(),获取有关异常的描述性字符串

#include <cassert>
// 主要提供了断言（assertions）的功能
// 断言是一种在调试过程中使用的机制，允许在代码中插入一些检查点，以确保程序在运行时满足某些条件
// 如果某个断言失败（即条件不满足），程序将立即终止，并可能输出一条错误消息。

namespace mystl
{

#ifndef NDEBUG
#define MYSTL_DEBUG(expr) assert(expr)

#define THROW_LENGTH_ERROR_IF(expr, what) \
    if ((expr)) throw std::length_error(what)

#define THROW_OUT_OF_RANGE_IF(expr, what) \
    if ((expr)) throw std::out_of_range(what)

#define THROW_RUNTIME_ERROR_IF(expr, what) \
    if ((expr)) throw std::runtime_error(what)

}// namespace mystl

#endif// !MYTINYSTL_EXCEPTDEF_H_
