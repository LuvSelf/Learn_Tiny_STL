#ifndef MYTINYSTL_HEAP_ALGO_H_
#define MYTINYSTL_HEAP_ALGO_H_

/**
 * @brief heap_algo.h
 * @note 包含 heap 的四个算法 : push_heap, pop_heap, sort_heap, make_heap
 */

#include "iterator.h"

namespace mystl
{

    /**
     * @brief 将容器末尾新元素插入堆，调整堆结构
     * @note 从末尾开始，将新元素与父节点比较并上浮，直到满足堆性质（上溯过程）
     */
    template<class RandomIter, class Distance, class T>
    void push_heap_aux(RandomIter first, Distance holeIndex, Distance topIndex, T value) {
        // 计算父节点位置
        auto parent = (holeIndex - 1) / 2;

        while (holeIndex > topIndex && *(first + parent) < value)
        {
            // 使用 operator<，所以 heap 为 max-heap
            *(first + holeIndex) = *(first + parent);
            holeIndex = parent;
            parent = (holeIndex - 1) / 2;
        }
        *(first + holeIndex) = value;
    }

    template<class RandomIter, class Distance>
    void push_heap_d(RandomIter first, RandomIter last, Distance *) {
        mystl::push_heap_aux(first, (last - first) - 1, static_cast<Distance>(0), *(last - 1));
    }

    /**
     * @brief 将容器末尾新元素插入堆，调整堆结构。
     * @note 新元素应该已置于底部容器的最尾端
     */
    template<class RandomIter>
    void push_heap(RandomIter first, RandomIter last) {
        mystl::push_heap_d(first, last, distance_type(first));
    }

    // 重载版本使用函数对象 comp 代替比较操作
    template<class RandomIter, class Distance, class T, class Compared>
    void push_heap_aux(RandomIter first, Distance holeIndex, Distance topIndex, T value, Compared comp) {
        auto parent = (holeIndex - 1) / 2;
        while (holeIndex > topIndex && comp(*(first + parent), value))
        {
            *(first + holeIndex) = *(first + parent);
            holeIndex = parent;
            parent = (holeIndex - 1) / 2;
        }
        *(first + holeIndex) = value;
    }

    template<class RandomIter, class Compared, class Distance>
    void push_heap_d(RandomIter first, RandomIter last, Distance *, Compared comp) {
        mystl::push_heap_aux(first, (last - first) - 1, static_cast<Distance>(0), *(last - 1), comp);
    }

    /**
     * @brief 将容器末尾新元素插入堆，调整堆结构。
     * @note 新元素应该已置于底部容器的最尾端
     */
    template<class RandomIter, class Compared>
    void push_heap(RandomIter first, RandomIter last, Compared comp) {
        mystl::push_heap_d(first, last, distance_type(first), comp);
    }

    /**
     * @brief 将堆顶元素移至容器末尾，调整剩余元素为新堆
     * @note 交换堆顶与末尾元素
     * @note 从根节点开始下沉，选择较大子节点替换空洞
     * @note 最后执行一次上溯确保完全堆化
     */
    template<class RandomIter, class T, class Distance>
    void adjust_heap(RandomIter first, Distance holeIndex, Distance len, T value) {
        // 先进行下溯(percolate down)过程
        auto topIndex = holeIndex;
        auto rchild = 2 * holeIndex + 2;
        while (rchild < len)
        {
            // 选较大子节点
            if (*(first + rchild) < *(first + rchild - 1)) --rchild;
            // 子节点上移
            *(first + holeIndex) = *(first + rchild);
            // 空洞位置下移
            holeIndex = rchild;
            // 更新右子节点
            rchild = 2 * (rchild + 1);
        }
        // 如果没有右子节点
        if (rchild == len)
        {
            *(first + holeIndex) = *(first + (rchild - 1));
            holeIndex = rchild - 1;
        }
        // 再执行一次上溯(percolate up)过程
        mystl::push_heap_aux(first, holeIndex, topIndex, value);
    }

    template<class RandomIter, class T, class Distance>
    void pop_heap_aux(RandomIter first, RandomIter last, RandomIter result, T value, Distance *) {
        // 先将首值调至尾节点，然后调整[first, last - 1)使之重新成为一个 max-heap
        *result = *first;
        mystl::adjust_heap(first, static_cast<Distance>(0), last - first, value);
    }

    /**
     * @brief 将堆顶元素移至容器末尾，调整剩余元素为新堆
     */
    template<class RandomIter>
    void pop_heap(RandomIter first, RandomIter last) {
        mystl::pop_heap_aux(first, last - 1, last - 1, *(last - 1), distance_type(first));
    }

