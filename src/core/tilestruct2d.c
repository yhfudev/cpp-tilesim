/******************************************************************************
 * Name:        tilestruct2d.c
 * Purpose:     some core functions for the tiles (2d)
 * Author:      Yunhui Fu
 * Created:     2008-09-18
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2008 Yunhui Fu
 * Licence:     LGPL licence
 ******************************************************************************/

#include <assert.h>
#include <string.h> // memset()
#include <time.h> // time()

#include "bitset.h"
#include "tilestruct2d.h"

#define GLUE_CAN_ADHERE(a,b) ((a) == (b))

#define TSPOS2D_ADD(a,b) \
  do { \
    (a).x += (b).x; \
    (a).y += (b).y; \
  } while (0)

/* Rotate the ractangle clockwise.
  the clockwise and counter-clockwise of the rotation related to the coordinate system.
 */
#define ROTATE_2D(rotnum,Xmax,Ymax,x0,y0) \
  switch (rotnum) { \
  case 0: \
  default: \
      break; \
  case 1: \
  { \
      size_t tmp; \
      tmp = Xmax - 1 - x0; \
      x0 = y0; \
      y0 = tmp; \
  } \
      break; \
  case 2: \
      y0 = Ymax - 1 - y0; \
      x0 = Xmax - 1 - x0; \
      break; \
  case 3: \
  { \
      size_t tmp; \
      tmp = Ymax - 1 - y0; \
      y0 = x0; \
      x0 = tmp; \
  } \
      break; \
  }

//////////////////////////////////////////

// Get the glue value of the designate direction of one tile.
// The tile information contained in tstilecombitem_t include the item idx which
// is the index of a global tile table.
// The parameter rotnum of the function denote that the tile is a part of one
// supertile and the supertile is rotated and then the tile have to rotate
// acording to the rotation.
int
tile_get_glue_2d (tstile_t *ptilevec, size_t maxtvec, tstilecombitem_t *ptsi, size_t dirglue, size_t rotnum)
{
    assert (ptsi->idtile < maxtvec);
    rotnum += ptsi->rotnum;
    rotnum %= 4;
    dirglue %= 4;
    //for (; dirglue < rotnum; dirglue += 4);
    if (dirglue < rotnum) {
        assert (dirglue + 4 >= rotnum);
        dirglue += 4;
    }
    dirglue -= rotnum;
    assert (dirglue >= 0);
    switch (dirglue) {
    case ORI_NORTH:
    case ORI_EAST:
    case ORI_SOUTH:
    case ORI_WEST:
        return ptilevec[ptsi->idtile].glues[dirglue];
    }
    return -1;
}

// make a bitmap of the tc. the bitmap will be rotated by rotnum times.
static void
tstc_set_bitmap_rota (tstilecomb_t *tc, unsigned char *buf, int rotnum)
{
    tstilecombitem_t tci;
    int i;

    rotnum %= 4;

    for (i = 0; i < slist_size (&(tc->tbuf)); i ++) {
        slist_data (&(tc->tbuf), i, &tci);
        ROTATE_2D (rotnum, tc->maxpos.x, tc->maxpos.y, tci.pos.x, tci.pos.y);
        switch (rotnum) {
        case 0:
        case 2:
            BM2D_SET (buf, tc->maxpos.x, tci.pos.x, tci.pos.y);
            break;
        case 1:
        case 3:
            BM2D_SET (buf, tc->maxpos.y, tci.pos.x, tci.pos.y);
            break;
        }
    }
}

// make a bitmap of the tc.
static void
tstc_set_bitmap (tstilecomb_t *tc, unsigned char *buf)
{
    tstilecombitem_t *pci;
    int i;
    for (i = 0; i < slist_size (&(tc->tbuf)); i ++) {
        pci = (tstilecombitem_t *) slist_data_lock (&(tc->tbuf), i);
        BM2D_SET (buf, tc->maxpos.x, pci->pos.x, pci->pos.y);
        slist_data_unlock (&(tc->tbuf), i);
    }
}

// if the two tstilecomb_t is equal, return 1.
// otherwise return 0
int
tstc_is_equal_2d (tstilecomb_t *tc1, tstilecomb_t *tc2, char flg_nonrotatable)
{
    // 循环4个方向做如下步骤：
    // 先检测bitmap是否吻合，如不是则下个循环
    // 先检测各个点和 glue 以及摆放(dir)是否相同，如不是则下个循环
    // 是否循环完都没有找到，如没找到则不相同，返回
    // 到此相同,返回
    int ret = 0;
    unsigned char *buf1 = NULL;
    unsigned char *buf2 = NULL;
    unsigned char *buf1tmp = NULL;
    unsigned char *buf2tmp = NULL;
    tstilecomb_t *tc2rot = NULL;
    tstilecomb_t *tc2rot2 = NULL;
    size_t sz_x;
    size_t sz_y;
    size_t sz_tmp;
    int i;
    int j;
    int maxrot = 4;

    assert (NULL != tc1);
    assert (NULL != tc2);
    if (slist_size (&(tc1->tbuf)) != slist_size (&(tc2->tbuf))) {
        return 0;
    }
    // check if the size of the two tstilecomb_t are the same.
    if (tc1->maxpos.x != tc2->maxpos.x) {
        if (tc1->maxpos.x != tc2->maxpos.y) {
            return 0;
        }
        if (tc1->maxpos.y != tc2->maxpos.x) {
            return 0;
        }
    } else {
        if (tc1->maxpos.y != tc2->maxpos.y) {
            return 0;
        }
    }

    assert (tc1->maxpos.x * tc1->maxpos.y == tc2->maxpos.x * tc2->maxpos.y);

    // set the bitmap of tstilecomb_t
    buf1 = BM2D_CREATE (tc1->maxpos.x, tc1->maxpos.y);
    buf2 = BM2D_CREATE (tc2->maxpos.x, tc2->maxpos.y);
    tstc_set_bitmap (tc1, buf1);
    tstc_set_bitmap (tc2, buf2);
    sz_x = tc2->maxpos.x;
    sz_y = tc2->maxpos.y;
    buf1tmp = buf1;

    // The tc2 is rotated four times to compair with the tc1
    maxrot = 4;
    if (flg_nonrotatable) {
        maxrot = 1;
    }
    for (i = 0; i < maxrot; i ++) {
        if (sz_x != tc1->maxpos.x) {
            continue;
        }
        if (sz_y != tc1->maxpos.y) {
            continue;
        }
        if (0 == bm_cmp (buf1tmp, buf2, tc1->maxpos.x * tc1->maxpos.y)) {
            // the shape is the same. and
            // check if the tile and its rotnum and glue value is the same.
            if (0 == i) {
                tc2rot2 = tc2; // FIXME: how to release the source?
            } else {
                tsposition_t tmppos;
                memset (&tmppos, 0, sizeof (tsposition_t));
                tc2rot2 = tstc_rotate_2d (tc2, tc2rot, i, &tmppos);
            }
            assert (slist_size (&(tc1->tbuf)) == slist_size (&(tc2rot2->tbuf)));
            for (j = 0; j < slist_size (&(tc2rot2->tbuf)); j ++) {
                int ret;
                tstilecombitem_t *ptci1;
                tstilecombitem_t *ptci2;
                ptci1 = (tstilecombitem_t *) slist_data_lock (&(tc1->tbuf), j);
                ptci2 = (tstilecombitem_t *) slist_data_lock (&(tc2rot2->tbuf), j);
                ret = tstci_cmp (ptci1, ptci2);
                slist_data_unlock (&(tc1->tbuf), j);
                slist_data_unlock (&(tc2rot2->tbuf), j);
                if (ret != 0) {
                    break;
                }
            }
            if (j >= slist_size (&(tc2rot2->tbuf))) {
                // there is no one item that is not equal.
                // 没有比较到不同的
                ret = 1;
                //break;
                goto end_tstc_isequal;
            }
        }
        if (i > 2) {
            // i = 0, 1, 2, 3
            break;
        }
        buf2tmp = bm2d_rotate (buf2, buf1tmp, sz_x, sz_y, 1);
        if (NULL == buf2tmp) {
            // error
            break;
        }
        buf1tmp = buf2;
        buf2 = buf2tmp;
        sz_tmp = sz_x;
        sz_x = sz_y;
        sz_y = sz_tmp;
    }

end_tstc_isequal:
    BM2D_FREE (buf1tmp);
    BM2D_FREE (buf2);
    return ret;
}

