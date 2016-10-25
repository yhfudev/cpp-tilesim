/******************************************************************************
 * Name:        src/pfsort.c
 * Purpose:     Sorting algorithms
 * Author:      Yunhui Fu
 * Created:     2009-04-10
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     LGPL licence
 ******************************************************************************/

#include <assert.h>

#include "pfutils.h"
#include "pfsort.h"

/* 折半方式查找记录
 * psl->marr 中指向的数据已经以先小后大方式排好序
 * data_pinpoint: 所要查找的 匹配数据指针
 * ret_idx: 查找到的位置;如果没有找到，则返回如添加该记录其所在的位置。
 * 找到则返回0，否则返回<0
 */
int
pf_bsearch_r (void *userdata, size_t num_data, pf_bsearch_cb_comp_t cb_comp, void *data_pinpoint, size_t *ret_idx)
{
    int retcomp;
    uint8_t flg_found;
    size_t ileft;
    size_t iright;
    size_t i;

    assert (NULL != ret_idx);
    /* 查找合适的位置 */
    if (num_data < 1) {
        *ret_idx = 0;
        return -1;
    }

    /* 折半查找 */
    /* 为了不出现负数，以免缩小索引的所表示的数据范围
     * (负数表明减少一位二进制位的使用)，
     * 内部 ileft 和 iright使用从1开始的下标，
     *   即1表示C语言中的0, 2表示语言中的1，以此类推。
     * 对外还是使用以 0 为开始的下标
     */
    i = 0;
    ileft = 1;
    iright = num_data;
    flg_found = 0;
    for (; ileft <= iright;) {
        i = (ileft + iright) / 2 - 1;
        /* cb_comp should return the *data_pinpoint - *userdata[i] */
        retcomp = cb_comp (userdata, i, data_pinpoint);
        if (retcomp < 0) {
            iright = i;
        } else if (retcomp > 0) {
            ileft = i + 2;
        } else {
            // found
            flg_found = 1;
            break;
        }
    }

    if (flg_found) {
        *ret_idx = i;
        return 0;
    }
    if (iright <= i) {
        *ret_idx = i;
    } else if (ileft >= i + 2) {
        *ret_idx = i + 1;
    }
    return -1;
}

// compare size_t type
int
pf_bsearch_cb_comp_sizet (void *userdata, size_t idx, void * data_pin)
{
    size_t pa = *((size_t *)data_pin);
    size_t pb = *((size_t *)userdata + idx);
    if (pa > pb) {
        return 1;
    } else if (pa < pb) {
        return -1;
    }
    return 0;
}

// compare size_t type
int
pf_sort_cb_comp_sizet (void *userdata, size_t idx1, size_t idx2)
{
    size_t pa = *((size_t *)userdata + idx1);
    size_t pb = *((size_t *)userdata + idx2);
    if (pa > pb) {
        return 1;
    } else if (pa < pb) {
        return -1;
    }
    return 0;
}

int
pf_sort_cb_swap_sizet (void *userdata, size_t idx1, size_t idx2)
{
    size_t *pa = (size_t *)userdata + idx1;
    size_t *pb = (size_t *)userdata + idx2;
    size_t tmp;
    tmp = *pa;
    *pa = *pb;
    *pb = tmp;
    return 0;
}

// size_t idxparent, idxleft, idxcur;
#define FIXHEAP(userdata, cb_comp, cb_swap, start_pos, num_data) \
	{ \
		size_t idxparent0, idxleft0, idxcur0; \
		idxparent0 = start_pos; \
		for (; idxparent0 < num_data; ) { \
			/* compare two children and swap with the larger child */ \
			idxleft0 = idxparent0 * 2 + 1; \
			if (idxleft0 >= num_data) { \
				/* idxparent0 is a leaf */ \
				break; \
			} \
			idxcur0 = idxparent0 * 2 + 2; /* right */ \
			if (idxcur0 < num_data) { \
				if (cb_comp (userdata, idxleft0, idxcur0) > 0) { \
					idxcur0 = idxleft0; \
				} \
			} else { \
				idxcur0 = idxleft0; \
			} \
			if (cb_comp (userdata, idxcur0, idxparent0) > 0) { \
				cb_swap (userdata, idxcur0, idxparent0); \
			} \
			idxparent0 = idxcur0; \
		} \
	}

int
pf_heapsort_r (void *userdata, size_t num_data, pf_sort_cb_comp_t cb_comp, pf_sort_cb_swap_t cb_swap)
{
	size_t idxparent;
	size_t idxleft;
	size_t idxcur;
	assert (NULL != cb_comp);
	assert (NULL != cb_swap);
	if (num_data < 2) {
		return 0;
	}
	// build heap
	for (idxcur = num_data - 1; idxcur > 0; ) {
		idxparent = (idxcur - 1) / 2;
		idxleft = idxparent * 2 + 1;
		assert ((idxcur == idxleft) || (idxcur == idxleft + 1));
		// get the max child
		if (idxcur != idxleft) {
			assert (idxcur == idxleft + 1);
			// compare userdata[idxleft] with userdata[idxcur]
			if (cb_comp (userdata, idxleft, idxcur) > 0) {
				idxcur = idxleft;
			}
		}
		// max heap
		if (cb_comp (userdata, idxcur, idxparent) > 0) {
			cb_swap (userdata, idxcur, idxparent);
			FIXHEAP (userdata, cb_comp, cb_swap, idxcur, num_data);
		}
		assert (idxleft > 0);
		idxcur = idxleft - 1;
	}
	for (; num_data > 0;) {
		// extract max
		cb_swap (userdata, 0, num_data - 1);
		num_data --;
		// fix heap
		FIXHEAP (userdata, cb_comp, cb_swap, 0, num_data);
	}
	// done!
	return 0;
}
