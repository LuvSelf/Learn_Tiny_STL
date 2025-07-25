#ifndef MYTINYSTL_TYPE_TRAITS_H_
#define MYTINYSTL_TYPE_TRAITS_H_

// 这个头文件用于在编译期提取和操作类型信息

// 使用标准type_traits头文件，类型萃取
#include <type_traits>

namespace mystl
{
    // 基础类型包装器

    // 基础模板：包装任意类型的常量值
    template<class T, T v>
    struct m_integral_constant
    {
        // 存储编译期常量值
        static constexpr T value = v;
    };

    // 创建专门的布尔常量包装
    template<bool b>
    using m_bool_constant = m_integral_constant<bool, b>;

    typedef m_bool_constant<true> m_true_type;  // 真标记
    typedef m_bool_constant<false> m_false_type;// 假标记

    /*****************************************************************************************/
    // type traits

    // is_pair

    // 类型检查，判断一个类型是不是pair
    //  --- forward declaration begin
    template<class T1, class T2>
    struct pair;
    // --- forward declaration end

    // 基础模板（默认 false）
    template<class T>
    struct is_pair : mystl::m_false_type
    {
    };

    // 特化版本（对 pair 返回 true）
    template<class T1, class T2>
    struct is_pair<mystl::pair<T1, T2>> : mystl::m_true_type
    {
    };

}// namespace mystl
// !MYTINYSTL_TYPE_TRAITS_H_
#endif