int
tstc_compare_2d (tstilecomb_t *tc1, tstilecomb_t *tc2, char flg_nonrotatable)
{
    size_t i;
    int ret = 0;
    tstilecombitem_t *ptsci1;
    tstilecombitem_t *ptsci2;

    // 首先判断两个 supertile 在可旋转状态下是否相等
    if (tstc_is_equal (tc1, tc2, flg_nonrotatable)) {
        return 0;
    }
    // 然后再根据 (x,y), idtile, rotnun, 来排序
    for (i = 0; i < slist_size (&(tc1->tbuf)); i ++) {
        if (i >= slist_size (&(tc2->tbuf))) {
            return 1;
        }
        ret = 0;
        ptsci1 = (tstilecombitem_t *)slist_data_lock (&(tc1->tbuf), i);
        ptsci2 = (tstilecombitem_t *)slist_data_lock (&(tc2->tbuf), i);
        // first check the x, y, z
        if (ptsci1->pos.x > ptsci2->pos.x) {
            ret = 1;
        } else if (ptsci1->pos.x < ptsci2->pos.x) {
            ret = -1;
        } else {
            if (ptsci1->pos.y > ptsci2->pos.y) {
                ret = 1;
            } else if (ptsci1->pos.y < ptsci2->pos.y) {
                ret = -1;
            } else {
#if USE_THREEDIMENTION
                if (ptsci1->pos.z > ptsci2->pos.z) {
                    ret = 1;
                } else if (ptsci1->pos.z < ptsci2->pos.z) {
                    ret = -1;
                } else {
#endif
                    if (ptsci1->idtile > ptsci2->idtile) {
                        ret = 1;
                    } else if (ptsci1->idtile < ptsci2->idtile) {
                        ret = -1;
                    } else {
                        if (ptsci1->rotnum > ptsci2->rotnum) {
                            ret = 1;
                        } else if (ptsci1->rotnum < ptsci2->rotnum) {
                            ret = -1;
                        } else {
                            ret = 0;
                        }
                    }
#if USE_THREEDIMENTION
                }
#endif
            }
        }
        slist_data_unlock (&(tc1->tbuf), i);
        slist_data_unlock (&(tc2->tbuf), i);
        if (ret != 0) {
            return ret;
        }
    }
    if (i < slist_size (&(tc2->tbuf))) {
        return -1;
    }

    // 最后如果还没有分出大小，则以上算法有问题
    assert (0);
    return 0;
}

/*
   rotate the buf_orig, and translate (x,y) after rotating
   store the result to buf_new
   num: the times for rotating 90 degree
 */
tstilecomb_t *
tstc_rotate_2d (tstilecomb_t *buf_orig, tstilecomb_t * buf_new, int num, tsposition_t *ptrans)
{
    size_t i;
    size_t tmp;
    char flg_newbuf = 0;

    if (NULL == buf_new) {
        buf_new = tstc_create ();
        flg_newbuf = 1;
    }
    if (NULL == buf_new) {
        return NULL;
    }
    tstc_reset (buf_new);
    switch (num % 4) {
    /*
      (the clockwise and counter-clockwise of the rotation related to the coordinate system)
        origin: maxx, maxy; x0, y0
        90: [maxy,maxx]
            X90 = y0
            Y90 = maxx - 1 - x0
        180: [maxx,maxy]
            X180 = maxx - 1 - x0
            Y180 = maxy - 1 - y0
        270: [maxy,maxx]
            X270 = maxy - 1 - y0
            Y270 = x0
    */
    case 0:
        for (i = 0; i < slist_size (&(buf_orig->tbuf)); i ++) {
            tstilecombitem_t tci;
            slist_data (&(buf_orig->tbuf), i, &tci);
            TSPOS2D_ADD (tci.pos, *ptrans);
            slist_store (&(buf_new->tbuf), &tci);
            slist_data_unlock (&(buf_orig->tbuf), i);
            // update maxx and maxy
            if (buf_new->maxpos.x <= tci.pos.x) {
                buf_new->maxpos.x = tci.pos.x + 1;
            }
            if (buf_new->maxpos.y <= tci.pos.y) {
                buf_new->maxpos.y = tci.pos.y + 1;
            }
        }
        assert (buf_new->maxpos.x == buf_orig->maxpos.x + ptrans->x);
        assert (buf_new->maxpos.y == buf_orig->maxpos.y + ptrans->y);
        break;
    case 1: /* 90 */
        for (i = 0; i < slist_size (&(buf_orig->tbuf)); i ++) {
            tstilecombitem_t tci;
            slist_data (&(buf_orig->tbuf), i, &tci);
            tmp = ptrans->y + (buf_orig->maxpos.x - 1 - tci.pos.x);
            tci.pos.x = ptrans->x + tci.pos.y;
            tci.pos.y = tmp;
            tci.rotnum += 1; // also rotate the tile dir
            tci.rotnum %= 4;
            slist_store (&(buf_new->tbuf), &tci);
            // update maxx and maxy
            if (buf_new->maxpos.x <= tci.pos.x) {
                buf_new->maxpos.x = tci.pos.x + 1;
            }
            if (buf_new->maxpos.y <= tci.pos.y) {
                buf_new->maxpos.y = tci.pos.y + 1;
            }
        }
        assert (buf_new->maxpos.x == buf_orig->maxpos.y + ptrans->x);
        assert (buf_new->maxpos.y == buf_orig->maxpos.x + ptrans->y);
        break;
    case 2: /* 180 */
        for (i = 0; i < slist_size (&(buf_orig->tbuf)); i ++) {
            tstilecombitem_t tci;
            slist_data (&(buf_orig->tbuf), i, &tci);
            tci.pos.x = buf_orig->maxpos.x - 1 - tci.pos.x;
            tci.pos.y = buf_orig->maxpos.y - 1 - tci.pos.y;
            tci.pos.x += ptrans->x;
            tci.pos.y += ptrans->y;
            tci.rotnum += 2;
            tci.rotnum %= 4;
            slist_store (&(buf_new->tbuf), &tci);
            // update maxx and maxy
            if (buf_new->maxpos.x <= tci.pos.x) {
                buf_new->maxpos.x = tci.pos.x + 1;
            }
            if (buf_new->maxpos.y <= tci.pos.y) {
                buf_new->maxpos.y = tci.pos.y + 1;
            }
        }
        assert (buf_new->maxpos.x == buf_orig->maxpos.x + ptrans->x);
        assert (buf_new->maxpos.y == buf_orig->maxpos.y + ptrans->y);
        break;
    case 3: /* 270 */
        for (i = 0; i < slist_size (&(buf_orig->tbuf)); i ++) {
            tstilecombitem_t tci;
            slist_data (&(buf_orig->tbuf), i, &tci);
            tmp = ptrans->y + tci.pos.x;
            tci.pos.x = ptrans->x + (buf_orig->maxpos.y - 1 - tci.pos.y);
            tci.pos.y = tmp;
            tci.rotnum += 3;
            tci.rotnum %= 4;
            slist_store (&(buf_new->tbuf), &tci);
            slist_data_unlock (&(buf_orig->tbuf), i);
            // update maxx and maxy
            if (buf_new->maxpos.x <= tci.pos.x) {
                buf_new->maxpos.x = tci.pos.x + 1;
            }
            if (buf_new->maxpos.y <= tci.pos.y) {
                buf_new->maxpos.y = tci.pos.y + 1;
            }
        }
        assert (buf_new->maxpos.x == buf_orig->maxpos.y + ptrans->x);
        assert (buf_new->maxpos.y == buf_orig->maxpos.x + ptrans->y);

        break;
    default:
        if (flg_newbuf) {
            tstc_destroy (buf_new);
        }
        buf_new = NULL;
        break;
    }
    return buf_new;
}


#if DEBUG
// check the supertile to see if there's any errors.
static int
tstc_chkassert_2d (tstilecomb_t *ptc)
{
    int i;

    tsposition_t posmin;
    tsposition_t posmax;
    tstilecombitem_t *pcuritem;

    assert (NULL != ptc);
    assert (slist_size (&(ptc->tbuf)) > 0);

    // find the left botton corner and top right corner.
    i = 0;
    pcuritem = (tstilecombitem_t *) slist_data_lock (&(ptc->tbuf), i);
    memmove (&posmin, &(pcuritem->pos), sizeof (posmin));
    memmove (&posmax, &(pcuritem->pos), sizeof (posmax));

    slist_data_unlock (&(ptc->tbuf), i);

    for (i = 1; i < slist_size (&(ptc->tbuf)); i ++) {
        pcuritem = (tstilecombitem_t *) slist_data_lock (&(ptc->tbuf), i);
        assert (NULL != pcuritem);

        if (posmin.x > pcuritem->pos.x) posmin.x = pcuritem->pos.x;
        if (posmin.y > pcuritem->pos.y) posmin.x = pcuritem->pos.y;
        if (posmax.y < pcuritem->pos.y) posmax.y = pcuritem->pos.y;
        if (posmax.x < pcuritem->pos.x) posmax.x = pcuritem->pos.x;
        if (posmax.y < pcuritem->pos.y) posmax.y = pcuritem->pos.y;
        slist_data_unlock (&(ptc->tbuf), i);
    }
    assert (posmin.x == 0);
    assert (posmin.y == 0);
    assert (ptc->maxpos.x == posmax.x + 1);
    assert (ptc->maxpos.y == posmax.y + 1);
    return 0;
}
#else
#define tstc_chkassert_2d(p) (0)
#endif

