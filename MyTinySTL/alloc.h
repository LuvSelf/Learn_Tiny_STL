#ifndef MYTINYSTL_ALLOC_H_
#define MYTINYSTL_ALLOC_H_

// 这个头文件包含一个类 alloc，用于分配和回收内存，以内存池的方式实现
//
// 从 v2.0.0 版本开始，将不再使用内存池，这个文件将被弃用，但暂时保留
//
// 注意！！！
// 我知道这个文件里很多实现是错的，这是很久很久前写的了，后面已经不用这个东西了，
// 所以我也没再维护，有诸多问题，已经有人在issue中都提了，free_list的修改，
// 指针作为参数时没实际修改到原指针，等等。相信会看这么仔细的，大部分都是
// 初学C++的朋友，大佬都不会看这些玩具了，所以其中的错误，就留给对内存池实现
// 感兴趣的朋友去修改啦！

#include <new>

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <algorithm>

namespace mystl
{

    // 共用体: FreeList
    // 采用链表的方式管理内存碎片，分配与回收小内存（<=4K）区块
    union FreeList
    {
        union FreeList *next;// 指向下一个区块
        char data[1];        // 储存本块内存的首地址
    };

    // 不同内存范围的上调大小
    enum
    {
        EAlign128 = 8,
        EAlign256 = 16,
        EAlign512 = 32,
        EAlign1024 = 64,
        EAlign2048 = 128,
        EAlign4096 = 256
    };

    // 小对象的内存大小
    // 小于等于4096字节的对象视为小对象，使用内存池管理
    enum
    {
        ESmallObjectBytes = 4096
    };

    // free lists 个数
    // 56个自由链表，每个链表管理特定大小的内存块
    enum
    {
        EFreeListsNumber = 56
    };

    // 空间配置类 alloc
    // 如果内存较大，超过 4096 bytes，直接调用 std::malloc, std::free
    // 当内存较小时，以内存池管理，每次配置一大块内存，并维护对应的自由链表
    class alloc
    {
    private:
        static char *start_free;// 内存池起始位置
        static char *end_free;  // 内存池结束位置
        // 记录向系统申请的总内存大小
        static size_t heap_size;// 申请 heap 空间附加值大小

        // 56个自由链表的数组，初始化为空
        static FreeList *free_list[EFreeListsNumber];// 自由链表

    public:
        static void *allocate(size_t n);
        static void deallocate(void *p, size_t n);
        static void *reallocate(void *p, size_t old_size, size_t new_size);

    private:
        static size_t M_align(size_t bytes);
        static size_t M_round_up(size_t bytes);
        static size_t M_freelist_index(size_t bytes);
        static void *M_refill(size_t n);
        static char *M_chunk_alloc(size_t size, size_t &nobj);
    };

    // 静态成员变量初始化

    char *alloc::start_free = nullptr;
    char *alloc::end_free = nullptr;
    size_t alloc::heap_size = 0;

