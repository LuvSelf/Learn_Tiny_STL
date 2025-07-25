#ifndef MYTINYSTL_ALGOBASE_H_
#define MYTINYSTL_ALGOBASE_H_

/**
 * @file algobase.h
 * @brief mystl 的基本算法
 */

#include <cstring>

#include "iterator.h"
#include "util.h"

namespace mystl
{
    // 在 Windows 的头文件（比如 <windows.h>）中，微软自己定义了两个宏：
    // #define min(a,b) ((a)<(b)?(a):(b))  // 取最小值的宏
    // #define max(a, b) ((a) > (b) ? (a) : (b))// 取最大值的宏
    // 会导致使用c++标准库的std::max时被替换

#ifdef max
#pragma message("#undefing marco max")
#undef max// 删除宏定义
#endif    // max

#ifdef min
#pragma message("#undefing marco min")
#undef min
#endif// min

    // =================================极值操作=================================
    /**
     * @brief 返回两者中的较大值（相等时保证返回第一个参数）
     * @param lhs 左值引用
     * @param rhs 右值引用
     * @note 使用 operator< 比较，语义相等时保证返回第一个参数
     */
    template<class T>
    const T &max(const T &lhs, const T &rhs) {
        return lhs < rhs ? rhs : lhs;
    }

    /**
     * @brief 返回两者中的较大值（使用自定义比较函数）
     */
    template<class T, class Compare>
    const T &max(const T &lhs, const T &rhs, Compare comp) {
        return comp(lhs, rhs) ? rhs : lhs;
    }

    /**
     * @brief 返回两者中的较小值（相等时保证返回第一个参数）
     * @note 使用 operator< 比较
     */
    template<class T>
    const T &min(const T &lhs, const T &rhs) {
        return rhs < lhs ? rhs : lhs;
    }

    /**
     * @brief 返回两者中的较小值（使用自定义比较函数）
     */
    template<class T, class Compare>
    const T &min(const T &lhs, const T &rhs, Compare comp) {
        return comp(rhs, lhs) ? rhs : lhs;
    }

    // ================================= 元素交换 =================================
    /**
     * @brief 交换两个迭代器指向的元素
     * @note 通过 mystl::swap 转发实现
     */
    template<class FIter1, class FIter2>
    void iter_swap(FIter1 lhs, FIter2 rhs) {
        mystl::swap(*lhs, *rhs);
    }

    // ================================= 拷贝操作 =================================

    /**
     * @brief 把 [first, last)区间内的元素拷贝到 [result, result + (last - first))内
     * @note input_iterator_tag 版本
     */
    template<class InputIter, class OutputIter>
    OutputIter unchecked_copy_cat(InputIter first, InputIter last, OutputIter result, mystl::input_iterator_tag) {
        for (; first != last; ++first, ++result) { *result = *first; }
        return result;
    }

    /**
     * @brief 把 [first, last)区间内的元素拷贝到 [result, result + (last - first))内
     * @note ramdom_access_iterator_tag 版本（计算距离优化）
     */
    template<class RandomIter, class OutputIter>
    OutputIter unchecked_copy_cat(RandomIter first, RandomIter last, OutputIter result,
                                  mystl::random_access_iterator_tag) {
        for (auto n = last - first; n > 0; --n, ++first, ++result) { *result = *first; }
        return result;
    }

    /**
     * @brief 把 [first, last)区间内的元素拷贝到 [result, result + (last - first))内
     * @note 类型萃取分发入口
     */
    template<class InputIter, class OutputIter>
    OutputIter unchecked_copy(InputIter first, InputIter last, OutputIter result) {
        return unchecked_copy_cat(first, last, result, iterator_category(first));
    }

    /**
     * @brief 特化：平凡可拷贝类型（trivially copyable types）的特化版本（使用 memmove）
     * @note 避免不必要的拷贝开销
     */
    template<class Tp, class Up>
    typename std::enable_if<std::is_same<typename std::remove_const<Tp>::type, Up>::value &&
                                    std::is_trivially_copy_assignable<Up>::value,
                            Up *>::type
    // 类型去const后相同
    // 可平凡拷贝赋值
    unchecked_copy(Tp *first, Tp *last, Up *result) {
        const auto n = static_cast<size_t>(last - first);
        if (n != 0) std::memmove(result, first, n * sizeof(Up));// 内存级拷贝优化
        return result + n;
    }