// nomalize the ptc: let the left bottom of the supertile segment move to the (0,0) position
int
tstc_nomalize_2d (tstilecomb_t *ptc)
{
    int i;

    tsposition_t posmin;
    tsposition_t posmax;
    tstilecombitem_t *pcuritem;

    assert (NULL != ptc);
    assert (slist_size (&(ptc->tbuf)) > 0);

    // find the left botton corner and top right corner.
    i = 0;
    pcuritem = (tstilecombitem_t *) slist_data_lock (&(ptc->tbuf), i);
    memmove (&posmin, &(pcuritem->pos), sizeof (posmin));
    memmove (&posmax, &(pcuritem->pos), sizeof (posmax));

    slist_data_unlock (&(ptc->tbuf), i);

    for (i = 1; i < slist_size (&(ptc->tbuf)); i ++) {
        pcuritem = (tstilecombitem_t *) slist_data_lock (&(ptc->tbuf), i);
        assert (NULL != pcuritem);

        if (posmin.x > pcuritem->pos.x) posmin.x = pcuritem->pos.x;
        if (posmin.y > pcuritem->pos.y) posmin.y = pcuritem->pos.y;
        if (posmax.x < pcuritem->pos.x) posmax.x = pcuritem->pos.x;
        if (posmax.y < pcuritem->pos.y) posmax.y = pcuritem->pos.y;
        slist_data_unlock (&(ptc->tbuf), i);
    }
    //assert (ptc->posmax.x == posmax.x + 1);
    //assert (ptc->posmax.y == posmax.y + 1);
    if ((posmin.x != 0)
      || (posmin.y != 0)
      ) {
        for (i = 0; i < slist_size (&(ptc->tbuf)); i ++) {
            pcuritem = (tstilecombitem_t *) slist_data_lock (&(ptc->tbuf), i);
            assert (NULL != pcuritem);

            pcuritem->pos.x -= posmin.x;
            pcuritem->pos.y -= posmin.y;
            slist_data_unlock (&(ptc->tbuf), i);
        }
    }
    ptc->maxpos.x = posmax.x - posmin.x + 1;
    ptc->maxpos.y = posmax.y - posmin.y + 1;
    return 0;
}

/*
坐标：

^
#
#
* maxy     +----------+
#          |          |
#          |   base   |
#          |          |
+----------+----------+
#          |
#   test   |
#          |
+==========+==========*==>
                      ^ maxx=(base->maxx + test->maxx)

*/

/* merge tc_base with tc_test which is rotated by dir and is at the position (x,y) adopt the slidetest coordinate system,
   the result is stored in tc_result */
/* 在(x,y)点处添加test, 并且 */
int
tstc_merge_2d (tstilecomb_t *tc_base, tstilecomb_t *tc_test,
    char dir, tsposition_t *ppos, tstilecomb_t *tc_result)
{
    // 首先检测使用坐标偏移(x,y)是否会导致tc_add上的tile覆盖tc_base上的tile.
    // 将tc_add上各个tile加上 x,y 后添加到 tc_base 中

    tsposition_t incbase; // the shift of the base coordinate
    tsposition_t inctest; // the shift of the test coordinate
    tsposition_t maxtest; // the size of the test after rotation
    size_t i; // supertile 的内部 tile 索引
    tstilecombitem_t stitem; // 临时存储的一个tile的信息

    assert (NULL != ppos);
    assert (tc_base != tc_result);
    assert (tc_test != tc_result);

    if (dir % 2 == 0) {
        maxtest.x = tc_test->maxpos.x;
        maxtest.y = tc_test->maxpos.y;
    } else {
        maxtest.x = tc_test->maxpos.y;
        maxtest.y = tc_test->maxpos.x;
    }
    // 因为结合后，最小位置的起始点究竟在哪个supertile上是根据其是否于(0,0)更近来确定的.
    if (ppos->x <= maxtest.x) {
        inctest.x = 0;
        incbase.x = maxtest.x - ppos->x;
    } else {
        inctest.x = ppos->x - maxtest.x;
        incbase.x = 0;
    }
    if (ppos->y <= maxtest.y) {
        inctest.y = 0;
        incbase.y = maxtest.y - ppos->y;
    } else {
        inctest.y = ppos->y - maxtest.y;
        incbase.y = 0;
    }
    // 数量少的一方用作插入
    //if (slist_size (&(tc_base->tbuf)) < slist_size (&(tc_test->tbuf))) {
        tstc_rotate_2d (tc_test, tc_result, dir, &inctest);
        for (i = 0; i < slist_size (&(tc_base->tbuf)); i ++) {
            slist_data (&(tc_base->tbuf), i, &stitem);
            TSPOS2D_ADD(stitem.pos, incbase);
            if (slist_store (&(tc_result->tbuf), &stitem) < 0) {
                return -1;//break;
            }
            if (stitem.pos.x >= tc_result->maxpos.x) {
                tc_result->maxpos.x = stitem.pos.x + 1;
            }
            if (stitem.pos.y >= tc_result->maxpos.y) {
                tc_result->maxpos.y = stitem.pos.y + 1;
            }
        }
    //} else {}
    return 0;
}

////////////////////////////
// 计算坐标的辅助函数

typedef struct _tsrange_t {
    size_t maxtest;  // test 的长度
    size_t maxbase;  // base 的长度
    size_t posx;     // test 左上角第一个 tile(有可能并不存在) 的位置
    size_t rang_start; // 范围最小值
    size_t rang_num;   // 范围个数
} tsrange_t;

// 示例
// tsrange_t tsrgabs_x;
// tsrange_t tsrgabs_y;
// memset (&tsrgabs_x, 0, sizeof (tsrgabs_x));
// memset (&tsrgabs_y, 0, sizeof (tsrgabs_y));
// tsrgabs_x.maxtest = tc_test->maxx;
// tsrgabs_x.maxbase = tc_base->maxx;
// tsrgabs_x.posx = x;
// ts_absolute_range_overlap (&tsrgabs_x);
// tsrgabs_y.maxtest = tc_test->maxy;
// tsrgabs_y.maxbase = tc_base->maxy;
// tsrgabs_y.posx = y;
// ts_absolute_range_overlap (&tsrgabs_y);
// for (i = 0; i < tsrgabs_y->rang_num; i ++) {
//   for (j = 0; j < tsrgabs_x->rang_num; j ++) {
//     // find the tile of the Base at 4 dir of one tile of the Test
//     i + tsrgabs_y->rang_start;
//     j + tsrgabs_x->rang_start;
//   }
// }
//

// 在slidetest中当固定的base和在某一个位置停留的test在做相遇检查时其相互覆盖的区域
// 输入当前MaxBase,MaxTest,x0，求出绝对位置重合范围
static int
ts_absolute_range_overlap (tsrange_t *ptsrg)
{
    assert (ptsrg != NULL);
    if ((ptsrg->posx < 1) || (ptsrg->posx >= ptsrg->maxtest + ptsrg->maxbase)) {
        // 因为初始时两个 Test 和 Base 是在x,y轴上均没有覆盖地紧密相连, 所以只有posx从1开始时才有覆盖
        // 在最后的一个tile覆盖位置为 MaxTest + MaxBase - 1
        //assert (0);
        ptsrg->rang_num = 0;
        return -1;
    }
    if (ptsrg->maxtest > ptsrg->maxbase) {
        if (ptsrg->posx < ptsrg->maxbase) {
            // (0 <= ptsrg->posx) && (ptsrg->posx < ptsrg->maxbase)
            ptsrg->rang_start = ptsrg->maxtest;
            ptsrg->rang_num = ptsrg->posx;
        } else if (ptsrg->posx < ptsrg->maxtest) {
            // (ptsrg->maxbase <= ptsrg->posx) && (ptsrg->posx < ptsrg->maxtest)
            ptsrg->rang_start = ptsrg->maxtest;
            ptsrg->rang_num = ptsrg->maxbase;
        } else {
            // (ptsrg->maxtest <= ptsrg->posx) && (ptsrg->posx < ptsrg->maxtest + ptsrg->maxbase)
            ptsrg->rang_start = ptsrg->posx;
            ptsrg->rang_num = ptsrg->maxtest + ptsrg->maxbase - ptsrg->posx;
        }
    } else {
        // ptsrg->maxtest <= ptsrg->maxbase
        if (ptsrg->posx < ptsrg->maxtest) {
            // (0 <= ptsrg->posx) && (ptsrg->posx < ptsrg->maxtest)
            ptsrg->rang_start = ptsrg->maxtest;
            ptsrg->rang_num = ptsrg->posx;
        } else if (ptsrg->posx < ptsrg->maxbase) {
            // (ptsrg->maxtest <= ptsrg->posx) && (ptsrg->posx < ptsrg->maxbase)
            ptsrg->rang_start = ptsrg->posx;
            ptsrg->rang_num = ptsrg->maxtest;
        } else {
            // (ptsrg->maxbase <= ptsrg->posx) && (ptsrg->posx < ptsrg->maxtest + ptsrg->maxbase)
            ptsrg->rang_start = ptsrg->posx;
            ptsrg->rang_num = ptsrg->maxtest + ptsrg->maxbase - ptsrg->posx;
        }
    }
    return 0;
}

