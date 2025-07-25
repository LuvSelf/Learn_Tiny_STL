#ifndef MYTINYSTL_ITERATOR_H_
#define MYTINYSTL_ITERATOR_H_

// 这个头文件用于迭代器设计，包含了一些模板结构体与全局函数，

#include <cstddef>// 包含ptrdiff_t等定义

#include "type_traits.h"

namespace mystl
{
    // ===================== 迭代器标签设计=====================

    // 迭代器类型标签
    // 输入迭代器（只读，单次遍历）
    struct input_iterator_tag {};
    // 输出迭代器（只写，单次遍历）
    struct output_iterator_tag {};
    // 前向迭代器（读写，多次向前遍历）
    struct forward_iterator_tag : public input_iterator_tag {};
    // 双向迭代器（支持前后移动）
    struct bidirectional_iterator_tag : public forward_iterator_tag {};
    // 随机访问迭代器（支持跳跃访问）
    struct random_access_iterator_tag : public bidirectional_iterator_tag {};

    // ===================== 迭代器基础模板 =====================
    // 自定义迭代器的嵌套类型
    // 用于简化用户自定义迭代器的类型定义
    template<class Category, class T, class Distance = ptrdiff_t, class Pointer = T *, class Reference = T &>
    struct iterator {
        typedef Category iterator_category;// 迭代器类型标签
        typedef T value_type;              // 迭代器指向的元素类型
        typedef Pointer pointer;           // 元素指针类型
        typedef Reference reference;       // 元素引用类型
        typedef Distance difference_type;  // 迭代器距离类型
    };


    // ===================== 迭代器特性萃取模板 =====================

    // ========== 迭代器有效性检测 ==========
    // 检测类型T是否具有iterator_category内嵌类型
    template<class T>
    struct has_iterator_cat {
    private:
        // 作为对比的类型，用于判断返回值的大小
        struct two {
            char a;
            char b;
        };
        // 通用匹配的函数模板，可以接受任意类型的参数
        // 返回一个 two 类型的对象
        template<class U>
        static two test(...);// sizeof = 2
        // 特定匹配函数模板
        // 用于检测类型 U 是否具有 iterator_category 内嵌类型
        // SFINAE (Substitution Failure Is Not An Error) 技术的经典应用
        // 只有当 U 有 iterator_category 内嵌类型时，这个参数声明才合法
        template<class U>
        static char test(typename U::iterator_category * = 0);// sizeof = 1

    public:
        // 用于表示类型 T 是否具有 iterator_category内嵌类型
        static const bool value = sizeof(test<T>(0)) == sizeof(char);
    };


    // ========== 萃取实现 ==========
    // 萃取实现基础模板（空实现）
    template<class Iterator, bool>
    struct iterator_traits_impl {};

    // 特性萃取实现（当条件满足时）
    template<class Iterator>
    struct iterator_traits_impl<Iterator, true> {
        // 将Iterator内部的iterator_category类型重命名为当前作用域下的iterator_category
        typedef typename Iterator::iterator_category iterator_category;
        typedef typename Iterator::value_type value_type;
        typedef typename Iterator::pointer pointer;
        typedef typename Iterator::reference reference;
        typedef typename Iterator::difference_type difference_type;
    };

    // ========== 中间辅助层 ==========
    // 特性萃取辅助模板（空基础）
    template<class Iterator, bool>
    struct iterator_traits_helper {};

    // 特性萃取辅助（当迭代器是五种标准迭代器时）
    template<class Iterator>
    struct iterator_traits_helper<Iterator, true>// 当第二个参数为true时被选择
        : public iterator_traits_impl<
                  Iterator,
                  std::is_convertible<typename Iterator::iterator_category, input_iterator_tag>::value ||
                          std::is_convertible<typename Iterator::iterator_category, output_iterator_tag>::value> {};
    // 判断迭代器的类别是否可以转换为input_iterator_tag或output_iterator_tag。如果可以，则为true。
    // 只要迭代器的类别是五种标准迭代器类别（输入、输出、前向、双向、随机访问）之一，这个布尔表达式就会是true

    // ========== 对外接口 ==========
    // 萃取迭代器的特性
    // 通过 has_iterator_cat 先检查是否是合法迭代器类型
    template<class Iterator>
    struct iterator_traits : public iterator_traits_helper<Iterator, has_iterator_cat<Iterator>::value> {};

