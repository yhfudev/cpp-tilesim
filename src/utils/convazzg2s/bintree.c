/******************************************************************************
 * Name:        bintree.c
 * Purpose:     binary tree for encoding of glues
 * Author:      Yunhui Fu
 * Created:     2009-11-01
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "bintree.h"

void
bintree_traversal_preorder (bintree_node_t * root, bintree_cb_visit_t visit, void *userdata)
{
    assert (NULL != visit);
    visit (root, userdata);
    if (root->chd_left) {
        bintree_traversal_preorder (root->chd_left, visit, userdata);
    }
    if (root->chd_right) {
        bintree_traversal_preorder (root->chd_right, visit, userdata);
    }
}

void
bintree_traversal_postorder (bintree_node_t * root, bintree_cb_visit_t visit, void *userdata)
{
    assert (NULL != visit);
    if (root->chd_left) {
        bintree_traversal_postorder (root->chd_left, visit, userdata);
    }
    if (root->chd_right) {
        bintree_traversal_postorder (root->chd_right, visit, userdata);
    }
    visit (root, userdata);
}

bintree_node_t *
bintree_create (void)
{
    bintree_node_t * root;
    root = (bintree_node_t *)malloc (sizeof (*root));
    memset (root, 0, sizeof (*root));
    return root;
}

void
bintree_cb_visit_4destroy (bintree_node_t * root, void *userdata)
{
    free (root);
}

void
bintree_destroy (bintree_node_t * root)
{
    bintree_traversal_postorder (root, bintree_cb_visit_4destroy, NULL);
}

int
bintree_has_nochild (bintree_node_t * root)
{
    if ((NULL == root->chd_left) && (NULL == root->chd_right)) {
        return 1;
    }
    return 0;
}

void
bintree_construct_bincode (bintree_node_t * root, size_t bincode, size_t binsize, char *name)
{
    bintree_node_t *pbtn;
    bintree_node_t *curnode = root;
    size_t i;

    for (i = 0; i < binsize; i ++) {
        if ((bincode >> (binsize - i - 1)) & 0x01) {
            if (NULL != curnode->chd_right) {
                // right child exit, so it don't need creat new one
                curnode = curnode->chd_right;
                continue;
            }
        } else {
            if (NULL != curnode->chd_left) {
                // left child exit, so it don't need creat new one
                curnode = curnode->chd_left;
                continue;
            }
        }
        pbtn = (bintree_node_t *)malloc (sizeof (bintree_node_t));
        memset (pbtn, 0, sizeof (*pbtn));
        pbtn->chd_left = NULL;
        pbtn->chd_right = NULL;
        if (i + 1 >= binsize) {
            pbtn->data = (void *)name;
        }
        if ((bincode >> (binsize - i - 1)) & 0x01) {
            curnode->chd_right = pbtn;
            curnode = curnode->chd_right;
        } else {
            curnode->chd_left = pbtn;
            curnode = curnode->chd_left;
        }
    }
}
