#ifndef MYTINYSTL_STREAM_ITERATOR_H_
#define MYTINYSTL_STREAM_ITERATOR_H_

/**
 * @brief stream_iterator.h
 * @note 定义了istream_iterator和ostream_iterator类
 */

#include "cc_Containers/basic_string.h"

namespace mystl
{
    /**
     * @param T 迭代器读取的数据类型
     * @param CharT 字符类型(兼容wchar_t等)
     * @param Traits 字符特性(默认std::char_traits)
     * @param Dist 距离类型(默认ptrdiff_t)
     * @note 继承自input_iterator_tag标记为输入迭代器
     * @note istream_iterator 是输入流迭代器，用于从 basic_istream 按顺序读取数据
     */
    template<typename T, typename CharT = char, typename Traits = std::char_traits<CharT>, typename Dist = ptrdiff_t>
    class istream_iterator : public iterator<input_iterator_tag, T, Dist, const T *, const T &>
    {
    public:
        using char_type = CharT;
        using traits_type = Traits;
        using istream_type = std::basic_istream<CharT, Traits>;

        istream_iterator() /* noexcept(std::is_nothrow_default_constructible<T>::value) */
            : m_stream{nullptr}, m_value{} {}

        // 立即执行首次读取
        istream_iterator(istream_type &is) : m_stream{std::addressof(is)} { read(); }

        istream_iterator(const istream_iterator &other) /* noexcept(std::is_nothrow_copy_constructible<T>::value) */
                = default;                              // memberwise copy

        istream_iterator &operator=(const istream_iterator &) = default;// memberwise copy-asgn

        ~istream_iterator() = default;

        // 返回 m_value 的常量引用（只读访问）
        const T &operator*() const noexcept {
            MYSTL_DEBUG(m_stream != nullptr);
            return m_value;
        }

        const T *operator->() const noexcept { return std::addressof(*this); }

        istream_iterator &operator++() {
            MYSTL_DEBUG(m_stream != nullptr);
            read();
            return *this;
        }

        istream_iterator operator++(int) {
            auto tmp = *this;
            ++*this;
            return tmp;
        }

    private:
        // 绑定的输入流指针
        istream_type *m_stream;
        // 当前读取的值
        T m_value;
        // 标识流结束（如 EOF）
        bool end_marker;

        // 执行实际读取 *m_stream >> m_value，失败时将 m_stream=nullptr
        void read() {
            /* if (m_stream && !(*m_stream >> m_value))
            {
                // m_stream 有效且读到 EOS
                m_stream = nullptr;
            } */
            if (m_stream)
            {
                *m_stream >> m_value;
                if (m_stream->fail() && !m_stream->eof()) { throw std::runtime_error("istream_iterator: read failed"); }
                end_marker = !(*m_stream);
            }
        }

        friend bool operator==(const istream_iterator &lhs, const istream_iterator &rhs) {
            return lhs.m_stream == rhs.m_stream;
        }

        friend bool operator!=(const istream_iterator &lhs, const istream_iterator &rhs) {
            return lhs.m_stream != rhs.m_stream;
        }
    };


    // TODO
    template<typename T, typename CharT = char, typename Traits = std::char_traits<CharT>>
    class ostream_iterator : public iterator<output_iterator_tag, void, void, void, void>
    {
    public:
        using ostream_type = std::basic_ostream<CharT, Traits>;
        using char_type = CharT;

        // 构造函数
        ostream_iterator(ostream_type &os) : stream(&os), delimiter(nullptr) {}
        ostream_iterator(ostream_type &os, const CharT *delim) : stream(&os), delimiter(delim) {}

        // 赋值操作符：输出值 + 分隔符
        ostream_iterator &operator=(const T &value) {
            if (stream)
            {
                *stream << value;
                if (delimiter) *stream << delimiter;
            }
            return *this;
        }
        // 迭代器操作（空操作）
        ostream_iterator &operator*() { return *this; }
        ostream_iterator &operator++() { return *this; }
        ostream_iterator &operator++(int) { return *this; }

    private:
        ostream_type *stream;  // 输出流指针
        const CharT *delimiter;// 分隔符（可选）
    };
}// namespace mystl


#endif