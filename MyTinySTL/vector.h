#ifndef MYTINYSTL_VECTOR_H_
#define MYTINYSTL_VECTOR_H_

// 这个头文件包含一个模板类 vector
// vector : 向量
/**
 * @brief vector.h
 * @note vector 类模板
 * @note 异常保证：
 *   * mystl::vecotr<T> 满足基本异常保证，部分函数无异常保证，并对以下函数做强异常安全保证：
 *     * emplace
 *     * emplace_back
 *     * push_back
 *   * 当 std::is_nothrow_move_assignable<T>::value == true 时，以下函数也满足强异常保证：
 *     * reserve
 *     * resize
 *     * insert
 */

#include <initializer_list>

#include "iterator.h"
#include "memory_L.h"
#include "util.h"
#include "exceptdef.h"
#include "algo.h"

namespace mystl
{

#ifdef max
#pragma message("#undefing marco max")
#undef max
#endif// max

#ifdef min
#pragma message("#undefing marco min")
#undef min
#endif// min

    /**
     * @brief vector 类模板
     * @tparam T 元素类型
     */
    template<class T>
    class vector
    {
        // 静态断言，不支持 bool 类型的特化
        static_assert(!std::is_same<bool, T>::value, "vector<bool> is abandoned in mystl");

    public:
        // vector 的嵌套型别定义
        typedef mystl::allocator<T> allocator_type;
        typedef mystl::allocator<T> data_allocator;

        // 标准容器类型别名
        typedef typename allocator_type::value_type value_type;
        typedef typename allocator_type::pointer pointer;
        typedef typename allocator_type::const_pointer const_pointer;
        typedef typename allocator_type::reference reference;
        typedef typename allocator_type::const_reference const_reference;
        typedef typename allocator_type::size_type size_type;
        typedef typename allocator_type::difference_type difference_type;

        // 迭代器类型
        typedef value_type *iterator;
        typedef const value_type *const_iterator;
        typedef mystl::reverse_iterator<iterator> reverse_iterator;
        typedef mystl::reverse_iterator<const_iterator> const_reverse_iterator;

        allocator_type get_allocator() { return data_allocator(); }

    private:
        // 表示目前使用空间的头部
        iterator begin_;
        // 表示目前使用空间的尾部
        iterator end_;
        // 表示目前储存空间的尾部
        iterator cap_;

    public:
        /**
         * @brief 默认构造
         */
        vector() noexcept { try_init(); }

        /**
         * @brief 自定义大小默认构造
         * @param n 元素个数
         */
        explicit vector(size_type n) { fill_init(n, value_type()); }
        /**
         * @brief 自定义大小和默认值构造
         * @param n 元素个数
         * @param value 元素值
         */
        vector(size_type n, const value_type &value) { fill_init(n, value); }
        /**
         * @brief 范围构造
         */
        template<class Iter, typename std::enable_if<mystl::is_input_iterator<Iter>::value, int>::type = 0>
        vector(Iter first, Iter last) {
            MYSTL_DEBUG(!(last < first));
            range_init(first, last);
        }
        /**
         * @brief 拷贝构造
         */
        vector(const vector &rhs) { range_init(rhs.begin_, rhs.end_); }
        /**
         * @brief 移动构造
         */
        vector(vector &&rhs) noexcept : begin_(rhs.begin_), end_(rhs.end_), cap_(rhs.cap_) {
            rhs.begin_ = nullptr;
            rhs.end_ = nullptr;
            rhs.cap_ = nullptr;
        }
        /**
         * @brief 范围构造
         * @note std::initializer_list 允许使用花括号初始化列表
         */
        vector(std::initializer_list<value_type> ilist) { range_init(ilist.begin(), ilist.end()); }

        vector &operator=(const vector &rhs);
        vector &operator=(vector &&rhs) noexcept;