// 在slidetest中当固定的base和在某一个位置停留的test在做相遇检查时需要的测试范围
// 包括了 Test 外一圈的tile
static int
ts_absolute_range_stest (tsrange_t *ptsrg)
{
    assert (ptsrg != NULL);
    if (ptsrg->posx > ptsrg->maxtest + ptsrg->maxbase) {
        // 因为初始时两个 Test 和 Base 是在x,y轴上均没有覆盖地紧密相连, 所以只有posx从1开始时才有覆盖
        // 在最后的一个tile覆盖位置为 MaxTest + MaxBase - 1
        //assert (0);
        ptsrg->rang_num = 0;
        return -1;
    }
    if (ptsrg->maxtest > ptsrg->maxbase) {
        if (ptsrg->posx <= ptsrg->maxbase) {
            // (0 <= ptsrg->posx) && (ptsrg->posx < ptsrg->maxbase)
            ptsrg->rang_start = ptsrg->maxtest - 1;
            ptsrg->rang_num = ptsrg->posx + 1;
        } else if (ptsrg->posx < ptsrg->maxtest) {
            // (ptsrg->maxbase <= ptsrg->posx) && (ptsrg->posx < ptsrg->maxtest)
            ptsrg->rang_start = ptsrg->maxtest - 1;
            ptsrg->rang_num = ptsrg->maxbase + 2;
        } else {
            // (ptsrg->maxtest <= ptsrg->posx) && (ptsrg->posx < ptsrg->maxtest + ptsrg->maxbase)
            ptsrg->rang_start = ptsrg->posx;
            ptsrg->rang_num = ptsrg->maxtest + ptsrg->maxbase - ptsrg->posx + 1;
        }
    } else {
        // ptsrg->maxtest <= ptsrg->maxbase
        if (ptsrg->posx < ptsrg->maxtest) {
            // (0 <= ptsrg->posx) && (ptsrg->posx < ptsrg->maxtest)
            ptsrg->rang_start = ptsrg->maxtest - 1;
            ptsrg->rang_num = ptsrg->posx + 1;
        } else if (ptsrg->posx <= ptsrg->maxbase) {
            // (ptsrg->maxtest <= ptsrg->posx) && (ptsrg->posx < ptsrg->maxbase)
            ptsrg->rang_start = ptsrg->posx;
            ptsrg->rang_num = ptsrg->maxtest;
        } else {
            // (ptsrg->maxbase <= ptsrg->posx) && (ptsrg->posx < ptsrg->maxtest + ptsrg->maxbase)
            ptsrg->rang_start = ptsrg->posx;
            ptsrg->rang_num = ptsrg->maxtest + ptsrg->maxbase - ptsrg->posx + 1;
        }
    }
    return 0;
}

// (x,y) 是否在指定的范围内
#define RNG2D_INRANGE(x,y,startx,numx,starty,numy) \
((((startx) <= (x)) && ((x) < (startx) + (numx))) && (((starty) <= (y)) && ((y) < (starty) + (numy))))

////////////////////////////

/*
  get the neighbor position of the side of tile in the supertile
  the value related to the coordinate system be selected.
*/
static int
set_dir_pos (int dir, tsposition_t *ptp)
{
    assert (NULL != ptp);
    dir = dir % 6;
    switch (dir) {
    case ORI_NORTH:
        ptp->y ++;
        break;
    case ORI_EAST:
        ptp->x ++;
        break;
    case ORI_SOUTH:
        if (ptp->y < 1) {
            return -1;
        }
        ptp->y --;
        break;
    case ORI_WEST:
        if (ptp->x < 1) {
            return -1;
        }
        ptp->x --;
        break;
    }
    return 0;
}

// 索引为当前滑动方向，结果为测试方向顺序
static char g_dir_stestofslide[4][4] = {
    /* 垂直方向,  右手(前进)方向, 后退方向, 垂直反方向 */
    {ORI_EAST,  ORI_NORTH, ORI_SOUTH, ORI_WEST},  /* ORI_NORTH */
    {ORI_NORTH, ORI_EAST,  ORI_WEST,  ORI_SOUTH}, /* ORI_EAST */
    {ORI_WEST,  ORI_SOUTH, ORI_NORTH, ORI_EAST},  /* ORI_SOUTH */
    {ORI_SOUTH, ORI_WEST,  ORI_EAST,  ORI_NORTH}, /* ORI_WEST */
};

/*!!! 更换坐标系（坐标方向）时需要修改本数据 */
// 求滑动方向的垂直方向
static char g_dir_plumbofslide[4] = {
    ORI_EAST,  /* ORI_NORTH */
    ORI_NORTH, /* ORI_EAST */
    ORI_WEST,  /* ORI_SOUTH */
    ORI_SOUTH, /* ORI_WEST */
};

// 各个方向的反方向
static char g_dir_opposite[4] = {
    ORI_SOUTH, /* ORI_NORTH */
    ORI_WEST,  /* ORI_EAST */
    ORI_NORTH, /* ORI_SOUTH */
    ORI_EAST,  /* ORI_WEST */
};

// 从绝对位置求Test的内部相对坐标
// int POS_ABS2REL_TEST(int pos_abstract, tsrange_t *ptr);
// 从绝对位置求Base的内部相对坐标
// int POS_ABS2REL_BASE(int pos_abstract, tsrange_t *ptr);
#define POS_ABS2REL_TEST(pos,ptr) ((pos)-((ptr)->posx))
#define POS_ABS2REL_BASE(pos,ptr) ((pos)-((ptr)->maxtest))

typedef struct _tsslidetestinfo_t {
    tssiminfo_t *psim;     /* IN; psim 的第1部分被用到 */

    tstilecomb_t *tc_base; /* IN; Base的数据 */
    tstilecomb_t *tc_test; /* IN; Test的数据 */

    int rotnum;            /* IN; tc_test 被顺时针旋转的次数 */
    char dir_cur_slide;    /* IN; 当前滑动的方向 */
    stk_t *pstk_local;     /* IN/USE; 待检测的位置 */

    // the next items are handled by the tsstinfo_init()/tsstinfo_clear()
    // etc. malloc and free the buffer by the tsstinfo_init()/tsstinfo_clear()
    unsigned char *bmbuf_base;    /* IN; tc_base 缩略图 */
    unsigned char *bmbuf_test;    /* IN; tc_test 缩略图 */
    unsigned char *bmbuf_checked_global; /* IN/UPDATE; 检查哪些位置被递归测试过,全局 */
    unsigned char *bmbuf_checked_local; /* USE; 检查哪些位置被测试过,在某个posx下 */
    tsrange_t tsrgabs_x; /* USE; Base x方向的范围，在函数调用时不用每次都设置maxx等参数 */
    tsrange_t tsrgabs_y; /* USE; Base y方向的范围，在函数调用时不用每次都设置maxy等参数 */
    tsrange_t tsrgstst_x; /* USE; Test x方向的范围，在函数调用时不用每次都设置maxx等参数 */
    tsrange_t tsrgstst_y; /* USE; Test y方向的范围，在函数调用时不用每次都设置maxy等参数 */
} tsslidetestinfo_t;

