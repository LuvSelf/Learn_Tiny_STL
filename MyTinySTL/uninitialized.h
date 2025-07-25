#ifndef MYTINYSTL_UNINITIALIZED_H_
#define MYTINYSTL_UNINITIALIZED_H_

/**
 * @file uninitialized.h
 * @brief 未初始化内存操作工具
 * @note 提供在未初始化内存上构造对象的高效操作
 *
 * 核心功能：
 *   - 复制构造 (uninitialized_copy)
 *   - 区间填充 (uninitialized_fill)
 *   - 移动构造 (uninitialized_move)
 *   - 带数量参数的变体 (_n 后缀)
 *
 * 设计原理：
 *   1. 对平凡类型使用批量操作 (memcpy/move)
 *   2. 对非平凡类型逐个构造 + 异常安全回滚
 *   3. 类型特性分发减少运行时开销
 */

#include "algobase.h"
#include "construct.h"
#include "iterator.h"
#include "type_traits.h"
#include "util.h"

namespace mystl
{
    /*****************************************************************************************/
    // uninitialized_copy
    // 把 [first, last) 上的内容复制到以 result 为起始处的空间，返回复制结束的位置
    /*****************************************************************************************/
    /**
     * @brief true_type 平凡类型，使用 memcpy 优化
     * @note 直接转发给 copy 算法
     */
    template<class InputIter, class ForwardIter>
    ForwardIter unchecked_uninit_copy(InputIter first, InputIter last, ForwardIter result, std::true_type) {
        return mystl::copy(first, last, result);
    }

    /**
     * @brief 非平凡类型实现 (逐个构造 + 异常安全)
     */
    template<class InputIter, class ForwardIter>
    ForwardIter unchecked_uninit_copy(InputIter first, InputIter last, ForwardIter result, std::false_type) {
        auto cur = result;
        try
        {
            for (; first != last; ++first, ++cur) { mystl::construct(&*cur, *first); }
        }
        catch (...)
        {
            for (; result != cur; --cur) mystl::destroy(&*cur);// 析构已构造部分
        }
        return cur;
    }

    /**
     * @brief 把 [first, last) 上的内容复制到以 result 为起始处的空间
     * @return 返回复制结束的位置
     */
    template<class InputIter, class ForwardIter>
    ForwardIter uninitialized_copy(InputIter first, InputIter last, ForwardIter result) {
        // 使用类型特性判断是否平凡可复制
        using value_type = typename iterator_traits<ForwardIter>::value_type;
        return mystl::unchecked_uninit_copy(first, last, result, std::is_trivially_copy_assignable<value_type>{});
    }


    /*****************************************************************************************/
    // uninitialized_copy_n
    // 把 [first, first + n) 上的内容复制到以 result 为起始处的空间，返回复制结束的位置
    /*****************************************************************************************/
    /**
     * @brief true_type 平凡类型
     */
    template<class InputIter, class Size, class ForwardIter>
    ForwardIter unchecked_uninit_copy_n(InputIter first, Size n, ForwardIter result, std::true_type) {
        return mystl::copy_n(first, n, result)
                .second;// copy_n 返回 pair<InputIter, ForwardIter>，取second为目标结束位置
    }

    /**
     * @brief 非平凡类型实现 (逐个构造 + 异常安全)
     */
    template<class InputIter, class Size, class ForwardIter>
    ForwardIter unchecked_uninit_copy_n(InputIter first, Size n, ForwardIter result, std::false_type) {
        auto cur = result;
        try
        {
            for (; n > 0; --n, ++cur, ++first) { mystl::construct(&*cur, *first); }
        }
        catch (...)
        {
            for (; result != cur; --cur) mystl::destroy(&*cur);
        }
        return cur;
    }

    /**
     * @brief 把 [first, first + n) 上的内容复制到以 result 为起始处的空间
     * @return 返回复制结束的位置
     */
    template<class InputIter, class Size, class ForwardIter>
    ForwardIter uninitialized_copy_n(InputIter first, Size n, ForwardIter result) {
        return mystl::unchecked_uninit_copy_n(
                first, n, result, std::is_trivially_copy_assignable<typename iterator_traits<InputIter>::value_type>{});
    }

    /*****************************************************************************************/
    // uninitialized_fill
    // 在 [first, last) 区间内填充元素值
    /*****************************************************************************************/
    /**
     * @brief true_type 平凡类型
     */
    template<class ForwardIter, class T>
    void unchecked_uninit_fill(ForwardIter first, ForwardIter last, const T &value, std::true_type) {
        mystl::fill(first, last, value);
    }