    FreeList *alloc::free_list[EFreeListsNumber] = {
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

    // 分配大小为 n 的空间， n > 0
    inline void *alloc::allocate(size_t n) {
        FreeList *my_free_list;
        FreeList *result;
        if (n > static_cast<size_t>(ESmallObjectBytes)) return std::malloc(n);
        my_free_list = free_list[M_freelist_index(n)];
        result = my_free_list;
        if (result == nullptr)
        {
            void *r = M_refill(M_round_up(n));
            return r;
        }
        my_free_list = result->next;
        return result;
    }

    // 释放 p 指向的大小为 n 的空间, p 不能为 0
    inline void alloc::deallocate(void *p, size_t n) {
        if (n > static_cast<size_t>(ESmallObjectBytes))
        {
            std::free(p);
            return;
        }
        // 修改：使用二级指针
        FreeList *q = reinterpret_cast<FreeList *>(p);
        FreeList **my_free_list;
        my_free_list = &free_list[M_freelist_index(n)];
        q->next = *my_free_list;
        *my_free_list = q;
    }

    // 重新分配空间，接受三个参数，参数一为指向新空间的指针，参数二为原来空间的大小，参数三为申请空间的大小
    inline void *alloc::reallocate(void *p, size_t old_size, size_t new_size) {
        deallocate(p, old_size);
        p = allocate(new_size);
        return p;
    }

    // bytes 对应上调大小
    inline size_t alloc::M_align(size_t bytes) {
        if (bytes <= 512)
        {
            return bytes <= 256 ? bytes <= 128 ? EAlign128 : EAlign256
                                : EAlign512;
        }
        return bytes <= 2048 ? bytes <= 1024 ? EAlign1024 : EAlign2048
                             : EAlign4096;
    }

    // 将 bytes 上调至对应区间大小
    // 如(100 + 128 - 1) = 227
    // 227 & ~(127) = 227 & 0xFFFFFF80 = 128
    inline size_t alloc::M_round_up(size_t bytes) {
        return ((bytes + M_align(bytes) - 1) & ~(M_align(bytes) - 1));
    }

    // 根据区块大小，选择第 n 个 free lists
    inline size_t alloc::M_freelist_index(size_t bytes) {
        if (bytes <= 512)
        {
            return bytes <= 256
                           ? bytes <= 128
                                     ? ((bytes + EAlign128 - 1) / EAlign128 - 1)
                                     : (15 +
                                        (bytes + EAlign256 - 129) / EAlign256)
                           : (23 + (bytes + EAlign512 - 257) / EAlign512);
        }
        return bytes <= 2048 ? bytes <= 1024
                                       ? (31 + (bytes + EAlign1024 - 513) /
                                                       EAlign1024)
                                       : (39 + (bytes + EAlign2048 - 1025) /
                                                       EAlign2048)
                             : (47 + (bytes + EAlign4096 - 2049) / EAlign4096);
    }

    // 重新填充 free list
    void *alloc::M_refill(size_t n) {
        /* //node: 好像没把新链表更新到原链表
        size_t nblock = 10;
        char *c = M_chunk_alloc(n, nblock);
        FreeList *my_free_list;
        FreeList *result, *cur, *next;
        // 如果只有一个区块，就把这个区块返回给调用者，free list 没有增加新节点
        if (nblock == 1) return c;
        // 否则把一个区块给调用者，剩下的纳入 free list 作为新节点
        my_free_list = free_list[M_freelist_index(n)];
        result = (FreeList *) c;// 将第一个区块作为返回给用户的结果
        my_free_list = next = (FreeList *) (c + n);
        for (size_t i = 1;; ++i)
        {
            cur = next;
            next = (FreeList *) ((char *) next + n);
            if (nblock - 1 == i)
            {
                cur->next = nullptr;
                break;
            }
            else { cur->next = next; }
        }
        return result; */
        size_t nblock = 10;                // 默认申请10个区块
        char *c = M_chunk_alloc(n, nblock);// 从内存池申请空间
        size_t index = M_freelist_index(n);// 获取自由链表索引
        // 情况1：仅获1个区块 → 直接返回用户
        if (nblock == 1) return c;

        // 情况2：多个区块 → 头插法构建新链表
        FreeList *result = (FreeList *) c;  // 首个区块返回用户
        FreeList **head = &free_list[index];// 获取链表头指针的地址（二级指针）
        FreeList *next_block = (FreeList *) (c + n);// 第二个区块起始位置

        // 头插法构建剩余区块的链表（逆序链接）
        for (size_t i = 1; i < nblock; ++i)
        {
            FreeList *current = next_block;
            next_block = (FreeList *) ((char *) next_block + n);
            current->next = *head;
            *head = current;
        }
    }

    // 从内存池中取空间给 free list 使用，条件不允许时，会调整 nblock
    char *alloc::M_chunk_alloc(size_t size, size_t &nblock) {
        char *result;
        size_t need_bytes = size * nblock;        // 总需求字节数
        size_t pool_bytes = end_free - start_free;// 内存池剩余大小

        // 情况1：内存池剩余空间完全满足需求
        if (pool_bytes >= need_bytes)
        {
            result = start_free;
            start_free += need_bytes;
            return result;
        }

        // 情况2：内存池剩余空间不足以满足全部需求，但至少能分配一个区块
        if (pool_bytes >= size)
        {
            nblock = pool_bytes / size;
            need_bytes = size * nblock;
            result = start_free;
            start_free += need_bytes;
            return result;
        }

        // 情况3：内存池剩余空间连一个区块都无法提供
        /* else
        {
            //原代码：未修改原链表且递归有栈溢出风险
            // 修改：使用二级指针
            //  如果内存池还有剩余，把剩余的空间加入到 free list 中
            if (pool_bytes > 0)
            {
                // 找到合适大小的空闲列表
                FreeList *my_free_list =
                        free_list[M_freelist_index(pool_bytes)];
                // 将剩余空间链入链表头
                ((FreeList *) start_free)->next = my_free_list;
                my_free_list = (FreeList *) start_free;
            }

            //  计算新申请 heap 内存大小
            size_t bytes_to_get =
                    (need_bytes << 1) + M_round_up(heap_size >> 4);
            start_free = (char *) std::malloc(bytes_to_get);

            // 申请失败处理
            if (!start_free)
            {
                FreeList *my_free_list, *p;
                // 试着查找有无未用且区块足够大的 free list
                for (size_t i = size; i <= ESmallObjectBytes; i += M_align(i))
                {
                    my_free_list = free_list[M_freelist_index(i)];
                    p = my_free_list;
                    if (p)
                    {
                        my_free_list = p->next;
                        start_free = (char *) p;
                        end_free = start_free + i;
                        return M_chunk_alloc(size, nblock);
                    }
                }
                std::printf("out of memory");
                end_free = nullptr;
                throw std::bad_alloc();
            }
            end_free = start_free + bytes_to_get;
            heap_size += bytes_to_get;
            return M_chunk_alloc(size, nblock);
        } */
        // 剩余内存放入free_list中
        if (pool_bytes > 0)
        {
            size_t idx = M_freelist_index(pool_bytes);
            FreeList *block = (FreeList *) start_free;
            block->next = free_list[idx];
            free_list[idx] = block;
        }

        // 计算新申请大小（带上限）
        size_t bytes_to_get = (need_bytes << 1) + M_round_up(heap_size >> 4);
        const size_t MAX_CHUNK = 1024 * 1024;// 1MB
        if (bytes_to_get > MAX_CHUNK)
        { bytes_to_get = std::max(need_bytes, MAX_CHUNK); }

        // 申请新内存
        start_free = static_cast<char *>(std::malloc(bytes_to_get));
        // 申请失败
        if (!start_free)
        {
            // 尝试使用自由链表中的大区块
            for (size_t bytes = size; bytes <= ESmallObjectBytes;
                 bytes += M_align(bytes))
            {
                size_t idx = M_freelist_index(bytes);
                if (free_list[idx])
                {
                    FreeList *block = free_list[idx];
                    free_list[idx] = block->next;// 从链表移除
                    start_free = reinterpret_cast<char *>(block);
                    end_free = start_free + bytes;
                    return M_chunk_alloc(size, nblock);// 重用该块
                }
            }
            end_free = nullptr;
            throw std::bad_alloc();
        }
        // 初始化新内存池
        end_free = start_free + bytes_to_get;
        heap_size += bytes_to_get;
        return M_chunk_alloc(size, nblock);// 递归处理
    }

}// namespace mystl
#endif// !MYTINYSTL_ALLOC_H_