    /**
     * @brief 将堆顶元素移至容器末尾，调整剩余元素为新堆
     */
    template<class RandomIter, class T, class Distance, class Compared>
    void adjust_heap(RandomIter first, Distance holeIndex, Distance len, T value, Compared comp) {
        // 先进行下溯(percolate down)过程
        auto topIndex = holeIndex;
        auto rchild = 2 * holeIndex + 2;
        while (rchild < len)
        {
            if (comp(*(first + rchild), *(first + rchild - 1))) --rchild;
            *(first + holeIndex) = *(first + rchild);
            holeIndex = rchild;
            rchild = 2 * (rchild + 1);
        }
        if (rchild == len)
        {
            *(first + holeIndex) = *(first + (rchild - 1));
            holeIndex = rchild - 1;
        }
        // 再执行一次上溯(percolate up)过程
        mystl::push_heap_aux(first, holeIndex, topIndex, value, comp);
    }

    template<class RandomIter, class T, class Distance, class Compared>
    void pop_heap_aux(RandomIter first, RandomIter last, RandomIter result, T value, Distance *, Compared comp) {
        *result = *first;// 先将尾指设置成首值，即尾指为欲求结果
        mystl::adjust_heap(first, static_cast<Distance>(0), last - first, value, comp);
    }

    /**
     * @brief 将堆顶元素移至容器末尾，调整剩余元素为新堆
     */
    template<class RandomIter, class Compared>
    void pop_heap(RandomIter first, RandomIter last, Compared comp) {
        mystl::pop_heap_aux(first, last - 1, last - 1, *(last - 1), distance_type(first), comp);
    }

    /**
     * @brief 将堆转为有序序列（升序）
     * @note 反复调用 pop_heap 将最大值移至末尾，缩小堆范围，最终得到升序序列
     */
    template<class RandomIter>
    void sort_heap(RandomIter first, RandomIter last) {
        // 每执行一次 pop_heap，最大的元素都被放到尾部，直到容器最多只有一个元素，完成排序
        while (last - first > 1) { mystl::pop_heap(first, last--); }
    }

    /**
     * @brief 将堆转为有序序列（升序）
     * @note 反复调用 pop_heap 将最大值移至末尾，缩小堆范围，最终得到升序序列
     */
    template<class RandomIter, class Compared>
    void sort_heap(RandomIter first, RandomIter last, Compared comp) {
        while (last - first > 1) { mystl::pop_heap(first, last--, comp); }
    }

    /**
     * @brief 将无序序列转为堆
     * @note 从最后一个非叶子节点（索引 (n-2)/2）向前遍历，对每个子树执行 adjust_heap 调整
     */
    template<class RandomIter, class Distance>
    void make_heap_aux(RandomIter first, RandomIter last, Distance *) {
        if (last - first < 2) return;
        auto len = last - first;
        // 从最后一个非叶子节点开始
        auto holeIndex = (len - 2) / 2;
        while (true)
        {
            // 重排以 holeIndex 为首的子树
            mystl::adjust_heap(first, holeIndex, len, *(first + holeIndex));
            if (holeIndex == 0) return;
            holeIndex--;
        }
    }

    /**
     * @brief 将无序序列转为堆
     * @note 从最后一个非叶子节点（索引 (n-2)/2）向前遍历，对每个子树执行 adjust_heap 调整
     */
    template<class RandomIter>
    void make_heap(RandomIter first, RandomIter last) {
        mystl::make_heap_aux(first, last, distance_type(first));
    }

    /**
     * @brief 将无序序列转为堆
     * @note 从最后一个非叶子节点（索引 (n-2)/2）向前遍历，对每个子树执行 adjust_heap 调整
     */
    template<class RandomIter, class Distance, class Compared>
    void make_heap_aux(RandomIter first, RandomIter last, Distance *, Compared comp) {
        if (last - first < 2) return;
        auto len = last - first;
        auto holeIndex = (len - 2) / 2;
        while (true)
        {
            // 重排以 holeIndex 为首的子树
            mystl::adjust_heap(first, holeIndex, len, *(first + holeIndex), comp);
            if (holeIndex == 0) return;
            holeIndex--;
        }
    }

    /**
     * @brief 将无序序列转为堆
     * @note 从最后一个非叶子节点（索引 (n-2)/2）向前遍历，对每个子树执行 adjust_heap 调整
     */
    template<class RandomIter, class Compared>
    void make_heap(RandomIter first, RandomIter last, Compared comp) {
        mystl::make_heap_aux(first, last, distance_type(first), comp);
    }

}// namespace mystl
#endif// !MYTINYSTL_HEAP_ALGO_H_