    // 针对原生指针的偏特化版本
    template<class T>
    struct iterator_traits<T *> {
        typedef random_access_iterator_tag iterator_category;
        typedef T value_type;
        typedef T *pointer;
        typedef T &reference;
        typedef ptrdiff_t difference_type;
    };

    template<class T>
    struct iterator_traits<const T *> {
        typedef random_access_iterator_tag iterator_category;
        typedef T value_type;
        typedef const T *pointer;
        typedef const T &reference;
        typedef ptrdiff_t difference_type;
    };


    // 萃取某种迭代器
    // 编译期判断迭代器是否属于特定类别
    // 检测类型 T 的迭代器类别是否能转换为目标类别 U
    template<class T, class U, bool = has_iterator_cat<iterator_traits<T>>::value>
    struct has_iterator_cat_of
        : public m_bool_constant<std::is_convertible<typename iterator_traits<T>::iterator_category, U>::value> {};
    // 特化版本：当迭代器无效时直接返回 false
    template<class T, class U>
    struct has_iterator_cat_of<T, U, false> : public m_false_type {};

    // 具体迭代器类型检测
    // 精确输入迭代器检测（排除更高级别）
    template<class Iter>
    struct is_exactly_input_iterator : public m_bool_constant<has_iterator_cat_of<Iter, input_iterator_tag>::value &&
                                                              !has_iterator_cat_of<Iter, forward_iterator_tag>::value> {
    };
    // 层级检测（支持继承关系）
    template<class Iter>
    struct is_input_iterator : public has_iterator_cat_of<Iter, input_iterator_tag> {};

    template<class Iter>
    struct is_output_iterator : public has_iterator_cat_of<Iter, output_iterator_tag> {};

    template<class Iter>
    struct is_forward_iterator : public has_iterator_cat_of<Iter, forward_iterator_tag> {};

    template<class Iter>
    struct is_bidirectional_iterator : public has_iterator_cat_of<Iter, bidirectional_iterator_tag> {};

    template<class Iter>
    struct is_random_access_iterator : public has_iterator_cat_of<Iter, random_access_iterator_tag> {};
    // 泛型迭代器检测
    // 输入或输出迭代器（所有有效迭代器的超集）
    template<class Iterator>
    struct is_iterator
        : public m_bool_constant<is_input_iterator<Iterator>::value || is_output_iterator<Iterator>::value> {};


    // 萃取某个迭代器的 category
    template<class Iterator>
    typename iterator_traits<Iterator>::iterator_category iterator_category(const Iterator &) {
        typedef typename iterator_traits<Iterator>::iterator_category Category;
        return Category();
    }

    // 萃取某个迭代器的 distance_type
    template<class Iterator>
    typename iterator_traits<Iterator>::difference_type *distance_type(const Iterator &) {
        return static_cast<typename iterator_traits<Iterator>::difference_type *>(0);
    }

    // 萃取某个迭代器的 value_type
    template<class Iterator>
    typename iterator_traits<Iterator>::value_type *value_type(const Iterator &) {
        return static_cast<typename iterator_traits<Iterator>::value_type *>(0);
    }

    // ===================== 计算迭代器间的距离 =====================
    // distance 的 input_iterator_tag 的版本
    template<class InputIterator>
    typename iterator_traits<InputIterator>::difference_type distance_dispatch(InputIterator first, InputIterator last,
                                                                               input_iterator_tag) {
        typename iterator_traits<InputIterator>::difference_type n = 0;
        while (first != last)
        {
            ++first;
            ++n;
        }
        return n;
    }

    // distance 的 random_access_iterator_tag 的版本
    template<class RandomIter>
    typename iterator_traits<RandomIter>::difference_type distance_dispatch(RandomIter first, RandomIter last,
                                                                            random_access_iterator_tag) {
        return last - first;
    }

    // 计算迭代器距离
    template<class InputIterator>
    typename iterator_traits<InputIterator>::difference_type distance(InputIterator first, InputIterator last) {
        return distance_dispatch(first, last, iterator_category(first));
    }


    // ===================== 让迭代器前进 n 个距离=====================
    // advance 的 input_iterator_tag 的版本
    template<class InputIterator, class Distance>
    void advance_dispatch(InputIterator &i, Distance n, input_iterator_tag) {
        while (n--) ++i;
    }