        // 初始化列表赋值运算符
        vector &operator=(std::initializer_list<value_type> ilist) {
            vector tmp(ilist.begin(), ilist.end());
            swap(tmp);
            return *this;
        }

        ~vector() {
            destroy_and_recover(begin_, end_, cap_ - begin_);
            begin_ = end_ = cap_ = nullptr;
        }

    public:
        // 迭代器相关操作
        iterator begin() noexcept { return begin_; }
        const_iterator begin() const noexcept { return begin_; }
        iterator end() noexcept { return end_; }
        const_iterator end() const noexcept { return end_; }

        reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
        const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
        reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
        const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }

        const_iterator cbegin() const noexcept { return begin(); }
        const_iterator cend() const noexcept { return end(); }
        const_reverse_iterator crbegin() const noexcept { return rbegin(); }
        const_reverse_iterator crend() const noexcept { return rend(); }

        /**
         * @brief 是否为空
         * @note 通过 begin_ == end_ 判断
         */
        bool empty() const noexcept { return begin_ == end_; }
        /**
         * @brief 元素个数
         * @note 通过 end_ - begin_ 计算
         */
        size_type size() const noexcept { return static_cast<size_type>(end_ - begin_); }
        /**
         * @brief 最大容量
         * @note 通过static_cast<size_type>(-1) / sizeof(T)，即该类型最大值/sizeof(T)
         */
        size_type max_size() const noexcept { return static_cast<size_type>(-1) / sizeof(T); }
        size_type capacity() const noexcept { return static_cast<size_type>(cap_ - begin_); }
        void reserve(size_type n);
        void shrink_to_fit();

        // 访问元素相关操作
        reference operator[](size_type n) {
            MYSTL_DEBUG(n < size());
            return *(begin_ + n);
        }
        const_reference operator[](size_type n) const {
            MYSTL_DEBUG(n < size());
            return *(begin_ + n);
        }
        reference at(size_type n) {
            THROW_OUT_OF_RANGE_IF(!(n < size()), "vector<T>::at() subscript out of range");
            return (*this)[n];
        }
        const_reference at(size_type n) const {
            THROW_OUT_OF_RANGE_IF(!(n < size()), "vector<T>::at() subscript out of range");
            return (*this)[n];
        }

        reference front() {
            MYSTL_DEBUG(!empty());
            return *begin_;
        }
        const_reference front() const {
            MYSTL_DEBUG(!empty());
            return *begin_;
        }
        reference back() {
            MYSTL_DEBUG(!empty());
            return *(end_ - 1);
        }
        const_reference back() const {
            MYSTL_DEBUG(!empty());
            return *(end_ - 1);
        }

        pointer data() noexcept { return begin_; }
        const_pointer data() const noexcept { return begin_; }

        // 修改容器相关操作

        /**
         * @brief 将n个元素赋值为value
         */
        void assign(size_type n, const value_type &value) { fill_assign(n, value); }
        /**
         * @brief 将范围[first, last)赋值给容器
         */
        template<class Iter, typename std::enable_if<mystl::is_input_iterator<Iter>::value, int>::type = 0>
        void assign(Iter first, Iter last) {
            MYSTL_DEBUG(!(last < first));
            copy_assign(first, last, iterator_category(first));
        }

        /**
         * @brief 将通过花括号赋值给容器
         */
        void assign(std::initializer_list<value_type> il) {
            copy_assign(il.begin(), il.end(), mystl::forward_iterator_tag{});
        }

        // emplace / emplace_back

        template<class... Args>
        iterator emplace(const_iterator pos, Args &&...args);

        template<class... Args>
        void emplace_back(Args &&...args);

        // push_back / pop_back

        void push_back(const value_type &value);
        void push_back(value_type &&value) { emplace_back(mystl::move(value)); }

        void pop_back();

        // insert

