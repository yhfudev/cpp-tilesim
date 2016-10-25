/******************************************************************************
 * Name:        src/pfheapsort.h
 * Purpose:     Sorting algorithms
 * Author:      Yunhui Fu
 * Created:     2009-04-10
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     LGPL licence
 ******************************************************************************/
#ifndef _PF_SORT_H
#define _PF_SORT_H

#include "pfutils.h"
_PSF_BEGIN_EXTERN_C

typedef int (* pf_sort_cb_swap_t)(void *userdata, size_t idx1, size_t idx2); /* userdata[idx1] <=> userdata[idx2] */
typedef int (* pf_sort_cb_comp_t)(void *userdata, size_t idx1, size_t idx2); /*"userdata[idx1] - userdata[idx2]"*/
CODE_SECTION_UTILS int pf_heapsort_r (void *userdata, size_t num_data, pf_sort_cb_comp_t cb_comp, pf_sort_cb_swap_t cb_swap);
CODE_SECTION_UTILS int pf_quicksort_r (void *userdata, size_t num_data, pf_sort_cb_comp_t cb_comp, pf_sort_cb_swap_t cb_swap);
CODE_SECTION_UTILS int pf_sort_cb_comp_sizet (void *userdata, size_t idx1, size_t idx2);
CODE_SECTION_UTILS int pf_sort_cb_swap_sizet (void *userdata, size_t idx1, size_t idx2);

typedef int (* pf_bsearch_cb_comp_t)(void *userdata, size_t idx, void * data_pin); /*"*data_pin - data_list[idx]"*/
CODE_SECTION_UTILS int pf_bsearch_cb_comp_sizet (void *userdata, size_t idx, void * data_pin);
CODE_SECTION_UTILS int pf_bsearch_r (void *userdata, size_t num_data, pf_bsearch_cb_comp_t cb_comp, void *data_pinpoint, size_t *ret_idx);

_PSF_END_EXTERN_C
#endif /*_PF_SORT_H*/