    /**
     * @brief 拷贝 [first, last) 到 [result, result + (last-first))
     * @note 自动选择最优实现（迭代器分类/类型特性）
     */
    template<class InputIter, class OutputIter>
    OutputIter copy(InputIter first, InputIter last, OutputIter result) {
        return unchecked_copy(first, last, result);
    }

    // =============================== 反向拷贝操作 ===============================

    /**
     * @brief 将 [first, last)区间内的元素拷贝到 [result - (last - first), result)内
     * @note bidirectional_iterator_tag 版本
     */
    template<class BidirectionalIter1, class BidirectionalIter2>
    BidirectionalIter2 unchecked_copy_backward_cat(BidirectionalIter1 first, BidirectionalIter1 last,
                                                   BidirectionalIter2 result, mystl::bidirectional_iterator_tag) {
        while (first != last) *--result = *--last;
        return result;
    }

    /**
     * @brief 将 [first, last)区间内的元素拷贝到 [result - (last - first), result)内
     * @note random_access_iterator_tag 版本
     */
    template<class RandomIter1, class BidirectionalIter2>
    BidirectionalIter2 unchecked_copy_backward_cat(RandomIter1 first, RandomIter1 last, BidirectionalIter2 result,
                                                   mystl::random_access_iterator_tag) {
        for (auto n = last - first; n > 0; --n) *--result = *--last;
        return result;
    }

    /**
     * @brief 将 [first, last)区间内的元素拷贝到 [result - (last - first), result)内
     * @note 类型萃取分发
     */
    template<class BidirectionalIter1, class BidirectionalIter2>
    BidirectionalIter2 unchecked_copy_backward(BidirectionalIter1 first, BidirectionalIter1 last,
                                               BidirectionalIter2 result) {
        return unchecked_copy_backward_cat(first, last, result, iterator_category(first));
    }

    /**
     * @brief 特化：平凡可拷贝类型的优化
     * @note 内存反向拷贝避免多次赋值
     */
    template<class Tp, class Up>
    typename std::enable_if<std::is_same<typename std::remove_const<Tp>::type, Up>::value &&
                                    std::is_trivially_copy_assignable<Up>::value,
                            Up *>::type
    unchecked_copy_backward(Tp *first, Tp *last, Up *result) {
        const auto n = static_cast<size_t>(last - first);
        if (n != 0)
        {
            result -= n;
            std::memmove(result, first, n * sizeof(Up));// 单次内存操作
        }
        return result;
    }

    /**
     * @brief 将 [first, last)区间内的元素拷贝到 [result - (last - first), result)内
     * @note 自动选择最优实现（迭代器分类/类型特性）
     */
    template<class BidirectionalIter1, class BidirectionalIter2>
    BidirectionalIter2 copy_backward(BidirectionalIter1 first, BidirectionalIter1 last, BidirectionalIter2 result) {
        return unchecked_copy_backward(first, last, result);
    }

    // =============================== 条件拷贝操作 ===============================

    /**
     * @brief 把[first, last)内满足一元操作 unary_pred 的元素拷贝到以 result为起始的位置上
     * @param unary_pred 一元谓词，返回 true 的元素被拷贝
     * @return 目标位置尾后迭代器
     */
    template<class InputIter, class OutputIter, class UnaryPredicate>
    OutputIter copy_if(InputIter first, InputIter last, OutputIter result, UnaryPredicate unary_pred) {
        for (; first != last; ++first)
        {
            if (unary_pred(*first)) *result++ = *first;
        }
        return result;
    }

    // =============================== 指定元素个数拷贝操作 ===============================