        /**
         * @brief 拷贝插入函数
         */
        iterator insert(const_iterator pos, const value_type &value);
        /**
         * @brief 移动插入函数
         */
        iterator insert(const_iterator pos, value_type &&value) { return emplace(pos, mystl::move(value)); }

        /**
         * @brief 在pos位置插入n个元素，每个元素的值为value
         */
        iterator insert(const_iterator pos, size_type n, const value_type &value) {
            MYSTL_DEBUG(pos >= begin() && pos <= end());
            return fill_insert(const_cast<iterator>(pos), n, value);
        }

        /**
         * @brief 范围插入函数
         */
        template<class Iter, typename std::enable_if<mystl::is_input_iterator<Iter>::value, int>::type = 0>
        void insert(const_iterator pos, Iter first, Iter last) {
            MYSTL_DEBUG(pos >= begin() && pos <= end() && !(last < first));
            copy_insert(const_cast<iterator>(pos), first, last);
        }

        // erase / clear
        iterator erase(const_iterator pos);
        iterator erase(const_iterator first, const_iterator last);
        void clear() { erase(begin(), end()); }

        // resize / reverse
        void resize(size_type new_size) { return resize(new_size, value_type()); }
        void resize(size_type new_size, const value_type &value);

        /**
         * @brief 反转元素
         */
        void reverse() { mystl::reverse(begin(), end()); }

        void swap(vector &rhs) noexcept;

    private:
        // helper functions

        void try_init() noexcept;

        void init_space(size_type size, size_type cap);

        void fill_init(size_type n, const value_type &value);
        template<class Iter>
        void range_init(Iter first, Iter last);

        void destroy_and_recover(iterator first, iterator last, size_type n);

        size_type get_new_cap(size_type add_size);

        // assign

        void fill_assign(size_type n, const value_type &value);

        template<class IIter>
        void copy_assign(IIter first, IIter last, input_iterator_tag);

        template<class FIter>
        void copy_assign(FIter first, FIter last, forward_iterator_tag);

        // reallocate

        template<class... Args>
        void reallocate_emplace(iterator pos, Args &&...args);
        void reallocate_insert(iterator pos, const value_type &value);

        // insert

        iterator fill_insert(iterator pos, size_type n, const value_type &value);
        template<class IIter>
        void copy_insert(iterator pos, IIter first, IIter last);

        // shrink_to_fit