static int
tsstinfo_init (tsslidetestinfo_t *psti, tstilecomb_t *tc_base, tstilecomb_t *tc_test, int rotnum)
{
    memset (psti, 0, sizeof (*psti));

    psti->rotnum  = rotnum;
    psti->tc_test = tc_test;
    psti->tc_base = tc_base;

    //memset (&(psti->tsrgabs_x), 0, sizeof (psti->tsrgabs_x));
    //memset (&(psti->tsrgabs_y), 0, sizeof (psti->tsrgabs_y));

    // 注意，只有在 tsrgabs_x/tsrgabs_y 中的 maxtest才是当前test的该方向上的最大值,因为此时test被旋转了
    // 而base不被旋转，所以其最大值可以在任何有 maxbase 的地方获取
    psti->tsrgabs_x.maxbase = tc_base->maxpos.x;
    psti->tsrgabs_y.maxbase = tc_base->maxpos.y;
    psti->tsrgabs_x.maxtest = tc_test->maxpos.x;
    psti->tsrgabs_y.maxtest = tc_test->maxpos.y;
    if (rotnum % 2 != 0) {
        psti->tsrgabs_x.maxtest = tc_test->maxpos.y;
        psti->tsrgabs_y.maxtest = tc_test->maxpos.x;
    }

    //memset (&(psti->tsrgstst_x), 0, sizeof (psti->tsrgstst_x));
    //memset (&(psti->tsrgstst_y), 0, sizeof (psti->tsrgstst_y));
    psti->tsrgstst_x.maxbase = tc_base->maxpos.x;
    psti->tsrgstst_y.maxbase = tc_base->maxpos.y;
    psti->tsrgstst_x.maxtest = tc_test->maxpos.x;
    psti->tsrgstst_y.maxtest = tc_test->maxpos.y;
    if (rotnum % 2 != 0) {
        psti->tsrgstst_x.maxtest = tc_test->maxpos.y;
        psti->tsrgstst_y.maxtest = tc_test->maxpos.x;
    }

    // bmbuf_checked_local 的范围应该是 test 围绕base 时其自身大小所扫过的范围
    psti->bmbuf_checked_local  = BM2D_CREATE ((psti->tsrgabs_x.maxbase + psti->tsrgabs_x.maxtest + psti->tsrgabs_x.maxtest + 2), (psti->tsrgabs_y.maxbase + psti->tsrgabs_y.maxtest + psti->tsrgabs_y.maxtest + 2));
    // bmbuf_checked_global 的范围是 test 的移动范围，即其左上顶点在作探测时的范围
    psti->bmbuf_checked_global = BM2D_CREATE ((psti->tsrgabs_x.maxbase + psti->tsrgabs_x.maxtest + 2), (psti->tsrgabs_y.maxbase + psti->tsrgabs_y.maxtest + 2));

    psti->bmbuf_base = BM2D_CREATE (psti->tsrgabs_x.maxbase, psti->tsrgabs_y.maxbase);
    psti->bmbuf_test = BM2D_CREATE (psti->tsrgabs_x.maxtest, psti->tsrgabs_y.maxtest);
    tstc_set_bitmap (tc_base, psti->bmbuf_base);
    tstc_set_bitmap_rota (tc_test, psti->bmbuf_test, rotnum);

    return 0;
}

static void
tsstinfo_clear (tsslidetestinfo_t *psti)
{
    BM2D_FREE (psti->bmbuf_checked_global);
    BM2D_FREE (psti->bmbuf_checked_local);
    BM2D_FREE (psti->bmbuf_base);
    BM2D_FREE (psti->bmbuf_test);
}

// 测试 test 和 base 在当前位置是否能够结合
// 通过计算各个 test tile 的四周对应 base tile 之间的glue 及其 强度和 是否大于当前温度来确定
// 由 tstc_plumb_test() 或 tstc_mesh_test_godhand() 调用
static int
tstc_plumb_position_test (tsslidetestinfo_t *psti, tsposition_t *ppos, memarr_t *plist_points_ok, size_t cnt_resi_dir[4])
{
    size_t i,j,k;
    size_t cnt_glue; // 某个位置上的 glue 的总计；
    tsposition_t tspos_base; // 临时，base的某个点

    cnt_glue = 0;

    assert (NULL != ppos);
    // 用作检测Test测试范围的参数
    psti->tsrgstst_x.posx = ppos->x;
    psti->tsrgstst_y.posx = ppos->y;
    ts_absolute_range_stest (&(psti->tsrgstst_x));
    ts_absolute_range_stest (&(psti->tsrgstst_y));

    // 用作检测Base范围的参数
    psti->tsrgabs_x.posx = ppos->x;
    psti->tsrgabs_y.posx = ppos->y;
    ts_absolute_range_overlap (&(psti->tsrgabs_x));
    ts_absolute_range_overlap (&(psti->tsrgabs_y));

#if USE_PRESENTATION
    if (psti->psim->cb_initadheredect) {
        tstilecombitem_t info_test; // 当前 test 状态，rotnum 表示 test 被旋转的次数，x,y,z 表示其位置
        info_test.idtile = 0; /*NO USED*/
        info_test.rotnum = psti->rotnum;
        info_test.pos.x = ppos->x;
        info_test.pos.y = ppos->y;

        psti->psim->cb_initadheredect (psti->psim->userdata, psti->tc_base, psti->tc_test, psti->psim->temperature, &info_test);
    }
#endif
    // 对每个需要测试的Test supertile 上的每个 tile 的四方向上作检查
    for (i = 0; i < psti->tsrgstst_y.rang_num; i ++) {
        for (j = 0; j < psti->tsrgstst_x.rang_num; j ++) {
            // find the tile of the Base at 4 dir of one tile of the Test
            // i + tsrgstst_y->rang_start;
            // j + tsrgstst_x->rang_start;
            int ret;

            // 当前
            tspos_base.x = j + psti->tsrgstst_x.rang_start;
            tspos_base.y = i + psti->tsrgstst_y.rang_start;
            tspos_base.x = POS_ABS2REL_TEST(tspos_base.x, &(psti->tsrgabs_x));
            tspos_base.y = POS_ABS2REL_TEST(tspos_base.y, &(psti->tsrgabs_y));
            // Test 上是否有tile
            ret = BM2D_IS_SET (psti->bmbuf_test, psti->tsrgabs_x.maxtest, tspos_base.x, tspos_base.y);
            if (0 == ret) {
                continue;
            }

            for (k = 0; k < 4; k ++) {
                // 对每个方向都检测是否有Base的tile
                int glue_base;
                int glue_test;
                size_t idx_tile_base;
                size_t idx_tile_test;
                tstilecombitem_t tsipin;
                tstilecombitem_t *ptsi_test;
                tstilecombitem_t *ptsi_base;
                //memset (&tsipin, 0, sizeof (tsipin));

                // 获取一个方向上邻接的Base的Tile的坐标
                tspos_base.x = j + psti->tsrgstst_x.rang_start;
                tspos_base.y = i + psti->tsrgstst_y.rang_start;
                if (set_dir_pos ((int)(g_dir_stestofslide[(int)psti->dir_cur_slide][k]), &tspos_base) < 0) {
                    // 坐标超出范围(x,y其中一项<0)
                    continue;
                }

                //RNG2D_INRANGE(x,y,startx,numx,starty,numy)
                // 邻接当前位置的该方向上的点是否在Base矩阵上
                ret = RNG2D_INRANGE (tspos_base.x, tspos_base.y, psti->tsrgabs_x.maxtest, psti->tsrgabs_x.maxbase, psti->tsrgabs_y.maxtest, psti->tsrgabs_y.maxbase);
                if (ret == 0) {
                    continue;
                }
                // 该绝对位置处于Base的范围

                // 看Base上该处是否存在tile
                memset (&tsipin, 0, sizeof (tsipin));
                tsipin.idtile = 0;
                tsipin.rotnum = 0;
                tsipin.pos.x = POS_ABS2REL_BASE(tspos_base.x, &(psti->tsrgabs_x));
                tsipin.pos.y = POS_ABS2REL_BASE(tspos_base.y, &(psti->tsrgabs_y));

                ret = BM2D_IS_SET (psti->bmbuf_base, psti->tc_base->maxpos.x, tsipin.pos.x, tsipin.pos.y);
                if (0 == ret) {
                    continue;
                }
                // test the glue:
                //   get the info from the list

                // base position
                if (slist_find (&(psti->tc_base->tbuf), &tsipin, NULL, &idx_tile_base) < 0) {
                    assert (0);
                }
                //slist_data (&(tc_add->tbuf), idx_tile, &tci);
                ptsi_base = (tstilecombitem_t *) slist_data_lock (&(psti->tc_base->tbuf), idx_tile_base);

                // test position
                tsipin.pos.x = POS_ABS2REL_TEST(j + psti->tsrgstst_x.rang_start, &(psti->tsrgabs_x));
                tsipin.pos.y = POS_ABS2REL_TEST(i + psti->tsrgstst_y.rang_start, &(psti->tsrgabs_y));
                // convert the rotated coordinate to the origin coordinate of the tc_test
                ROTATE_2D (4 - psti->rotnum, psti->tsrgstst_x.maxtest, psti->tsrgstst_y.maxtest, tsipin.pos.x, tsipin.pos.y);
                if (slist_find (&(psti->tc_test->tbuf), &tsipin, NULL, &idx_tile_test) < 0) {
                    assert (0);
                }
                ptsi_test = (tstilecombitem_t *) slist_data_lock (&(psti->tc_test->tbuf), idx_tile_test);

                assert (NULL != ptsi_base);
                assert (NULL != ptsi_test);
                assert (NULL != psti->psim);

                // to check if the glues can be stick with each other?

                // calculate the rotated direction of the glue of the tc_test
                glue_test = tile_get_glue_2d (psti->psim->ptilevec, psti->psim->num_tilevec, ptsi_test, (int)(g_dir_stestofslide[(int)psti->dir_cur_slide][k]), psti->rotnum);
                glue_base = tile_get_glue_2d (psti->psim->ptilevec, psti->psim->num_tilevec, ptsi_base, (int)(g_dir_opposite[(int)(g_dir_stestofslide[(int)psti->dir_cur_slide][k])]), 0);

                slist_data_unlock (&(psti->tc_base->tbuf), idx_tile_base);
                slist_data_unlock (&(psti->tc_test->tbuf), idx_tile_test);

#if USE_PRESENTATION
                tsipin.rotnum = (int)(g_dir_stestofslide[(int)psti->dir_cur_slide][k]);
                tsipin.pos.x = j + psti->tsrgstst_x.rang_start;
                tsipin.pos.y = i + psti->tsrgstst_y.rang_start;
#endif

                if (GLUE_CAN_ADHERE (glue_test, glue_base)) {
                    //assert (glue_test < psti->num_gluevec);
                    cnt_glue += psti->psim->pgluevec[glue_test];
#if USE_PRESENTATION
                    if (NULL != psti->psim->cb_adherepos) {
                        psti->psim->cb_adherepos (psti->psim->userdata, 1/*true*/,  &tsipin);
                    }
                } else {
                    if (NULL != psti->psim->cb_adherepos) {
                        psti->psim->cb_adherepos (psti->psim->userdata, 0/*false*/, &tsipin);
                    }
#endif
                }

                // calculate the abuted tile of Base
                cnt_resi_dir[k] ++;
                // ...
                // 判断Test与Base的距离不要超过1格
            }
        }
    }
#if USE_PRESENTATION
    if (NULL != psti->psim->cb_clearadheredect) {
        psti->psim->cb_clearadheredect (psti->psim->userdata);
    }
#endif
    if (cnt_glue >= psti->psim->temperature) {
        // save the position
        tsstpos_t stpos;
        stpos.rotnum = psti->rotnum;
        memmove (&(stpos.pos), ppos, sizeof (stpos.pos));
#if USE_THREEDIMENTION
        stpos.pos.z = 1;
#endif
        ma_insert (plist_points_ok, ma_size(plist_points_ok), &stpos);
    }
    return 0;
}

