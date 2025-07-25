#ifndef MYTINYSTL_ALLOCATOR_H_
#define MYTINYSTL_ALLOCATOR_H_

/**
 * @file allocator.h
 * @brief 内存分配器模板类
 * @note 分离内存管理与对象生命周期控制，为容器提供统一的内存接口
 *
 * 核心功能：
 *   1. 内存分配 (allocate)
 *   2. 内存释放 (deallocate)
 *   3. 对象构造 (construct)
 *   4. 对象析构 (destroy)
 *
 * 设计原则：
 *   - 符合 STL 分配器接口标准 [[18][37]]
 *   - 零运行时开销（所有成员静态）
 *   - 类型安全模板化
 */

#include "construct.h"
#include "util.h"

// allocator的实现分为两部分：①提供STL标准化的类型接口 ②提供4组功能函数接口

namespace mystl
{

    /**
     * @class allocator
     * @brief 通用内存分配器模板类
     *
     * @tparam T 分配对象类型
     *
     * STL 标准要求接口：
     *   value_type, pointer, const_pointer,
     *   reference, const_reference,
     *   size_type, difference_type
     *
     * @example
     *   mystl::allocator<int> alloc;
     *   int* p = alloc.allocate(5);     // 分配5个int的内存
     *   alloc.construct(p, 42);         // 在p位置构造int(42)
     *   alloc.destroy(p);               // 析构对象
     *   alloc.deallocate(p, 5);         // 释放内存
     */
    template<class T>
    class allocator
    {
        // ======================= STL 标准类型定义 ======================
    public:
        typedef T value_type;             // 对象类型
        typedef T *pointer;               // 对象指针
        typedef const T *const_pointer;   // 常量对象指针
        typedef T &reference;             // 对象引用
        typedef const T &const_reference; // 常量对象引用
        typedef size_t size_type;         // 大小类型（无符号）
        typedef ptrdiff_t difference_type;// 指针差异类型（有符号）

        // ======================= 内存管理接口 =======================
    public:
        static T *allocate();
        static T *allocate(size_type n);

        static void deallocate(T *ptr);
        static void deallocate(T *ptr, size_type n);

        // ======================= 对象生命周期管理 =======================
        static void construct(T *ptr);
        static void construct(T *ptr, const T &value);
        static void construct(T *ptr, T &&value);

        template<class... Args>
        static void construct(T *ptr, Args &&...args);

        static void destroy(T *ptr);
        static void destroy(T *first, T *last);
    };

    /**
     * @brief 分配单个对象的内存
     * @return 指向未初始化内存的指针
     *
     * @note 等价于 operator new(sizeof(T))
     */
    template<class T>
    T *allocator<T>::allocate() {
        return static_cast<T *>(::operator new(sizeof(T)));
    }

    /**
     * @brief 分配多个对象的内存
     * @param n 对象数量
     * @return 指向未初始化内存块的指针
     *
     * @note 等价于 operator new(n * sizeof(T))
     *       特殊情况：n=0 返回 nullptr (符合标准要求)
     */
    template<class T>
    T *allocator<T>::allocate(size_type n) {
        if (n == 0) return nullptr;
        return static_cast<T *>(::operator new(n * sizeof(T)));
    }

    /**
     * @brief 释放单个对象的内存
     * @param ptr 待释放内存指针
     *
     * @note 等价于 operator delete(ptr)
     *       安全检查：空指针安全
     */
    template<class T>
    void allocator<T>::deallocate(T *ptr) {
        if (ptr == nullptr) return;
        ::operator delete(ptr);
    }

    /**
     * @brief 释放多个对象的内存
     * @param ptr 待释放内存起始指针
     * @param n   对象数量 (被忽略，标准化设计)
     *
     * @note 实际实现忽略 n 参数（符合常见 STL 实现习惯）
     */
    template<class T>
    void allocator<T>::deallocate(T *ptr, size_type /*size*/) {
        if (ptr == nullptr) return;
        ::operator delete(ptr);
    }

    /**
     * @brief 在指定位置构造对象（无参构造）
     * @param ptr 已分配的内存地址
     */
    template<class T>
    void allocator<T>::construct(T *ptr) {
        mystl::construct(ptr);
    }
    
    /**
     * @brief 在指定位置构造对象（拷贝构造）
     * @param ptr   内存地址
     * @param value 拷贝来源
     */
    template<class T>
    void allocator<T>::construct(T *ptr, const T &value) {
        mystl::construct(ptr, value);
    }

    /**
     * @brief 在指定位置构造对象（移动构造）
     * @param ptr   内存地址
     * @param value 移动来源
     */
    template<class T>
    void allocator<T>::construct(T *ptr, T &&value) {
        mystl::construct(ptr, mystl::move(value));
    }

    /**
     * @brief 在指定位置构造对象（完美转发）
     * @tparam Args 参数类型
     * @param ptr  内存地址
     * @param args 构造参数包
     */
    template<class T>
    template<class... Args>
    void allocator<T>::construct(T *ptr, Args &&...args) {
        mystl::construct(ptr, mystl::forward<Args>(args)...);
    }

    /**
     * @brief 析构单个对象
     * @param ptr 对象指针
     */
    template<class T>
    void allocator<T>::destroy(T *ptr) {
        mystl::destroy(ptr);
    }

    /**
     * @brief 析构 [first, last) 范围内的对象
     * @param first 起始位置
     * @param last  结束位置（尾后）
     */
    template<class T>
    void allocator<T>::destroy(T *first, T *last) {
        mystl::destroy(first, last);
    }

}// namespace mystl
#endif// !MYTINYSTL_ALLOCATOR_H_
