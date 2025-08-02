#ifndef MYTINYSTL_UTIL_H_
#define MYTINYSTL_UTIL_H_

/**
 * @brief util.h
 * @note 一些通用工具，包括 move, forward, swap 等函数，以及 pair 等
 */

#include <cstddef>

#include "type_traits.h"

namespace mystl
{

    // move
    // 将传入参数转换为右值引用
    // 通过static_cast将类型转换为去掉引用后的类型的右值引用
    // 使用std::remove_reference来确保不管T是左值引用还是右值引用，都能返回右值引用
    template<class T>
    typename std::remove_reference<T>::type &&move(T &&arg) noexcept {
        return static_cast<typename std::remove_reference<T>::type &&>(arg);
    }

    // forward  通常用于模板函数中，以保持参数的值类别
    // 完美转发，保持参数原有的左值或右值属性
    // 处理左值：当T为左值引用类型时，将参数转发为左值
    template<class T>
    T &&forward(typename std::remove_reference<T>::type &arg) noexcept {
        return static_cast<T &&>(arg);
    }
    // 处理右值：当T为非左值引用类型（右值引用或普通类型）时，将参数转发为右值
    // static_assert确保不会错误地将左值传递给右值引用
    // is_lvalue_reference<T>::value 检测模板参数 T 是否是左值引用类型
    template<class T>
    T &&forward(typename std::remove_reference<T>::type &&arg) noexcept {
        static_assert(!std::is_lvalue_reference<T>::value, "bad forward");
        return static_cast<T &&>(arg);
    }

    // swap
    // 通用swap：交换两个同类型对象
    template<class Tp>
    void swap(Tp &lhs,
              Tp &rhs) noexcept(std::is_nothrow_move_constructible<Tp>::value &&
                                std::is_nothrow_move_assignable<Tp>::value) {
        auto tmp(mystl::move(lhs));
        lhs = mystl::move(rhs);
        rhs = mystl::move(tmp);
    }
    // swap_range：交换两个迭代器范围内的元素
    template<class ForwardIter1, class ForwardIter2>
    ForwardIter2
    swap_range(ForwardIter1 first1, ForwardIter1 last1,
               ForwardIter2 first2) noexcept(noexcept(swap(*first1, *first2))) {
        for (; first1 != last1; ++first1, (void) ++first2)
            mystl::swap(*first1, *first2);
        return first2;
    }
    // 数组swap：交换两个同类型、同大小的数组
    template<class Tp, size_t N>
    void swap(Tp (&a)[N],
              Tp (&b)[N]) noexcept(noexcept(swap_range(a, a + N, b))) {
        mystl::swap_range(a, a + N, b);
    }

    // --------------------------------------------------------------------------------------
    // pair

    // 结构体模板 : pair
    // 两个模板参数分别表示两个数据的类型
    // 用 first 和 second 来分别取出第一个数据和第二个数据
    template<class Ty1, class Ty2>
    struct pair {
        typedef Ty1 first_type;
        typedef Ty2 second_type;

        first_type first;  // 保存第一个数据
        second_type second;// 保存第二个数据

        // 默认构造
        // 编译时验证 Other1 和 Other2 是否有默认构造函数
        template<class Other1 = Ty1, class Other2 = Ty2,
                 typename = typename std::enable_if<
                         std::is_default_constructible<Other1>::value &&
                                 std::is_default_constructible<Other2>::value,
                         void>::type>
        // 编译期初始化
        constexpr pair() : first(), second() {}

        // 隐式构造
        // std::enable_if是一个模板工具，当第一个模板参数（条件）为true时，它有一个公共成员type（定义为int）；如果条件为false，则没有type成员（此时替换失败，该构造函数被从重载集中移除）
        // is_copy_constructible：存在拷贝构造函数
        // is_convertible：从const U1 &到Ty1必须是隐式可转换的
        template<class U1 = Ty1, class U2 = Ty2,
                 typename std::enable_if<
                         std::is_copy_constructible<U1>::value &&
                                 std::is_copy_constructible<U2>::value &&
                                 std::is_convertible<const U1 &, Ty1>::value &&
                                 std::is_convertible<const U2 &, Ty2>::value,
                         int>::type = 0>
        constexpr pair(const Ty1 &a, const Ty2 &b) : first(a), second(b) {}

