#ifndef MYTINYSTL_MEMORY_H_
#define MYTINYSTL_MEMORY_H_

/**
 * @file memory.h
 * @brief 高级内存管理工具
 * @note 包含地址获取、临时缓冲区管理、auto_ptr智能指针
 *
 * 核心功能：
 *   1. address_of - 获取对象真实地址（避免 operator& 重载影响）
 *   2. 临时缓冲区获取/释放 (get_temporary_buffer/release_temporary_buffer)
 *   3. temporary_buffer 类 - 带析构安全性的临时内存管理
 *   4. auto_ptr - 严格所有权的智能指针（C++98风格，已弃用）
 */

#include <cstddef>
#include <cstdlib>
#include <climits>

#include "algobase.h"
#include "allocator.h"
#include "construct.h"
#include "uninitialized.h"

namespace mystl
{

    /**
     * @brief 获取对象的真实内存地址
     * @tparam Tp 对象类型
     * @param value 对象引用
     * @return 指向对象的真实指针
     *
     * @note 规避用户自定义 operator& 的重载影响
     *
     * @example
     *   重载取地址运算符
     *   struct Custom { int* operator&() { return nullptr; } };
     *   Custom obj;
     *   auto p = address_of(obj); // 获取真实地址而非nullptr
     */
    template<class Tp>
    constexpr Tp *address_of(Tp &value) noexcept {
        return &value;
    }

    /**
     * @brief 辅助函数：尝试获取指定大小的缓冲区
     * @param len 请求的元素数量
     * @param (T*) 类型推断标签
     * @return pair<指针, 实际获得的元素数量>
     *
     * @note 内存不足时会尝试减半请求大小，直到成功或为0
     */
    template<class T>
    pair<T *, ptrdiff_t> get_buffer_helper(ptrdiff_t len, T *) {
        // 处理超大请求（避免整数溢出）
        if (len > static_cast<ptrdiff_t>(INT_MAX / sizeof(T))) len = INT_MAX / sizeof(T);
        // 尝试分配内存（可能减半多次）
        while (len > 0)
        {
            T *tmp = static_cast<T *>(malloc(static_cast<size_t>(len) * sizeof(T)));
            if (tmp) return pair<T *, ptrdiff_t>(tmp, len);
            len /= 2;// 申请失败时减少 len 的大小
        }
        return pair<T *, ptrdiff_t>(nullptr, 0);
    }

    /**
     * @brief 获取临时缓冲区（直接指定类型）
     * @param len 请求的元素数量
     * @return pair<缓冲区指针, 实际获得的元素数量>
     *
     * @example
     *   auto buf = get_temporary_buffer<int>(100);
     *   if (buf.second > 0) {
     *       // 使用 buf.first
     *   }
     */
    template<class T>
    pair<T *, ptrdiff_t> get_temporary_buffer(ptrdiff_t len) {
        return get_buffer_helper(len, static_cast<T *>(0));
    }
    // 通过指针类型推导
    template<class T>
    pair<T *, ptrdiff_t> get_temporary_buffer(ptrdiff_t len, T *) {
        return get_buffer_helper(len, static_cast<T *>(0));
    }

    /**
     * @brief 释放临时缓冲区
     * @param ptr 缓冲区指针
     */
    template<class T>
    void release_temporary_buffer(T *ptr) {
        free(ptr);
    }

    // --------------------------------------------------------------------------------------
    /**
     * @class temporary_buffer
     * @brief 安全的临时内存缓冲区管理
     *
     * @tparam ForwardIterator 迭代器类型（用于推断元素类型）
     * @tparam T 元素类型
     *
     * 特性：
     *   - 自动管理内存生命周期
     *   - 对非平凡类型正确构造和析构
     *   - 内存不足时自动减少请求大小
     *
     * 禁止复制（符合临时缓冲区独占语义）
     */
    template<class ForwardIterator, class T>
    class temporary_buffer
    {
    private:
        ptrdiff_t original_len;// 缓冲区申请的大小
        ptrdiff_t len;         // 缓冲区实际的大小
        T *buffer;             // 指向缓冲区的指针

    public:
        // 构造、析构函数
        temporary_buffer(ForwardIterator first, ForwardIterator last);

        /**
         * @brief 析构函数
         *   - 析构已构造的元素
         *   - 释放内存
         */
        ~temporary_buffer() {
            mystl::destroy(buffer, buffer + len);
            free(buffer);
        }

    public:
        ptrdiff_t size() const noexcept { return len; }
        ptrdiff_t requested_size() const noexcept { return original_len; }
        T *begin() noexcept { return buffer; }
        T *end() noexcept { return buffer + len; }