        void reinsert(size_type size);
    };

    /*****************************************************************************************/

    // 复制赋值操作符
    template<class T>
    vector<T> &vector<T>::operator=(const vector &rhs) {
        if (this != &rhs)
        {
            const auto len = rhs.size();
            if (len > capacity())
            {
                vector tmp(rhs.begin(), rhs.end());
                swap(tmp);
            }
            else if (size() >= len)
            {
                auto i = mystl::copy(rhs.begin(), rhs.end(), begin());
                data_allocator::destroy(i, end_);
                end_ = begin_ + len;
            }
            else
            {
                mystl::copy(rhs.begin(), rhs.begin() + size(), begin_);
                mystl::uninitialized_copy(rhs.begin() + size(), rhs.end(), end_);
                // cap_ = end_ = begin_ + len;
                end_ = begin_ + len;
            }
        }
        return *this;
    }

    // 移动赋值操作符
    template<class T>
    vector<T> &vector<T>::operator=(vector &&rhs) noexcept {
        destroy_and_recover(begin_, end_, cap_ - begin_);
        begin_ = rhs.begin_;
        end_ = rhs.end_;
        cap_ = rhs.cap_;
        rhs.begin_ = nullptr;
        rhs.end_ = nullptr;
        rhs.cap_ = nullptr;
        return *this;
    }

    /**
     * @brief 预留空间大小，当原容量小于要求大小时，才会重新分配
     */
    template<class T>
    void vector<T>::reserve(size_type n) {
        if (capacity() < n)
        {
            THROW_LENGTH_ERROR_IF(n > max_size(), "n can not larger than max_size() in vector<T>::reserve(n)");
            const auto old_size = size();
            auto tmp = data_allocator::allocate(n);
            mystl::uninitialized_move(begin_, end_, tmp);
            data_allocator::deallocate(begin_, cap_ - begin_);
            begin_ = tmp;
            end_ = tmp + old_size;
            cap_ = begin_ + n;
        }
    }

    /**
     * @brief 若容器大小大于元素个数，则将多余的容量释放
     */
    template<class T>
    void vector<T>::shrink_to_fit() {
        if (end_ < cap_) { reinsert(size()); }
    }

    /**
     * @brief 在 pos 位置就地构造元素，避免额外的复制或移动开销
     * @return 返回迭代器指向插入元素的位置
     */
    template<class T>
    template<class... Args>
    typename vector<T>::iterator vector<T>::emplace(const_iterator pos, Args &&...args) {
        MYSTL_DEBUG(pos >= begin() && pos <= end());
        iterator xpos = const_cast<iterator>(pos);
        const size_type n = xpos - begin_;
        // 在尾部插入且有剩余空间
        if (end_ != cap_ && xpos == end_)
        {
            data_allocator::construct(mystl::address_of(*end_), mystl::forward<Args>(args)...);
            ++end_;
        }
        else if (end_ != cap_)
        {
            auto new_end = end_;
            data_allocator::construct(mystl::address_of(*end_), *(end_ - 1));
            ++new_end;
            mystl::copy_backward(xpos, end_ - 1, end_);
            *xpos = value_type(mystl::forward<Args>(args)...);
            end_ = new_end;
        }
        else { reallocate_emplace(xpos, mystl::forward<Args>(args)...); }
        return begin() + n;
    }

    /**
     * @brief 在尾部就地构造元素，避免额外的复制或移动开销
     */
    template<class T>
    template<class... Args>
    void vector<T>::emplace_back(Args &&...args) {
        if (end_ < cap_)
        {
            data_allocator::construct(mystl::address_of(*end_), mystl::forward<Args>(args)...);
            ++end_;
        }
        else { reallocate_emplace(end_, mystl::forward<Args>(args)...); }
    }

    /**
     * @brief 在尾部插入元素
     */
    template<class T>
    void vector<T>::push_back(const value_type &value) {
        if (end_ != cap_)
        {
            data_allocator::construct(mystl::address_of(*end_), value);
            ++end_;
        }
        else { reallocate_insert(end_, value); }
    }

    /**
     * @brief 删除尾部元素
     */
    template<class T>
    void vector<T>::pop_back() {
        MYSTL_DEBUG(!empty());
        data_allocator::destroy(end_ - 1);
        --end_;
    }

    /**
     * @brief 在 pos 位置插入元素
     */
    template<class T>
    typename vector<T>::iterator vector<T>::insert(const_iterator pos, const value_type &value) {
        MYSTL_DEBUG(pos >= begin() && pos <= end());
        iterator xpos = const_cast<iterator>(pos);
        const size_type n = pos - begin_;
        // 容器未满，且插入位置是尾部
        if (end_ != cap_ && xpos == end_)
        {
            data_allocator::construct(mystl::address_of(*end_), value);
            ++end_;
        }
        // 容器未满，但插入位置不是尾部
        else if (end_ != cap_)
        {
            auto new_end = end_;
            // 在尾部构造一个副本元素
            data_allocator::construct(mystl::address_of(*end_), *(end_ - 1));
            ++new_end;
            auto value_copy = value;// 避免元素因以下复制操作而被改变
            // 将[xpos, end_-1)区间的元素向后移动一个位置
            mystl::copy_backward(xpos, end_ - 1, end_);
            *xpos = mystl::move(value_copy);
            end_ = new_end;
        }
        // 容器已满，则重新分配空间
        else { reallocate_insert(xpos, value); }
        return begin_ + n;
    }

    /**
     * @brief 删除 pos 位置上的元素
     */
    template<class T>
    typename vector<T>::iterator vector<T>::erase(const_iterator pos) {
        MYSTL_DEBUG(pos >= begin() && pos < end());
        iterator xpos = begin_ + (pos - begin());
        // 将后续元素向前移动一位（使用移动语义）
        mystl::move(xpos + 1, end_, xpos);
        // 销毁最后一个元素
        data_allocator::destroy(end_ - 1);
        --end_;
        return xpos;
    }

    /**
     * @brief 删除[first, last)上的元素
     * @return 返回迭代器指向删除区间之后的首元素
     */
    template<class T>
    typename vector<T>::iterator vector<T>::erase(const_iterator first, const_iterator last) {
        MYSTL_DEBUG(first >= begin() && last <= end() && !(last < first));
        const auto n = first - begin();
        iterator r = begin_ + (first - begin());
        // mystl::move 将 [last, end()) 区间的元素移动到从 r 开始的位置（覆盖 [first, last) 区间）
        // destroy 销毁移动后原位置剩余的元素
        data_allocator::destroy(mystl::move(r + (last - first), end_, r), end_);
        end_ = end_ - (last - first);
        return begin_ + n;
    }

    /**
     * @brief 调整容器大小，若 new_size 小于当前大小，则删除多余元素，若 new_size 大于当前大小，则添加元素
     */
    template<class T>
    void vector<T>::resize(size_type new_size, const value_type &value) {
        if (new_size < size()) { erase(begin() + new_size, end()); }
        else { insert(end(), new_size - size(), value); }
    }

    /**
     * @brief 交换两个 vector 的内容
     * @note 仅交换关键迭代器
     */
    template<class T>
    void vector<T>::swap(vector<T> &rhs) noexcept {
        if (this != &rhs)
        {
            mystl::swap(begin_, rhs.begin_);
            mystl::swap(end_, rhs.end_);
            mystl::swap(cap_, rhs.cap_);
        }
    }

    /*****************************************************************************************/
    // helper function

    /**
     * @brief 尝试分配空间，若分配失败则忽略，不抛出异常
     * @note 默认16个元素
     */
    template<class T>
    void vector<T>::try_init() noexcept {
        try
        {
            begin_ = data_allocator::allocate(16);
            end_ = begin_;
            cap_ = begin_ + 16;
        }
        catch (...)
        {
            begin_ = nullptr;
            end_ = nullptr;
            cap_ = nullptr;
        }
    }

    /**
     * @brief 构造cap个元素的空间，分配失败则抛出异常
     * @note 容器元素修改为size个
     */
    template<class T>
    void vector<T>::init_space(size_type size, size_type cap) {
        try
        {
            begin_ = data_allocator::allocate(cap);
            end_ = begin_ + size;
            cap_ = begin_ + cap;
        }
        catch (...)
        {
            begin_ = nullptr;
            end_ = nullptr;
            cap_ = nullptr;
            throw;
        }
    }

    /**
     * @brief 构造n元素为 value 的 vector
     * @note 若 n > 16，则分配空间为 n，否则分配空间为 16
     */
    template<class T>
    void vector<T>::fill_init(size_type n, const value_type &value) {
        const size_type init_size = mystl::max(static_cast<size_type>(16), n);
        init_space(n, init_size);
        mystl::uninitialized_fill_n(begin_, n, value);
    }

    /**
     * @brief 构造元素为 [first, last) 的 vector
     * @note 若 [first, last) 长度大于 16，则分配空间为 [first, last) 长度，否则分配空间为 16
     */
    template<class T>
    template<class Iter>
    void vector<T>::range_init(Iter first, Iter last) {
        const size_type len = mystl::distance(first, last);
        const size_type init_size = mystl::max(len, static_cast<size_type>(16));
        init_space(len, init_size);
        mystl::uninitialized_copy(first, last, begin_);
    }


    /**
     * @brief 销毁 [first, last) 区间上的元素，并释放内存
     */
    template<class T>
    void vector<T>::destroy_and_recover(iterator first, iterator last, size_type n) {
        data_allocator::destroy(first, last);
        data_allocator::deallocate(first, n);
    }

    // get_new_cap 函数
    /**
     * @brief 计算增加新元素后的新容量大小
     * @note 如果接近上限，则按需分配空间，最大多增加16个元素位置
     */
    template<class T>
    typename vector<T>::size_type vector<T>::get_new_cap(size_type add_size) {
        const auto old_size = capacity();
        THROW_LENGTH_ERROR_IF(old_size > max_size() - add_size, "vector<T>'s size too big");
        // 当前容量已经接近最大容量上限
        if (old_size > max_size() - old_size / 2)
        { return old_size + add_size > max_size() - 16 ? old_size + add_size : old_size + add_size + 16; }
        // 常规扩容
        const size_type new_size = old_size == 0 ? mystl::max(add_size, static_cast<size_type>(16))
                                                 : mystl::max(old_size + old_size / 2, old_size + add_size);
        return new_size;
    }

    /**
     * @brief 填充 n 个元素为 value
     * @note 若 n > 容量，则分配新空间，否则调整元素
     */
    template<class T>
    void vector<T>::fill_assign(size_type n, const value_type &value) {
        if (n > capacity())
        {
            vector tmp(n, value);
            swap(tmp);
        }
        else if (n > size())
        {
            mystl::fill(begin(), end(), value);
            end_ = mystl::uninitialized_fill_n(end_, n - size(), value);
        }
        else { erase(mystl::fill_n(begin_, n, value), end_); }
    }

    /**
     * @brief 用 [first, last) 为容器赋值
     * @note 若 [first, last) 长度小于容器，则删除容器多余元素
     * @note 若 [first, last) 长度大于容器，则在尾部插入剩余元素
     */
    template<class T>
    template<class IIter>
    void vector<T>::copy_assign(IIter first, IIter last, input_iterator_tag) {
        auto cur = begin_;
        for (; first != last && cur != end_; ++first, ++cur) { *cur = *first; }
        if (first == last) { erase(cur, end_); }
        else { insert(end_, first, last); }
    }

    /**
     * @brief 用 [first, last) 为容器赋值
     * @note 当前元素个数大于等于要赋值的元素个数，则删除容器多余元素
     * @note 容量不足，则构造临时容器直接交换
     * @note 当前元素个数小于输入范围长度，但当前容量足够，直接拷贝
     */
    template<class T>
    template<class FIter>
    void vector<T>::copy_assign(FIter first, FIter last, forward_iterator_tag) {
        const size_type len = mystl::distance(first, last);
        // 容量不足
        if (len > capacity())
        {
            vector tmp(first, last);
            swap(tmp);
        }
        // 当前元素个数大于等于要赋值的元素个数
        else if (size() >= len)
        {
            auto new_end = mystl::copy(first, last, begin_);
            data_allocator::destroy(new_end, end_);
            end_ = new_end;
        }
        // 当前元素个数小于输入范围长度，但当前容量足够
        else
        {
            auto mid = first;
            mystl::advance(mid, size());
            // 覆盖已有元素
            mystl::copy(first, mid, begin_);
            // 将剩余元素复制到未初始化空间
            auto new_end = mystl::uninitialized_copy(mid, last, end_);
            end_ = new_end;
        }
    }

    /**
     * @brief 重新分配空间并在 pos 处就地构造元素
     */
    template<class T>
    template<class... Args>
    void vector<T>::reallocate_emplace(iterator pos, Args &&...args) {
        const auto new_size = get_new_cap(1);
        auto new_begin = data_allocator::allocate(new_size);
        auto new_end = new_begin;
        try
        {
            new_end = mystl::uninitialized_move(begin_, pos, new_begin);
            data_allocator::construct(mystl::address_of(*new_end), mystl::forward<Args>(args)...);
            ++new_end;
            new_end = mystl::uninitialized_move(pos, end_, new_end);
        }
        catch (...)
        {
            data_allocator::deallocate(new_begin, new_size);
            throw;
        }
        destroy_and_recover(begin_, end_, cap_ - begin_);
        begin_ = new_begin;
        end_ = new_end;
        cap_ = new_begin + new_size;
    }

    /**
     * @brief 重新分配空间并在 pos 处插入元素
     */
    template<class T>
    void vector<T>::reallocate_insert(iterator pos, const value_type &value) {
        // 获取新的容量
        const auto new_size = get_new_cap(1);
        auto new_begin = data_allocator::allocate(new_size);
        auto new_end = new_begin;
        const value_type &value_copy = value;
        try
        {
            // 将pos前的元素移动到新空间
            new_end = mystl::uninitialized_move(begin_, pos, new_begin);
            // 在pos处插入元素
            data_allocator::construct(mystl::address_of(*new_end), value_copy);
            ++new_end;
            // 移动剩余元素
            new_end = mystl::uninitialized_move(pos, end_, new_end);
        }
        catch (...)
        {
            data_allocator::deallocate(new_begin, new_size);
            throw;
        }
        // 销毁旧元素并释放内存
        destroy_and_recover(begin_, end_, cap_ - begin_);
        begin_ = new_begin;
        end_ = new_end;
        cap_ = new_begin + new_size;
    }

    /**
     * @brief 在 pos 位置插入 n 个 value 值
     */
    template<class T>
    typename vector<T>::iterator vector<T>::fill_insert(iterator pos, size_type n, const value_type &value) {
        if (n == 0) return pos;
        const size_type xpos = pos - begin_;
        // 避免被覆盖
        const value_type value_copy = value;
        // 如果备用空间大于等于增加的空间
        if (static_cast<size_type>(cap_ - end_) >= n)
        {
            const size_type after_elems = end_ - pos;
            auto old_end = end_;
            // 插入点后的元素多于n
            if (after_elems > n)
            {
                // 将end_ - n到end_的元素（即最后n个元素）复制到未初始化的空间（从end_开始）
                mystl::uninitialized_copy(end_ - n, end_, end_);
                end_ += n;
                // 将[pos, old_end - n)的元素向后移动n个位置
                mystl::move_backward(pos, old_end - n, old_end);
                // 将[pos, pos + n)的元素赋值为value_copy
                mystl::uninitialized_fill_n(pos, n, value_copy);
            }
            // 插入点后元素少于或等于要插入的元素个数
            else
            {
                // 在尾部填充n - after_elems个value_copy
                // 即部分元素可以直接放到末尾
                end_ = mystl::uninitialized_fill_n(end_, n - after_elems, value_copy);
                // 将[pos, old_end)的元素后移到新末尾
                end_ = mystl::uninitialized_move(pos, old_end, end_);
                // 在[pos, pos+after_elems)区间填充value_copy
                mystl::uninitialized_fill_n(pos, after_elems, value_copy);
            }
        }
        // 如果备用空间不足
        else
        {
            const auto new_size = get_new_cap(n);
            auto new_begin = data_allocator::allocate(new_size);
            auto new_end = new_begin;
            try
            {
                new_end = mystl::uninitialized_move(begin_, pos, new_begin);
                new_end = mystl::uninitialized_fill_n(new_end, n, value);
                new_end = mystl::uninitialized_move(pos, end_, new_end);
            }
            catch (...)
            {
                destroy_and_recover(new_begin, new_end, new_size);
                throw;
            }
            data_allocator::deallocate(begin_, cap_ - begin_);
            begin_ = new_begin;
            end_ = new_end;
            cap_ = begin_ + new_size;
        }
        return begin_ + xpos;
    }

    /**
     * @brief 在 pos 位置插入 [first, last) 范围内的元素
     */
    template<class T>
    template<class IIter>
    void vector<T>::copy_insert(iterator pos, IIter first, IIter last) {
        if (first == last) return;
        const auto n = mystl::distance(first, last);
        // 如果备用空间大小足够
        if ((cap_ - end_) >= n)
        {
            const auto after_elems = end_ - pos;
            auto old_end = end_;
            // 插入点后的元素多于n
            if (after_elems > n)
            {
                // 将end_ - n到end_的元素（即最后n个元素）复制到未初始化的空间（从end_开始）
                end_ = mystl::uninitialized_copy(end_ - n, end_, end_);
                // 将[pos, old_end - n)的元素向后移动n个位置
                mystl::move_backward(pos, old_end - n, old_end);
                // 将[pos, pos + n)的元素赋值为first到last的元素
                mystl::uninitialized_copy(first, last, pos);
            }
            else
            {
                auto mid = first;
                mystl::advance(mid, after_elems);
                // 在尾部复制n - after_elems个元素
                // 即部分元素可以直接放到末尾
                end_ = mystl::uninitialized_copy(mid, last, end_);
                // 将[pos, old_end)的元素后移到新末尾
                end_ = mystl::uninitialized_move(pos, old_end, end_);
                // 将[pos, pos + after_elems)的元素赋值为first到mid的元素
                mystl::uninitialized_copy(first, mid, pos);
            }
        }
        // 备用空间不足
        else
        {
            const auto new_size = get_new_cap(n);
            auto new_begin = data_allocator::allocate(new_size);
            auto new_end = new_begin;
            try
            {
                new_end = mystl::uninitialized_move(begin_, pos, new_begin);
                new_end = mystl::uninitialized_copy(first, last, new_end);
                new_end = mystl::uninitialized_move(pos, end_, new_end);
            }
            catch (...)
            {
                destroy_and_recover(new_begin, new_end, new_size);
                throw;
            }
            data_allocator::deallocate(begin_, cap_ - begin_);
            begin_ = new_begin;
            end_ = new_end;
            cap_ = begin_ + new_size;
        }
    }

    /**
     * @brief 重新分配空间，并将原容器元素迁移到新空间
     */
    template<class T>
    void vector<T>::reinsert(size_type size) {
        auto new_begin = data_allocator::allocate(size);
        try
        { mystl::uninitialized_move(begin_, end_, new_begin); }
        catch (...)
        {
            data_allocator::deallocate(new_begin, size);
            throw;
        }
        data_allocator::deallocate(begin_, cap_ - begin_);
        begin_ = new_begin;
        end_ = begin_ + size;
        cap_ = begin_ + size;
    }

    /*****************************************************************************************/
    // 重载比较操作符

    template<class T>
    bool operator==(const vector<T> &lhs, const vector<T> &rhs) {
        return lhs.size() == rhs.size() && mystl::equal(lhs.begin(), lhs.end(), rhs.begin());
    }

    template<class T>
    bool operator<(const vector<T> &lhs, const vector<T> &rhs) {
        return mystl::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    template<class T>
    bool operator!=(const vector<T> &lhs, const vector<T> &rhs) {
        return !(lhs == rhs);
    }

    template<class T>
    bool operator>(const vector<T> &lhs, const vector<T> &rhs) {
        return rhs < lhs;
    }

    template<class T>
    bool operator<=(const vector<T> &lhs, const vector<T> &rhs) {
        return !(rhs < lhs);
    }

    template<class T>
    bool operator>=(const vector<T> &lhs, const vector<T> &rhs) {
        return !(lhs < rhs);
    }

    // 重载 mystl 的 swap
    template<class T>
    void swap(vector<T> &lhs, vector<T> &rhs) {
        lhs.swap(rhs);
    }

}// namespace mystl
#endif// !MYTINYSTL_VECTOR_H_