    /**
     * @brief 把 [first, first + n)区间上的元素拷贝到 [result, result + n)上
     * @param unary_pred 一元谓词，返回 true 的元素被拷贝
     * @return 返回一个 pair 分别指向拷贝结束的尾部
     * @note InputIter 版本
     */
    template<class InputIter, class Size, class OutputIter>
    mystl::pair<InputIter, OutputIter> unchecked_copy_n(InputIter first, Size n, OutputIter result,
                                                        mystl::input_iterator_tag) {
        for (; n > 0; --n, ++first, ++result) { *result = *first; }
        return mystl::pair<InputIter, OutputIter>(first, result);
    }

    /**
     * @brief 把 [first, first + n)区间上的元素拷贝到 [result, result + n)上
     * @param unary_pred 一元谓词，返回 true 的元素被拷贝
     * @return 返回一个 pair 分别指向拷贝结束的尾部
     * @note RandomIter 版本
     */
    template<class RandomIter, class Size, class OutputIter>
    mystl::pair<RandomIter, OutputIter> unchecked_copy_n(RandomIter first, Size n, OutputIter result,
                                                         mystl::random_access_iterator_tag) {
        auto last = first + n;
        return mystl::pair<RandomIter, OutputIter>(last, mystl::copy(first, last, result));
    }

    /**
     * @brief 把 [first, first + n)区间上的n个元素拷贝到 [result, result + n)上
     * @note 自动选择最优实现（迭代器分类/类型特性）
     */
    template<class InputIter, class Size, class OutputIter>
    mystl::pair<InputIter, OutputIter> copy_n(InputIter first, Size n, OutputIter result) {
        return unchecked_copy_n(first, n, result, iterator_category(first));
    }

    // ================================ 移动操作 =================================

    /**
     * @brief 把 [first, last)区间内的元素移动到 [result, result + (last - first))内
     * @note input_iterator_tag 版本
     */
    template<class InputIter, class OutputIter>
    OutputIter unchecked_move_cat(InputIter first, InputIter last, OutputIter result, mystl::input_iterator_tag) {
        for (; first != last; ++first, ++result) { *result = mystl::move(*first); }
        return result;
    }

    /**
     * @brief 把 [first, last)区间内的元素移动到 [result, result + (last - first))内
     * @note ramdom_access_iterator_tag 版本
     */
    template<class RandomIter, class OutputIter>
    OutputIter unchecked_move_cat(RandomIter first, RandomIter last, OutputIter result,
                                  mystl::random_access_iterator_tag) {
        for (auto n = last - first; n > 0; --n, ++first, ++result) { *result = mystl::move(*first); }
        return result;
    }

    /**
     * @brief 把 [first, first + n)区间上的n个元素拷贝到 [result, result + n)上
     * @note 自动选择最优实现（迭代器分类/类型特性）
     */
    template<class InputIter, class OutputIter>
    OutputIter unchecked_move(InputIter first, InputIter last, OutputIter result) {
        return unchecked_move_cat(first, last, result, iterator_category(first));
    }

    // 平凡可移动类型：那些移动操作可以简单地通过内存拷贝来实现的类型
    /**
     * @brief 平凡可移动类型(Trivially Move Assignable)类型的特化版本
     * @note 使用 memmove 避免多次移动操作
     */
    template<class Tp, class Up>
    typename std::enable_if<std::is_same<typename std::remove_const<Tp>::type, Up>::value &&
                                    std::is_trivially_move_assignable<Up>::value,
                            Up *>::type
    unchecked_move(Tp *first, Tp *last, Up *result) {
        const size_t n = static_cast<size_t>(last - first);
        if (n != 0) std::memmove(result, first, n * sizeof(Up));
        return result + n;
    }

    /**
     * @brief 移动 [first, last) 到 [result, result + (last-first))
     * @note 资源转移而非拷贝，优化性能
     */
    template<class InputIter, class OutputIter>
    OutputIter move(InputIter first, InputIter last, OutputIter result) {
        return unchecked_move(first, last, result);
    }

    // =============================== 反向移动操作 ===============================

    /**
     * @brief 将 [first, last)区间内的元素移动到 [result - (last - first), result)内
     * @note bidirectional_iterator_tag 版本
     */
    template<class BidirectionalIter1, class BidirectionalIter2>
    BidirectionalIter2 unchecked_move_backward_cat(BidirectionalIter1 first, BidirectionalIter1 last,
                                                   BidirectionalIter2 result, mystl::bidirectional_iterator_tag) {
        while (first != last) *--result = mystl::move(*--last);
        return result;
    }

