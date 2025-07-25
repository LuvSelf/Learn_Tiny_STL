﻿#ifndef MYTINYSTL_SET_ALGO_H_
#define MYTINYSTL_SET_ALGO_H_

/**
 * @brief set_algo.h
 * @note 包含 set 的四种算法: union, intersection, difference, symmetric_difference
 * @note 所有函数都要求序列有序
 */

#include "algobase.h"
#include "iterator.h"

namespace mystl
{

    /**
     * @brief 计算 S1∪S2 的结果并保存到 result 中，返回一个迭代器指向输出结果的尾部
     * @note 并集
     * @note 必须有序，且结果也有序
     */
    template<class InputIter1, class InputIter2, class OutputIter>
    OutputIter set_union(InputIter1 first1, InputIter1 last1, InputIter2 first2, InputIter2 last2, OutputIter result) {
        while (first1 != last1 && first2 != last2)
        {
            if (*first1 < *first2)
            {
                *result = *first1;
                ++first1;
            }
            else if (*first2 < *first1)
            {
                *result = *first2;
                ++first2;
            }
            else
            {
                *result = *first1;
                ++first1;
                ++first2;
            }
            ++result;
        }
        // 将剩余元素拷贝到 result
        return mystl::copy(first2, last2, mystl::copy(first1, last1, result));
    }


    /**
     * @brief 计算 S1∪S2 的结果并保存到 result 中，返回一个迭代器指向输出结果的尾部
     * @note 并集
     * @note 必须有序，且结果也有序
     */
    template<class InputIter1, class InputIter2, class OutputIter, class Compared>
    OutputIter set_union(InputIter1 first1, InputIter1 last1, InputIter2 first2, InputIter2 last2, OutputIter result,
                         Compared comp) {
        while (first1 != last1 && first2 != last2)
        {
            if (comp(*first1, *first2))
            {
                *result = *first1;
                ++first1;
            }
            else if (comp(*first2, *first1))
            {
                *result = *first2;
                ++first2;
            }
            else
            {
                *result = *first1;
                ++first1;
                ++first2;
            }
            ++result;
        }
        // 将剩余元素拷贝到 result
        return mystl::copy(first2, last2, mystl::copy(first1, last1, result));
    }

    /**
     * @brief 计算 S1∩S2 的结果并保存到 result 中，返回一个迭代器指向输出结果的尾部
     * @note 交集
     * @note 必须有序，且结果也有序
     */
    template<class InputIter1, class InputIter2, class OutputIter>
    OutputIter set_intersection(InputIter1 first1, InputIter1 last1, InputIter2 first2, InputIter2 last2,
                                OutputIter result) {
        while (first1 != last1 && first2 != last2)
        {
            if (*first1 < *first2) { ++first1; }
            else if (*first2 < *first1) { ++first2; }
            else
            {
                *result = *first1;
                ++first1;
                ++first2;
                ++result;
            }
        }
        return result;
    }

    /**
     * @brief 计算 S1∩S2 的结果并保存到 result 中，返回一个迭代器指向输出结果的尾部
     * @note 交集
     * @note 必须有序，且结果也有序
     */
    template<class InputIter1, class InputIter2, class OutputIter, class Compared>
    OutputIter set_intersection(InputIter1 first1, InputIter1 last1, InputIter2 first2, InputIter2 last2,
                                OutputIter result, Compared comp) {
        while (first1 != last1 && first2 != last2)
        {
            if (comp(*first1, *first2)) { ++first1; }
            else if (comp(*first2, *first1)) { ++first2; }
            else
            {
                *result = *first1;
                ++first1;
                ++first2;
                ++result;
            }
        }
        return result;
    }

    /**
     * @brief 计算 S1-S2 的结果并保存到 result 中，返回一个迭代器指向输出结果的尾部
     * @note 差集
     * @note 必须有序，且结果也有序
     */
    template<class InputIter1, class InputIter2, class OutputIter>
    OutputIter set_difference(InputIter1 first1, InputIter1 last1, InputIter2 first2, InputIter2 last2,
                              OutputIter result) {
        while (first1 != last1 && first2 != last2)
        {
            if (*first1 < *first2)
            {
                *result = *first1;
                ++first1;
                ++result;
            }
            else if (*first2 < *first1) { ++first2; }
            else
            {
                ++first1;
                ++first2;
            }
        }
        return mystl::copy(first1, last1, result);
    }

    /**
     * @brief 计算 S1-S2 的结果并保存到 result 中，返回一个迭代器指向输出结果的尾部
     * @note 差集
     * @note 必须有序，且结果也有序
     */
    template<class InputIter1, class InputIter2, class OutputIter, class Compared>
    OutputIter set_difference(InputIter1 first1, InputIter1 last1, InputIter2 first2, InputIter2 last2,
                              OutputIter result, Compared comp) {
        while (first1 != last1 && first2 != last2)
        {
            if (comp(*first1, *first2))
            {
                *result = *first1;
                ++first1;
                ++result;
            }
            else if (comp(*first2, *first1)) { ++first2; }
            else
            {
                ++first1;
                ++first2;
            }
        }
        return mystl::copy(first1, last1, result);
    }

    /**
     * @brief 计算 (S1-S2)∪(S2-S1) 的结果并保存到 result 中，返回一个迭代器指向输出结果的尾部
     * @note 对称差集
     * @note 必须有序，且结果也有序
     */
    template<class InputIter1, class InputIter2, class OutputIter>
    OutputIter set_symmetric_difference(InputIter1 first1, InputIter1 last1, InputIter2 first2, InputIter2 last2,
                                        OutputIter result) {
        while (first1 != last1 && first2 != last2)
        {
            if (*first1 < *first2)
            {
                *result = *first1;
                ++first1;
                ++result;
            }
            else if (*first2 < *first1)
            {
                *result = *first2;
                ++first2;
                ++result;
            }
            else
            {
                ++first1;
                ++first2;
            }
        }
        return mystl::copy(first2, last2, mystl::copy(first1, last1, result));
    }

    /**
     * @brief 计算 (S1-S2)∪(S2-S1) 的结果并保存到 result 中，返回一个迭代器指向输出结果的尾部
     * @note 对称差集
     * @note 必须有序，且结果也有序
     */
    template<class InputIter1, class InputIter2, class OutputIter, class Compared>
    OutputIter set_symmetric_difference(InputIter1 first1, InputIter1 last1, InputIter2 first2, InputIter2 last2,
                                        OutputIter result, Compared comp) {
        while (first1 != last1 && first2 != last2)
        {
            if (comp(*first1, *first2))
            {
                *result = *first1;
                ++first1;
                ++result;
            }
            else if (comp(*first2, *first1))
            {
                *result = *first2;
                ++first2;
                ++result;
            }
            else
            {
                ++first1;
                ++first2;
            }
        }
        return mystl::copy(first2, last2, mystl::copy(first1, last1, result));
    }

}// namespace mystl
#endif// !MYTINYSTL_SET_ALGO_H_