        // 显式构造
        template<
                class U1 = Ty1, class U2 = Ty2,
                typename std::enable_if<
                        std::is_copy_constructible<U1>::value &&
                                std::is_copy_constructible<U2>::value &&
                                (!std::is_convertible<const U1 &, Ty1>::value ||
                                 !std::is_convertible<const U2 &, Ty2>::value),
                        int>::type = 0>
        explicit constexpr pair(const Ty1 &a, const Ty2 &b)
            : first(a), second(b) {}

        // 拷贝构造 (默认)
        pair(const pair &rhs) = default;
        // 移动构造 (默认)
        pair(pair &&rhs) = default;

        // 其他类型的隐式可构造
        // is_constructible：类型Ty可以从Other类型的对象构造
        template<class Other1, class Other2,
                 typename std::enable_if<
                         std::is_constructible<Ty1, Other1>::value &&
                                 std::is_constructible<Ty2, Other2>::value &&
                                 std::is_convertible<Other1 &&, Ty1>::value &&
                                 std::is_convertible<Other2 &&, Ty2>::value,
                         int>::type = 0>
        //&& 语法声明参数，可根据传入实参类型自动推导为左值引用或右值引用
        constexpr pair(Other1 &&a, Other2 &&b)
            : first(mystl::forward<Other1>(a)),
              second(mystl::forward<Other2>(b)) {}

        // 其他类型的显式可构造
        template<class Other1, class Other2,
                 typename std::enable_if<
                         std::is_constructible<Ty1, Other1>::value &&
                                 std::is_constructible<Ty2, Other2>::value &&
                                 (!std::is_convertible<Other1, Ty1>::value ||
                                  !std::is_convertible<Other2, Ty2>::value),
                         int>::type = 0>
        explicit constexpr pair(Other1 &&a, Other2 &&b)
            : first(mystl::forward<Other1>(a)),
              second(mystl::forward<Other2>(b)) {}

        // 其他 pair 的隐式可构造性
        template<
                class Other1, class Other2,
                typename std::enable_if<
                        std::is_constructible<Ty1, const Other1 &>::value &&
                                std::is_constructible<Ty2,
                                                      const Other2 &>::value &&
                                std::is_convertible<const Other1 &,
                                                    Ty1>::value &&
                                std::is_convertible<const Other2 &, Ty2>::value,
                        int>::type = 0>
        constexpr pair(const pair<Other1, Other2> &other)
            : first(other.first), second(other.second) {}

        // 其他 pair 的显式可构造性
        template<class Other1, class Other2,
                 typename std::enable_if<
                         std::is_constructible<Ty1, const Other1 &>::value &&
                                 std::is_constructible<Ty2,
                                                       const Other2 &>::value &&
                                 (!std::is_convertible<const Other1 &,
                                                       Ty1>::value ||
                                  !std::is_convertible<const Other2 &,
                                                       Ty2>::value),
                         int>::type = 0>
        explicit constexpr pair(const pair<Other1, Other2> &other)
            : first(other.first), second(other.second) {}

        // implicit constructiable for other pair
        template<class Other1, class Other2,
                 typename std::enable_if<
                         std::is_constructible<Ty1, Other1>::value &&
                                 std::is_constructible<Ty2, Other2>::value &&
                                 std::is_convertible<Other1, Ty1>::value &&
                                 std::is_convertible<Other2, Ty2>::value,
                         int>::type = 0>
        constexpr pair(pair<Other1, Other2> &&other)
            : first(mystl::forward<Other1>(other.first)),
              second(mystl::forward<Other2>(other.second)) {}