// 递归查看从一个位置开始，看能否粘合两个supertile
// plist_points_ok: tsstpos_t 类型的列表
// 注意psti->tc_test 是原始没有根据 psti->rotnum 转置的，需要在使用中转换坐标。
static void
tstc_plumb_test (tsslidetestinfo_t *psti, memarr_t *plist_points_ok)
{
    size_t i,j,k;
    size_t cnt_glue; // 某个位置上的 glue 的总计；
    size_t cnt_resi_dir[4]; // 四个方向上遇到的Base的tile的个数
    tsposition_t tspos_popup; // 从栈中弹出的一个点的坐标
    tsposition_t tspos_base; // 临时，base的某个点

    // bmbuf_checked_local 保存垂直探测时那些不需要被邻接探测的点，如已经被压入栈待检测的点和被忽略的点（如不处于Base上）
    BM2D_RESET (psti->bmbuf_checked_local,
      (psti->tc_base->maxpos.x + psti->tsrgabs_x.maxtest + psti->tsrgabs_x.maxtest + 2),
      (psti->tc_base->maxpos.y + psti->tsrgabs_y.maxtest + psti->tsrgabs_y.maxtest + 2));

    for (; st_size (psti->pstk_local) > 0; ) {
        st_pop (psti->pstk_local, &tspos_popup);

        if (BM2D_IS_SET (psti->bmbuf_checked_global, (psti->tc_base->maxpos.x + psti->tsrgabs_x.maxtest + 2), tspos_popup.x, tspos_popup.y)) {
            // 在该rotnum下的test的该位置已经被探测过了，这个位置可能是其他出发点出发滑动测试检测过的
            continue;
        }
        BM2D_SET (psti->bmbuf_checked_global, (psti->tc_base->maxpos.x + psti->tsrgabs_x.maxtest + 2), tspos_popup.x, tspos_popup.y);

        memset (cnt_resi_dir, 0, sizeof (cnt_resi_dir));

        tstc_plumb_position_test (psti, &(tspos_popup), plist_points_ok, cnt_resi_dir);

        for (k = 0; k < NUM_TYPE(cnt_resi_dir, size_t); k ++) {
            // 判断4个方向上是否可移动
            if (cnt_resi_dir[k] < 1) {
                // 该方向上没有Base的tile
                // push the position to stack
                // g_dir_stestofslide[psti->dir_cur_slide][k]

                // 获取一个方向上邻接的点的坐标
                memmove (&tspos_base, &tspos_popup, sizeof (tspos_popup));
                if (set_dir_pos ((int)(g_dir_stestofslide[(int)psti->dir_cur_slide][k]), &tspos_base) < 0) {
                    continue;
                }
                // 检查是否与base没有交界了
                if ((tspos_base.x > psti->tc_base->maxpos.x + psti->tsrgabs_x.maxtest)
                    || (tspos_base.y > psti->tc_base->maxpos.y + psti->tsrgabs_y.maxtest)
                   )
                {
                    continue;
                }
                if (BM2D_IS_SET (psti->bmbuf_checked_local,
                        (psti->tc_base->maxpos.x + psti->tsrgabs_x.maxtest + psti->tsrgabs_x.maxtest + 2),
                        tspos_base.x, tspos_base.y)) {
                    // 已经被探测过了
                    continue;
                }
                BM2D_SET (psti->bmbuf_checked_local,
                    (psti->tc_base->maxpos.x + psti->tsrgabs_x.maxtest + psti->tsrgabs_x.maxtest + 2),
                    tspos_base.x, tspos_base.y);

                if (0 == BM2D_IS_SET (psti->bmbuf_checked_global, (psti->tc_base->maxpos.x + psti->tsrgabs_x.maxtest + 2), tspos_base.x, tspos_base.y)) {
                    // 没被探测过
                    // 保存下个可以移动到的位置到堆栈中
                    st_push (psti->pstk_local, &tspos_base);
                }
            }
        }
    }
}

