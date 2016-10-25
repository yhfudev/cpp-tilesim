/******************************************************************************
 * Name:        bintree.h
 * Purpose:     binary tree for encoding of glues
 * Author:      Yunhui Fu
 * Created:     2009-11-01
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/
#ifndef _BINTREE_H
#define _BINTREE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _bintree_node_t {
    struct _bintree_node_t *chd_left;
    struct _bintree_node_t *chd_right;
    void *data;
} bintree_node_t;

typedef void (* bintree_cb_visit_t) (bintree_node_t * root, void *userdata);

bintree_node_t * bintree_create (void);
void bintree_destroy (bintree_node_t * root);

void bintree_traversal_preorder (bintree_node_t * root, bintree_cb_visit_t visit, void *userdata);
void bintree_traversal_postorder (bintree_node_t * root, bintree_cb_visit_t visit, void *userdata);

int bintree_has_nochild (bintree_node_t * root);

void bintree_construct_bincode (bintree_node_t * root, size_t bincode, size_t binsize, char *name);

#ifdef __cplusplus
}
#endif

#endif /* _BINTREE_H */