    /**
     * @brief 非平凡类型实现 (逐个构造 + 异常安全)
     */
    template<class ForwardIter, class T>
    void unchecked_uninit_fill(ForwardIter first, ForwardIter last, const T &value, std::false_type) {
        auto cur = first;
        try
        {
            for (; cur != last; ++cur) { mystl::construct(&*cur, value); }
        }
        catch (...)
        {
            for (; first != cur; ++first) mystl::destroy(&*first);
        }
    }

    /**
     * @brief 在 [first, last) 区间内填充元素值
     */
    template<class ForwardIter, class T>
    void uninitialized_fill(ForwardIter first, ForwardIter last, const T &value) {
        mystl::unchecked_uninit_fill(
                first, last, value,
                std::is_trivially_copy_assignable<typename iterator_traits<ForwardIter>::value_type>{});
    }

    /*****************************************************************************************/
    // uninitialized_fill_n
    // 从 first 位置开始，填充 n 个元素值，返回填充结束的位置
    /*****************************************************************************************/
    /**
     * @brief true_type 平凡类型
     */
    template<class ForwardIter, class Size, class T>
    ForwardIter unchecked_uninit_fill_n(ForwardIter first, Size n, const T &value, std::true_type) {
        return mystl::fill_n(first, n, value);
    }

    /**
     * @brief 非平凡类型实现 (逐个构造 + 异常安全)
     */
    template<class ForwardIter, class Size, class T>
    ForwardIter unchecked_uninit_fill_n(ForwardIter first, Size n, const T &value, std::false_type) {
        auto cur = first;
        try
        {
            for (; n > 0; --n, ++cur) { mystl::construct(&*cur, value); }
        }
        catch (...)
        {
            for (; first != cur; ++first) mystl::destroy(&*first);
        }
        return cur;
    }

    /**
     * @brief 从 first 位置开始，填充 n 个元素值，返回填充结束的位置
     */
    template<class ForwardIter, class Size, class T>
    ForwardIter uninitialized_fill_n(ForwardIter first, Size n, const T &value) {
        return mystl::unchecked_uninit_fill_n(
                first, n, value,
                std::is_trivially_copy_assignable<typename iterator_traits<ForwardIter>::value_type>{});
    }

    /*****************************************************************************************/
    // uninitialized_move
    // 把[first, last)上的内容移动到以 result 为起始处的空间，返回移动结束的位置
    /*****************************************************************************************/
    // 平凡类型
    template<class InputIter, class ForwardIter>
    ForwardIter unchecked_uninit_move(InputIter first, InputIter last, ForwardIter result, std::true_type) {
        return mystl::move(first, last, result);
    }

    //非平凡类型
    template<class InputIter, class ForwardIter>
    ForwardIter unchecked_uninit_move(InputIter first, InputIter last, ForwardIter result, std::false_type) {
        ForwardIter cur = result;
        try
        {
            for (; first != last; ++first, ++cur) { mystl::construct(&*cur, mystl::move(*first)); }
        }
        catch (...)
        { mystl::destroy(result, cur); }
        return cur;
    }

    /**
     * @brief 把[first, last)上的内容移动到以 result 为起始处的空间，返回移动结束的位置
     */
    template<class InputIter, class ForwardIter>
    ForwardIter uninitialized_move(InputIter first, InputIter last, ForwardIter result) {
        return mystl::unchecked_uninit_move(
                first, last, result,
                std::is_trivially_move_assignable<typename iterator_traits<InputIter>::value_type>{});
    }

    /*****************************************************************************************/
    // uninitialized_move_n
    // 把[first, first + n)上的内容移动到以 result 为起始处的空间，返回移动结束的位置
    /*****************************************************************************************/
    template<class InputIter, class Size, class ForwardIter>
    ForwardIter unchecked_uninit_move_n(InputIter first, Size n, ForwardIter result, std::true_type) {
        return mystl::move(first, first + n, result);
    }

    template<class InputIter, class Size, class ForwardIter>
    ForwardIter unchecked_uninit_move_n(InputIter first, Size n, ForwardIter result, std::false_type) {
        auto cur = result;
        try
        {
            for (; n > 0; --n, ++first, ++cur) { mystl::construct(&*cur, mystl::move(*first)); }
        }
        catch (...)
        {
            for (; result != cur; ++result) mystl::destroy(&*result);
            throw;
        }
        return cur;
    }
    /**
     * @brief 把[first, first + n)上的内容移动到以 result 为起始处的空间，返回移动结束的位置
     */
    template<class InputIter, class Size, class ForwardIter>
    ForwardIter uninitialized_move_n(InputIter first, Size n, ForwardIter result) {
        return mystl::unchecked_uninit_move_n(
                first, n, result, std::is_trivially_move_assignable<typename iterator_traits<InputIter>::value_type>{});
    }

}// namespace mystl
#endif// !MYTINYSTL_UNINITIALIZED_H_