        // explicit constructiable for other pair
        template<class Other1, class Other2,
                 typename std::enable_if<
                         std::is_constructible<Ty1, Other1>::value &&
                                 std::is_constructible<Ty2, Other2>::value &&
                                 (!std::is_convertible<Other1, Ty1>::value ||
                                  !std::is_convertible<Other2, Ty2>::value),
                         int>::type = 0>
        explicit constexpr pair(pair<Other1, Other2> &&other)
            : first(mystl::forward<Other1>(other.first)),
              second(mystl::forward<Other2>(other.second)) {}

        // copy assign for this pair
        pair &operator=(const pair &rhs) {
            /* if (this != &rhs)
            {
                first = rhs.first;
                second = rhs.second;
            }
            return *this; */
            if (this != &rhs)
            {
                // 先创建副本再交换（强异常保证）
                Ty1 new_first = rhs.first;
                Ty2 new_second = rhs.second;

                // 无异常操作
                first = std::move(new_first);
                second = std::move(new_second);
            }
            return *this;
        }

        // move assign for this pair
        // is_nothrow_move_assignable：在编译期检查类型特性确保Ty1和Ty2类型都支持无异常抛出的移动赋值
        pair &operator=(pair &&rhs)  {
            if (this != &rhs)
            {
                first = mystl::move(rhs.first);
                second = mystl::move(rhs.second);
            }
            return *this;
        }

        // copy assign for other pair
        template<class Other1, class Other2>
        pair &operator=(const pair<Other1, Other2> &other) {
            first = other.first;
            second = other.second;
            return *this;
        }

        // move assign for other pair
        template<class Other1, class Other2>
        pair &operator=(pair<Other1, Other2> &&other) {
            first = mystl::forward<Other1>(other.first);
            second = mystl::forward<Other2>(other.second);
            return *this;
        }

        ~pair() = default;

        void swap(pair &other){
            if (this != &other)
            {
                mystl::swap(first, other.first);
                mystl::swap(second, other.second);
            }
        }
    };

    // 重载比较操作符
    template<class Ty1, class Ty2>
    bool operator==(const pair<Ty1, Ty2> &lhs, const pair<Ty1, Ty2> &rhs) {
        return lhs.first == rhs.first && lhs.second == rhs.second;
    }

    template<class Ty1, class Ty2>
    bool operator<(const pair<Ty1, Ty2> &lhs, const pair<Ty1, Ty2> &rhs) {
        return lhs.first < rhs.first ||
               (lhs.first == rhs.first && lhs.second < rhs.second);
    }

    template<class Ty1, class Ty2>
    bool operator!=(const pair<Ty1, Ty2> &lhs, const pair<Ty1, Ty2> &rhs) {
        return !(lhs == rhs);
    }

    template<class Ty1, class Ty2>
    bool operator>(const pair<Ty1, Ty2> &lhs, const pair<Ty1, Ty2> &rhs) {
        return rhs < lhs;
    }

    template<class Ty1, class Ty2>
    bool operator<=(const pair<Ty1, Ty2> &lhs, const pair<Ty1, Ty2> &rhs) {
        return !(rhs < lhs);
    }

    template<class Ty1, class Ty2>
    bool operator>=(const pair<Ty1, Ty2> &lhs, const pair<Ty1, Ty2> &rhs) {
        return !(lhs < rhs);
    }

    // 重载 mystl 的 swap
    template<class Ty1, class Ty2>
    void swap(pair<Ty1, Ty2> &lhs, pair<Ty1, Ty2> &rhs) {
        lhs.swap(rhs);
    }

    // 全局函数，让两个数据成为一个 pair
    template<class Ty1, class Ty2>
    pair<Ty1, Ty2> make_pair(Ty1 &&first, Ty2 &&second) {
        return pair<Ty1, Ty2>(mystl::forward<Ty1>(first),
                              mystl::forward<Ty2>(second));
    }
}// namespace std
#endif// !MYTINYSTL_UTIL_H_