/* test if two tilecomp can be merged, return the positions(dir,gluevalue,x,y) */
// plist_points_ok: 检验符合 temperature 的点的列表，包括dir,x,y
// psim: the shared part of it will be used in tstc_plumb_test()
// 注意 tc_test 是原始没有根据 rotnum 转置的，需要在使用中转换坐标。
int
tstc_mesh_test2d_nature (tssiminfo_t *psim, tstilecomb_t *tc_base, tstilecomb_t *tc_test, memarr_t *plist_points_ok)
{
    // slide test
    // bitmap test
    // 步骤：
    // 先使用bitmap检测一个接触位置，该接触位置是逆时针方向每次固定测试。
    // 如果检测可以接触的话，则检测接触位置各个方向上是否可以粘合，即满足 temperature 条件。
    // 如果满足，则记录下该位置
    size_t x, y;
    int i;
    tsposition_t tspos_push; // 将压入栈中的一个坐标点
    tsslidetestinfo_t slidetestinfo;

    stk_t stk_local; // 在某 posx , 将可以访问的临近位置压入该栈，待稍后做深度优先搜索。
    st_init (&stk_local, sizeof (tsposition_t));

    i = 0;
    if (psim->flg_norotate) {
        i = 3;
    }
    for (; i < 4; i ++) {
        // 2D test supertile 的四个旋转位置的测试
        tsstinfo_init (&slidetestinfo, tc_base, tc_test, 3 - i);
        slidetestinfo.psim = psim;
        slidetestinfo.pstk_local = &stk_local;
        slidetestinfo.rotnum = 3 - i;

        // bmbuf_checked_global 保存在test的该旋转角度下已经被检测的点和被忽略的点（如不处于Base上）
        BM2D_RESET (slidetestinfo.bmbuf_checked_global,
            (slidetestinfo.tc_base->maxpos.x + slidetestinfo.tsrgabs_x.maxtest + 2),
            (slidetestinfo.tc_base->maxpos.y + slidetestinfo.tsrgabs_y.maxtest + 2));

        // the sliding testing 1: from the west to east, begin at the left bottom
        slidetestinfo.dir_cur_slide = ORI_EAST;
        for (x = 1; x < slidetestinfo.tsrgabs_x.maxtest + slidetestinfo.tsrgabs_x.maxbase; x ++) {
        /* 先找出在进入Base区域前会遇到的Base的tile, 找到后压如栈中。
              如果栈不空，则一直做如下操作：
                从栈中弹出一个记录
                对该位置做覆盖检测：
                   检查相邻的是否有Base的tile
                     如果有则将该方向的 cnt_resi_xxx ++; 计算是否可以glue,如可以，累加glue
                   统计所有覆盖的区域后
                     如果glue大于temp则记录该位置到返回列表中
                     如果cnt_resi_xxx的值 < 1 且全局记录和本地记录表示没有访问过该点，则将该方向邻接位置压入栈中,并将该方向邻接位置设置到本地已访问点中（防止重复压栈）
                     将该点计入全局的已访问点中
        */

            tspos_push.x = x;
            tspos_push.y = 0;

            st_push (&stk_local, &tspos_push);
            tstc_plumb_test (&slidetestinfo, plist_points_ok);
        }

        // the sliding testing 2: from the south to north, begin at the left bottom
        slidetestinfo.dir_cur_slide = ORI_NORTH;
        for (y = 1; y < slidetestinfo.tsrgabs_y.maxtest + slidetestinfo.tsrgabs_y.maxbase; y ++) {
            tspos_push.x = 0;
            tspos_push.y = y;

            st_push (&stk_local, &tspos_push);
            tstc_plumb_test (&slidetestinfo, plist_points_ok);
        }

        // the sliding testing 3: from the north to south, begin at the right top
        slidetestinfo.dir_cur_slide = ORI_SOUTH;
        for (y = slidetestinfo.tsrgabs_y.maxtest + slidetestinfo.tsrgabs_y.maxbase - 1; y > 0; y --) {
            tspos_push.x = slidetestinfo.tsrgabs_x.maxtest + slidetestinfo.tsrgabs_x.maxbase;
            tspos_push.y = y;

            st_push (&stk_local, &tspos_push);
            tstc_plumb_test (&slidetestinfo, plist_points_ok);
        }

        // the sliding testing 3: from the east to west, begin at the right top
        slidetestinfo.dir_cur_slide = ORI_WEST;
        for (x = slidetestinfo.tsrgabs_x.maxtest + slidetestinfo.tsrgabs_x.maxbase - 1; x > 0; x --) {
            tspos_push.x = x;
            tspos_push.y = slidetestinfo.tsrgabs_y.maxtest + slidetestinfo.tsrgabs_y.maxbase;

            st_push (&stk_local, &tspos_push);
            tstc_plumb_test (&slidetestinfo, plist_points_ok);
        }
    }

    st_clear (&stk_local, NULL);
    tsstinfo_clear (&slidetestinfo);
    return 0;
}

// test all of the positions of the base supertile, including the holes in the base supertile.
// call this function when psim->flg_inserthole = 1
int
tstc_mesh_test2d_godhand (tssiminfo_t *psim, tstilecomb_t *tc_base, tstilecomb_t *tc_test, memarr_t *plist_points_ok)
{
    tsposition_t pos;
    int ii;
    int i, j, k;
    tsslidetestinfo_t slidetestinfo;
    tsslidetestinfo_t *psti = &slidetestinfo;
    tsposition_t tspos_popup; // 从栈中弹出的一个点的坐标
    size_t cnt_resi_dir[4]; // 四个方向上遇到的Base的tile的个数

    ii = 0;
    if (psim->flg_norotate) {
        ii = 3;
    }
    for (; ii < 4; ii ++) {
        // 2D test supertile 的四个旋转位置的测试
        tsstinfo_init (&slidetestinfo, tc_base, tc_test, 3 - ii);
        slidetestinfo.psim = psim;
        slidetestinfo.rotnum = 3 - ii;

        for (pos.x = 0; pos.x < slidetestinfo.tsrgabs_x.maxtest + slidetestinfo.tsrgabs_x.maxbase + 1; pos.x ++) {
            for (pos.y = 0; pos.y < slidetestinfo.tsrgabs_y.maxtest + slidetestinfo.tsrgabs_y.maxbase + 1; pos.y ++) {
                // check the overlay area of the two supertiles

                // 用作检测Test测试范围的参数
                psti->tsrgstst_x.posx = pos.x;
                psti->tsrgstst_y.posx = pos.y;
                //ts_absolute_range_stest (&(psti->tsrgstst_x));
                //ts_absolute_range_stest (&(psti->tsrgstst_y));
                ts_absolute_range_overlap (&(psti->tsrgstst_x));
                ts_absolute_range_overlap (&(psti->tsrgstst_y));

                // 用作检测Base范围的参数
                psti->tsrgabs_x.posx = pos.x;
                psti->tsrgabs_y.posx = pos.y;
                ts_absolute_range_overlap (&(psti->tsrgabs_x));
                ts_absolute_range_overlap (&(psti->tsrgabs_y));

                // make sure that there're no tiles in the same position
                for (k = 0; k < (psti->tsrgabs_x.rang_num) * (psti->tsrgabs_y.rang_num); k ++) {
                    tsposition_t tspos_base; // 临时，base的某个点

                    i = k / psti->tsrgabs_x.rang_num;
                    j = k % psti->tsrgabs_x.rang_num;

                    // 当前
                    tspos_base.x = j + psti->tsrgstst_x.rang_start;
                    tspos_base.y = i + psti->tsrgstst_y.rang_start;
                    tspos_base.x = POS_ABS2REL_TEST(tspos_base.x, &(psti->tsrgabs_x));
                    tspos_base.y = POS_ABS2REL_TEST(tspos_base.y, &(psti->tsrgabs_y));
                    // Test 上是否有tile
                    if (0 != BM2D_IS_SET (psti->bmbuf_test, psti->tsrgabs_x.maxtest, tspos_base.x, tspos_base.y)) {
                        tspos_base.x = j + psti->tsrgabs_x.rang_start;
                        tspos_base.y = i + psti->tsrgabs_y.rang_start;
                        tspos_base.x = POS_ABS2REL_BASE(tspos_base.x, &(psti->tsrgabs_x));
                        tspos_base.y = POS_ABS2REL_BASE(tspos_base.y, &(psti->tsrgabs_y));
                        if (0 != BM2D_IS_SET (psti->bmbuf_base, psti->tsrgabs_x.maxbase, tspos_base.x, tspos_base.y)) {
                            // 存在相同位置上既有属于base的tile 也有属于test的tile
                            // 則退出并忽略掉該位置
                            break;
                        }
                    }
                }
                if (k < psti->tsrgabs_x.rang_num * psti->tsrgabs_y.rang_num) {
                    // 存在相同位置上同時有 test 和 base 的 tile
                    // 忽略掉該位置
                    continue;
                }

                // check the four sides of one tile of the test and calculate the glue
                // if glue >= t then save the position to the plist_points_ok
                memset (cnt_resi_dir, 0, sizeof (cnt_resi_dir));
                tstc_plumb_position_test (psti, &pos, plist_points_ok, cnt_resi_dir);
            }
        }
    }

    tsstinfo_clear (&slidetestinfo);
    return 0;
}

#include <igraph/igraph.h>

/*2D=2;3D=3*/
// 2D: for the matrix of tiles, the edges of the supertile can be get from the directions ORI_NORTH and ORI_EAST of each tile.
static int g_dir_split_test[] = {
    ORI_NORTH,
    ORI_EAST,
};

/*
  tstc_split(): find the list of the spliting
  use the method of min-cut of graph

  ptc_2bsplit:
  psim:
  temperature:
  plist_splited: the list of spliting result (tstilecomb_t), the tstilecomb_t.id is the number of this type of tile
 */