    // advance 的 bidirectional_iterator_tag 的版本
    template<class BidirectionalIterator, class Distance>
    void advance_dispatch(BidirectionalIterator &i, Distance n, bidirectional_iterator_tag) {
        if (n >= 0)
            while (n--) ++i;
        else
            while (n++) --i;
    }

    // advance 的 random_access_iterator_tag 的版本
    template<class RandomIter, class Distance>
    void advance_dispatch(RandomIter &i, Distance n, random_access_iterator_tag) {
        i += n;
    }

    template<class InputIterator, class Distance>
    void advance(InputIterator &i, Distance n) {
        advance_dispatch(i, n, iterator_category(i));
    }

    /******************************** 反向迭代器 **********************************/

    // 模板类 : reverse_iterator
    // 代表反向迭代器，使前进为后退，后退为前进
    template<class Iterator>
    class reverse_iterator
    {
    private:
        Iterator current;// 记录对应的正向迭代器

    public:
        // 反向迭代器的五种相应型别
        typedef typename iterator_traits<Iterator>::iterator_category iterator_category;
        typedef typename iterator_traits<Iterator>::value_type value_type;
        typedef typename iterator_traits<Iterator>::difference_type difference_type;
        typedef typename iterator_traits<Iterator>::pointer pointer;
        typedef typename iterator_traits<Iterator>::reference reference;

        typedef Iterator iterator_type;
        typedef reverse_iterator<Iterator> self;

    public:
        // 构造函数
        reverse_iterator() {}
        explicit reverse_iterator(iterator_type i) : current(i) {}
        reverse_iterator(const self &rhs) : current(rhs.current) {}

    public:
        // 取出对应的正向迭代器
        iterator_type base() const { return current; }

        // 重载操作符
        reference operator*() const {// 实际对应正向迭代器的前一个位置
            auto tmp = current;
            return *--tmp;
        }
        pointer operator->() const { return &(operator*()); }

        // 前进(++)变为后退(--)
        self &operator++() {
            --current;
            return *this;
        }
        self operator++(int) {
            self tmp = *this;
            --current;
            return tmp;
        }
        // 后退(--)变为前进(++)
        self &operator--() {
            ++current;
            return *this;
        }
        self operator--(int) {
            self tmp = *this;
            ++current;
            return tmp;
        }

        self &operator+=(difference_type n) {
            current -= n;
            return *this;
        }
        self operator+(difference_type n) const { return self(current - n); }
        self &operator-=(difference_type n) {
            current += n;
            return *this;
        }
        self operator-(difference_type n) const { return self(current + n); }

        reference operator[](difference_type n) const { return *(*this + n); }
    };

    // 重载 operator-
    template<class Iterator>
    typename reverse_iterator<Iterator>::difference_type operator-(const reverse_iterator<Iterator> &lhs,
                                                                   const reverse_iterator<Iterator> &rhs) {
        return rhs.base() - lhs.base();
    }

    // 重载比较操作符
    template<class Iterator>
    bool operator==(const reverse_iterator<Iterator> &lhs, const reverse_iterator<Iterator> &rhs) {
        return lhs.base() == rhs.base();
    }

    template<class Iterator>
    bool operator<(const reverse_iterator<Iterator> &lhs, const reverse_iterator<Iterator> &rhs) {
        return rhs.base() < lhs.base();
    }

    template<class Iterator>
    bool operator!=(const reverse_iterator<Iterator> &lhs, const reverse_iterator<Iterator> &rhs) {
        return !(lhs == rhs);
    }

    template<class Iterator>
    bool operator>(const reverse_iterator<Iterator> &lhs, const reverse_iterator<Iterator> &rhs) {
        return rhs < lhs;
    }

    template<class Iterator>
    bool operator<=(const reverse_iterator<Iterator> &lhs, const reverse_iterator<Iterator> &rhs) {
        return !(rhs < lhs);
    }

    template<class Iterator>
    bool operator>=(const reverse_iterator<Iterator> &lhs, const reverse_iterator<Iterator> &rhs) {
        return !(lhs < rhs);
    }

}// namespace mystl

#endif// !MYTINYSTL_ITERATOR_H_