    private:
        /**
         * @brief 内存分配逻辑
         *   - 处理超大请求
         *   - 循环尝试分配（可能减半请求大小）
         */
        void allocate_buffer();
        /**
         * @brief 初始化缓冲区
         * @param value 初始化值
         * @param true_type 平凡类型特化（无操作）
         */
        void initialize_buffer(const T &, std::true_type) {}
        /**
         * @brief 初始化缓冲区
         * @param value 初始化值
         * @param false_type 非平凡类型实现
         */
        void initialize_buffer(const T &value, std::false_type) { mystl::uninitialized_fill_n(buffer, len, value); }

    private:
        temporary_buffer(const temporary_buffer &);
        void operator=(const temporary_buffer &);
    };

    /**
     * @brief 构造函数
     * @param first, last 定义缓冲区大小的区间
     *
     * @note 1. 计算区间长度
     *       2. 分配内存
     *       3. 构造元素（若非平凡类型）
     */
    template<class ForwardIterator, class T>
    temporary_buffer<ForwardIterator, T>::temporary_buffer(ForwardIterator first, ForwardIterator last) {
        try
        {
            // 1. 计算区间长度
            len = mystl::distance(first, last);
            // 2. 分配内存
            allocate_buffer();
            // 3. 初始化元素（若非平凡类型）
            // 类型特性分发：平凡类型跳过初始化
            if (len > 0) { initialize_buffer(*first, std::is_trivially_default_constructible<T>()); }
        }
        catch (...)
        {
            free(buffer);
            buffer = nullptr;
            len = 0;
        }
    }

    /**
     * @brief 内存分配逻辑
     *   - 处理超大请求
     *   - 循环尝试分配（可能减半请求大小）
     */
    template<class ForwardIterator, class T>
    void temporary_buffer<ForwardIterator, T>::allocate_buffer() {
        original_len = len;
        // 处理超大请求（避免整数溢出）
        if (len > static_cast<ptrdiff_t>(INT_MAX / sizeof(T))) len = INT_MAX / sizeof(T);
        // 尝试分配（可能减半多次）
        while (len > 0)
        {
            buffer = static_cast<T *>(malloc(len * sizeof(T)));
            if (buffer) break;
            len /= 2;// 申请失败时减少申请空间大小
        }
    }

    // --------------------------------------------------------------------------------------
    /**
     * @class auto_ptr
     * @brief C++98风格的智能指针（严格所有权语义）
     *
     * @note 特性：
     *   - 独占所有权（复制操作转移所有权）
     *   - 自动释放管理的内存
     *   - 已被C++11的unique_ptr取代
     *
     * 使用场景：临时兼容旧代码，不推荐在新代码中使用
     */
    template<class T>
    class auto_ptr
    {
    public:
        typedef T elem_type;

    private:
        T *m_ptr;// 实际指针

    public:
        /**
         * @brief 构造函数
         * @param p 原始指针（可选）
         */
        explicit auto_ptr(T *p = nullptr) : m_ptr(p) {}
        /**
         * @brief 复制构造函数（转移所有权）
         * @param rhs 源 auto_ptr
         *
         * @note 源对象释放所有权（变为nullptr）
         */
        auto_ptr(auto_ptr &rhs) : m_ptr(rhs.release()) {}
        /**
         * @brief 模板复制构造（支持派生类到基类的转换）
         */
        template<class U>
        auto_ptr(auto_ptr<U> &rhs) : m_ptr(rhs.release()) {}

        /**
         * @brief 赋值操作（转移所有权）
         * @note 先释放当前资源，再接管新资源
         */
        auto_ptr &operator=(auto_ptr &rhs) {
            if (this != &rhs)
            {
                delete m_ptr;
                m_ptr = rhs.release();
            }
            return *this;
        }

        template<class U>
        auto_ptr &operator=(auto_ptr<U> &rhs) {
            if (this->get() != rhs.get())
            {
                delete m_ptr;
                m_ptr = rhs.release();
            }
            return *this;
        }

        /**
         * @brief 析构函数
         *   - 释放管理的内存
         */
        ~auto_ptr() { delete m_ptr; }

    public:
        // 重载 operator* 和 operator->
        T &operator*() const { return *m_ptr; }
        T *operator->() const { return m_ptr; }

        // 获得指针
        T *get() const { return m_ptr; }

        // 释放指针
        T *release() {
            T *tmp = m_ptr;
            m_ptr = nullptr;
            return tmp;
        }

        // 重置指针
        void reset(T *p = nullptr) {
            if (m_ptr != p)
            {
                delete m_ptr;
                m_ptr = p;
            }
        }
    };

}// namespace mystl
#endif// !MYTINYSTL_MEMORY_H_