int
tstc_split_2d (tstilecomb_t *ptc_2bsplit, tssiminfo_t *psim, size_t temperature, memarr_t *plist_splited)
{
    int ret_func = 0;
    int ret;
    igraph_t g;
    size_t num_pos;
    size_t i;
    size_t j;
    size_t k;
    size_t m;
    size_t idx;
    size_t cnt_edges;
    tstilecombitem_t curitem;
    tstilecombitem_t pinitem;

    int glue_base;
    int glue_test;
    tstilecomb_t *pttcomb_cur;
    tstilecomb_t *pttcomb_new;

    igraph_real_t val_mincut;
    igraph_vector_t vedges;
    igraph_vector_t vweights;
    igraph_vector_t vpart1;
    igraph_vector_t vpart2;
    igraph_vector_t vcut;

    pttcomb_cur = ptc_2bsplit;

    tstc_chkassert_2d (pttcomb_cur);
    // get the number of the tiles in the supertile
    num_pos = slist_size (&(pttcomb_cur->tbuf));
    if (num_pos < 2) {
        // do not need to calculate
        assert (num_pos == 1);
        return 0;
    }
    /*if (num_pos < 3) {
        assert (num_pos == 2);
        // test the glues between two tiles
        return ?;
    }*/
    // for each of the current items in list, do:
    //   calculate the min cut of an un-directed graph
    for (k = 0; ;) {

        igraph_vector_init (&vedges,   num_pos * 2);
        igraph_vector_init (&vweights, num_pos);
        igraph_vector_resize (&vedges, 0);
        igraph_vector_resize (&vweights, 0);

        cnt_edges = 0;
        // int igraph_add_vertices (igraph_t *graph, igraph_integer_t nv, void *attr);
        // add the edges to the graph
        for (i = 0; i < num_pos; i ++) {
            // add the edges: the east and south of each tile
            slist_data (&(pttcomb_cur->tbuf), i, &curitem);
            for (j = 0; j < NUM_TYPE (g_dir_split_test, int); j ++) {
                memmove (&pinitem, &curitem, sizeof (curitem));
                set_dir_pos (g_dir_split_test[j], &(pinitem.pos));
                assert (pinitem.pos.x >= curitem.pos.x);
                assert (pinitem.pos.y >= curitem.pos.y);

                ret = slist_find (&(pttcomb_cur->tbuf), &pinitem, NULL, &idx);
                if (ret >= 0) {
                    // here (idx > i) means that the link is non-direct
                    assert (idx > i);
                    // found
                    slist_data (&(pttcomb_cur->tbuf), idx, &pinitem);
                    // get the glue index: glue_base & glue_test
                    glue_base = tile_get_glue_2d (psim->ptilevec, psim->num_tilevec, &curitem, g_dir_split_test[j], 0);
                    glue_test = tile_get_glue_2d (psim->ptilevec, psim->num_tilevec, &pinitem, g_dir_opposite[g_dir_split_test[j]], 0);
                    if (GLUE_CAN_ADHERE (glue_test, glue_base)) {
                        //igraph_add_edge (&g, i, idx);
                        igraph_vector_resize (&vedges, cnt_edges * 2 + 2);
                        igraph_vector_set (&vedges, cnt_edges * 2,     i);
                        igraph_vector_set (&vedges, cnt_edges * 2 + 1, idx);
                        // add weight:
                        igraph_vector_resize (&vweights, cnt_edges + 1);
                        // get the glue value: psim->pgluevec[glue_base]
                        //VECTOR(vweights)[cnt_edges] = psim->pgluevec[glue_base];
                        igraph_vector_set (&vweights, cnt_edges, psim->pgluevec[glue_base]);
                        cnt_edges ++;
                    }
                }
            }
        }
        assert (igraph_vector_size (&vedges) == cnt_edges * 2);
        assert (igraph_vector_size (&vweights) == cnt_edges);

        // int igraph_empty (igraph_t *graph, igraph_integer_t n, igraph_bool_t directed);
        // ret = igraph_empty (&g, num_pos, 0/* 0: not directed */);
        ret = igraph_create (&g, &vedges, num_pos, 0/* 0: not directed */);
        if (ret == IGRAPH_EINVAL) {
            assert (0);
            ret_func = -1;
            igraph_vector_destroy (&vedges);
            igraph_vector_destroy (&vweights);
            break;
        }

        igraph_vector_init (&vpart1, 0);
        igraph_vector_init (&vpart2, 0);
        igraph_vector_init (&vcut, 0);

        igraph_mincut (&g, &val_mincut, &vpart1, &vpart2, NULL, &vweights);

        if (val_mincut >= temperature) {
            // the supertile segment will not split in this temperature
            k ++;
        } else /*if (value < temperature) */{
            // the sum of glue strenth less than temperature, the supertile will break away at this line
            assert (igraph_vector_size (&vpart1) > 0);
            assert (igraph_vector_size (&vpart2) > 0);
            assert (num_pos == igraph_vector_size (&vpart1) + igraph_vector_size (&vpart2));

            // 1. the second part of the supertile segment
            // copy the 2nd part segment to a new location of the supertile segment list (SSL)
            igraph_vector_sort (&vpart2);

            // 同一个plist_splited申请内存可能导致其基址被改变，所以重新获取当前 pttcomb_cur
            if (pttcomb_cur != ptc_2bsplit) {
                ma_data_unlock (plist_splited, k);
            }
            idx = ma_inc (plist_splited);
            pttcomb_new = (tstilecomb_t *) ma_data_lock (plist_splited, idx);
            if (pttcomb_cur != ptc_2bsplit) {
                pttcomb_cur = (tstilecomb_t *) ma_data_lock (plist_splited, k);
            }
            tstc_init (pttcomb_new);
            for (m = 0; m < igraph_vector_size (&vpart2); m ++) {
                slist_data (&(pttcomb_cur->tbuf), (int)igraph_vector_e (&vpart2, m), &curitem);
                tstc_additem (pttcomb_new, &curitem);
            }
            // nomalize the pttcomb_new: let the left bottom of the supertile segment move to the (0,0) position
            tstc_nomalize_2d (pttcomb_new);
            tstc_chkassert_2d (pttcomb_new);
            pttcomb_new->id = 1;
            assert (slist_size (&(pttcomb_new->tbuf)) > 0);
            assert (slist_size (&(pttcomb_new->tbuf)) == igraph_vector_size (&vpart2));
            ma_data_unlock (plist_splited, idx);

            // 2. the first part of the supertile segment
            igraph_vector_sort (&vpart1);
            if (pttcomb_cur == ptc_2bsplit) {
                // copy the other half to the list SSL
                assert (ma_size (plist_splited) == 1);
                idx = ma_inc (plist_splited);
                pttcomb_new = (tstilecomb_t *) ma_data_lock (plist_splited, idx);
                tstc_init (pttcomb_new);
                for (m = 0; m < igraph_vector_size (&vpart1); m ++) {
                    slist_data (&(pttcomb_cur->tbuf), (int)igraph_vector_e (&vpart1, m), &curitem);
                    tstc_additem (pttcomb_new, &curitem);
                }
                // nomalize the pttcomb_new: let the left bottom of the supertile segment move to the (0,0) position
                tstc_nomalize_2d (pttcomb_new);
                tstc_chkassert_2d (pttcomb_new);
                pttcomb_new->id = 1;
                assert (slist_size (&(pttcomb_new->tbuf)) > 0);
                assert (slist_size (&(pttcomb_new->tbuf)) == igraph_vector_size (&vpart1));
                ma_data_unlock (plist_splited, idx);
            } else {
                // remove the items denoted by vect2 in the current SSL node reversely
                for (m = igraph_vector_size (&vpart2); m > 0; m --) {
                    assert (slist_size (&(pttcomb_cur->tbuf)) > 1);
                    slist_rmdata (&(pttcomb_cur->tbuf), (int)igraph_vector_e (&vpart2, m - 1), NULL);
                    assert (slist_size (&(pttcomb_cur->tbuf)) > 0);
                }
                // nomalize the pttcomb_cur: let the left bottom of the supertile segment move to the (0,0) position
                tstc_nomalize_2d (pttcomb_cur);
                tstc_chkassert_2d (pttcomb_cur);
                assert (slist_size (&(pttcomb_cur->tbuf)) > 0);
                assert (slist_size (&(pttcomb_cur->tbuf)) == igraph_vector_size (&vpart1));
                // free pttcomb_cur
                ma_data_unlock (plist_splited, k);
            }
            if (k >= ma_size (plist_splited)) {
                // reach to the end of the list of split segments
                break;
            }
            // next supertile segment to be processed
            pttcomb_cur = (tstilecomb_t *)ma_data_lock (plist_splited, k);
            assert (NULL != pttcomb_cur);
            tstc_chkassert_2d (pttcomb_cur);

            // skip the supertile in which the number of tiles less than 2( == 1).
            // get the number of the tiles in the supertile
            num_pos = slist_size (&(pttcomb_cur->tbuf));
            for (; (num_pos < 2) && (k < ma_size (plist_splited)); ) {
                // do not need to calculate
                assert (num_pos == 1);
                ma_data_unlock (plist_splited, k);
                k ++;
                if (k >= ma_size (plist_splited)) {
                    break;
                }
                pttcomb_cur = (tstilecomb_t *)ma_data_lock (plist_splited, k);
                assert (NULL != pttcomb_cur);
                tstc_chkassert_2d (pttcomb_cur);
                num_pos = slist_size (&(pttcomb_cur->tbuf));
            }
            if (k >= ma_size (plist_splited)) {
                // reach to the end of the list of split segments
                break;
            }
        }
        igraph_vector_destroy (&vedges);
        igraph_vector_destroy (&vweights);
        igraph_vector_destroy (&vpart1);
        igraph_vector_destroy (&vpart2);
        igraph_vector_destroy (&vcut);
        igraph_destroy (&g);
        if (k >= ma_size (plist_splited)) {
            // reach to the end of the list of split segments
            break;
        }
    }
    return ret_func;
}