    /**
     * @brief 将 [first, last)区间内的元素移动到 [result - (last - first), result)内
     * @note random_access_iterator_tag 版本
     */
    template<class RandomIter1, class RandomIter2>
    RandomIter2 unchecked_move_backward_cat(RandomIter1 first, RandomIter1 last, RandomIter2 result,
                                            mystl::random_access_iterator_tag) {
        for (auto n = last - first; n > 0; --n) *--result = mystl::move(*--last);
        return result;
    }

    template<class BidirectionalIter1, class BidirectionalIter2>
    BidirectionalIter2 unchecked_move_backward(BidirectionalIter1 first, BidirectionalIter1 last,
                                               BidirectionalIter2 result) {
        return unchecked_move_backward_cat(first, last, result, iterator_category(first));
    }

    /**
     * @brief 平凡可移动类型(Trivially Move Assignable)类型的特化版本
     * @note 使用 memmove 避免多次移动操作
     */
    template<class Tp, class Up>
    typename std::enable_if<std::is_same<typename std::remove_const<Tp>::type, Up>::value &&
                                    std::is_trivially_move_assignable<Up>::value,
                            Up *>::type
    unchecked_move_backward(Tp *first, Tp *last, Up *result) {
        const size_t n = static_cast<size_t>(last - first);
        if (n != 0)
        {
            result -= n;
            std::memmove(result, first, n * sizeof(Up));
        }
        return result;
    }

    /**
     * @brief 将 [first, last)区间内的元素移动到 [result - (last - first), result)内
     * @note 资源转移而非拷贝，优化性能
     */
    template<class BidirectionalIter1, class BidirectionalIter2>
    BidirectionalIter2 move_backward(BidirectionalIter1 first, BidirectionalIter1 last, BidirectionalIter2 result) {
        return unchecked_move_backward(first, last, result);
    }

    // =============================== 比较操作 ===============================

    /**
     * @brief 比较第一序列在 [first, last)区间上的元素值是否和第二序列相等
     * @note operator== 用于元素值比较，返回 true 则相等，否则不等
     */
    template<class InputIter1, class InputIter2>
    bool equal(InputIter1 first1, InputIter1 last1, InputIter2 first2) {
        for (; first1 != last1; ++first1, ++first2)
        {
            if (*first1 != *first2) return false;
        }
        return true;
    }

    /**
     * @brief 比较第一序列在 [first, last)区间上的元素值是否和第二序列相等
     * @note 重载版本使用函数对象 comp 代替比较操作
     */
    template<class InputIter1, class InputIter2, class Compared>
    bool equal(InputIter1 first1, InputIter1 last1, InputIter2 first2, Compared comp) {
        for (; first1 != last1; ++first1, ++first2)
        {
            if (!comp(*first1, *first2)) return false;
        }
        return true;
    }

    // =============================== 填充操作 ===============================

    /**
     * @brief 从 first 位置开始填充 n 个元素值
     * @return 填充结束位置
     */
    template<class OutputIter, class Size, class T>
    OutputIter unchecked_fill_n(OutputIter first, Size n, const T &value) {
        for (; n > 0; --n, ++first) { *first = value; }
        return first;
    }

    /**
     * @brief 特化：单字节类型的优化（使用 memset）
     * @note 避免循环赋值，直接内存操作
     */
    template<class Tp, class Size, class Up>
    typename std::enable_if<std::is_integral<Tp>::value && sizeof(Tp) == 1 && !std::is_same<Tp, bool>::value &&
                                    std::is_integral<Up>::value && sizeof(Up) == 1,
                            Tp *>::type
    unchecked_fill_n(Tp *first, Size n, Up value) {
        if (n > 0) { std::memset(first, (unsigned char) value, (size_t) (n)); }
        return first + n;
    }

    /**
     * @brief 从 first 开始填充 n 个 value
     */
    template<class OutputIter, class Size, class T>
    OutputIter fill_n(OutputIter first, Size n, const T &value) {
        return unchecked_fill_n(first, n, value);
    }

    /**
     * @brief 为 [first, last)区间内的所有元素填充新值
     * @note forward_iterator_tag 版本
     */
    template<class ForwardIter, class T>
    void fill_cat(ForwardIter first, ForwardIter last, const T &value, mystl::forward_iterator_tag) {
        for (; first != last; ++first) { *first = value; }
    }

    /**
     * @brief 为 [first, last)区间内的所有元素填充新值
     * @note random_access_iterator_tag 版本
     */
    template<class RandomIter, class T>
    void fill_cat(RandomIter first, RandomIter last, const T &value, mystl::random_access_iterator_tag) {
        fill_n(first, last - first, value);
    }

    /**
     * @brief 为 [first, last)区间内的所有元素填充新值
     * @note 资源转移而非拷贝，优化性能
     */
    template<class ForwardIter, class T>
    void fill(ForwardIter first, ForwardIter last, const T &value) {
        fill_cat(first, last, value, iterator_category(first));
    }

    // ============================== 字典序比较 ==============================

    /**
     * @brief 字典序比较两个序列
     * @return true 当序列1 < 序列2
     *
     * 比较规则：
     * 1. 发现不等元素：序列1元素小则返回 true
     * 2. 序列1结束而序列2未结束：返回 true
     * 3. 序列2结束而序列1未结束：返回 false
     * 4. 同时结束：返回 false
     */
    template<class InputIter1, class InputIter2>
    bool lexicographical_compare(InputIter1 first1, InputIter1 last1, InputIter2 first2, InputIter2 last2) {
        for (; first1 != last1 && first2 != last2; ++first1, ++first2)
        {
            if (*first1 < *first2) return true;
            if (*first2 < *first1) return false;
        }
        return first1 == last1 && first2 != last2;
    }

    /**
     * @brief 字典序比较（使用自定义比较函数）
     */
    template<class InputIter1, class InputIter2, class Compred>
    bool lexicographical_compare(InputIter1 first1, InputIter1 last1, InputIter2 first2, InputIter2 last2,
                                 Compred comp) {
        for (; first1 != last1 && first2 != last2; ++first1, ++first2)
        {
            if (comp(*first1, *first2)) return true;
            if (comp(*first2, *first1)) return false;
        }
        return first1 == last1 && first2 != last2;
    }

    /**
     * @brief 特化：无符号字符指针的高效比较
     * @note 使用 memcmp 优化字节序列比较
     */
    bool lexicographical_compare(const unsigned char *first1, const unsigned char *last1, const unsigned char *first2,
                                 const unsigned char *last2) {
        const auto len1 = last1 - first1;
        const auto len2 = last2 - first2;
        // 先比较相同长度的部分
        const auto result = std::memcmp(first1, first2, mystl::min(len1, len2));
        // 若相等，长度较长的比较大
        return result != 0 ? result < 0 : len1 < len2;
    }

    /**
     * @brief 平行比较两个序列，找到第一处失配的元素
     * @return 返回一对迭代器，分别指向两个序列中失配的元素
     */
    template<class InputIter1, class InputIter2>
    mystl::pair<InputIter1, InputIter2> mismatch(InputIter1 first1, InputIter1 last1, InputIter2 first2) {
        while (first1 != last1 && *first1 == *first2)
        {
            ++first1;
            ++first2;
        }
        return mystl::pair<InputIter1, InputIter2>(first1, first2);
    }

    /**
     * @brief 平行比较两个序列，找到第一处失配的元素
     * @param comp 自定义比较函数
     * @return 返回一对迭代器，分别指向两个序列中失配的元素
     */
    template<class InputIter1, class InputIter2, class Compred>
    mystl::pair<InputIter1, InputIter2> mismatch(InputIter1 first1, InputIter1 last1, InputIter2 first2, Compred comp) {
        while (first1 != last1 && comp(*first1, *first2))
        {
            ++first1;
            ++first2;
        }
        return mystl::pair<InputIter1, InputIter2>(first1, first2);
    }

}// namespace mystl
#endif// !MYTINYSTL_ALGOBASE_H_
