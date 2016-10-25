/******************************************************************************
 * Name:        src/pfrandom.c
 * Purpose:     Random init
 * Author:      Yunhui Fu
 * Created:     2008-03-30
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2007 Yunhui Fu
 * Licence:     LGPL licence
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h> // srand()
#include <time.h> // time()
#include <string.h> // memset()
#include <assert.h>

#include "pfrandom.h"

void
init_seed (void)
{
    unsigned int seed_val;
    FILE *fp_rand = NULL;
    seed_val = time (NULL);
    fp_rand = fopen ("/dev/urandom", "rb");
    if (NULL != fp_rand) {
        if (sizeof (seed_val) != fread (&seed_val, 1, sizeof (seed_val), fp_rand)) {
            fprintf (stderr, "Error in read from the random device\n");
        } else {
            fprintf (stderr, "use the value from random device: %u\n", seed_val);
        }
        fclose (fp_rand);
    } else {
        //seed_val = time (NULL);
        fprintf (stderr, "use the default random number: time(NULL)=%u\n", seed_val);
    }
    srand (seed_val);
}

#ifdef WIN32
#include <windows.h>
#include <wincrypt.h>

static HCRYPTPROV hCryptProv = NULL;

size_t
my_irand_sec (void)
{
    size_t val = 0;
    if (hCryptProv == NULL) {
        CryptAcquireContext((HCRYPTPROV*)&hCryptProv, NULL, NULL, PROV_RSA_FULL, 0);
    }
    if (hCryptProv == NULL) {
        return rand ();
    } else {
        CryptGenRandom (hCryptProv, sizeof (val), (BYTE*)(&val));
    }
    // if (hCryptProv != NULL) CryptReleaseContext (hCryptProv, 0);
    return val;
}

#else
static FILE *g_fp_rand = NULL;
static FILE *g_fp_urand = NULL;

size_t
my_irand_sec (void)
{
    size_t val;
    if (NULL == g_fp_rand) {
        g_fp_rand = fopen ("/dev/random", "rb");
    }
    if (NULL == g_fp_rand) {
        val = rand ();
    } else {
        fread (&val, 1, sizeof (size_t), g_fp_rand);
    }
    return val;
}

size_t
my_irand_weak (void)
{
    size_t val;
    if (NULL == g_fp_urand) {
        g_fp_urand = fopen ("/dev/urandom", "rb");
    }
    if (NULL == g_fp_urand) {
        val = rand ();
    } else {
        if (fread (&val, 1, sizeof (size_t), g_fp_urand) < sizeof (size_t)) {
            printf ("Error in read random\n");
            val = rand ();
        }
    }
    return val;
}

#endif

#define my_drand_sec0() ((double)my_irand_sec() / (double)((size_t)(-1)))
#define my_drrand_sec0(max) my_drand_sec0() * ((double)(max))
double
my_drand_sec (void)
{
    double val;
    val = 1.0 / ((double)((size_t)(-1)));
    val = my_drand_sec0() + my_drrand_sec0(val);
    return val;
}

#define my_drand_weak0() ((double)my_irand_weak() / (double)((size_t)(-1)))
#define my_drrand_weak0(max) my_drand_weak0() * ((double)(max))
double
my_drand_weak (void)
{
    double val;
    val = 1.0 / ((double)((size_t)(-1)));
    val = my_drand_weak0() + my_drrand_weak0(val);
    return val;
}

#if DEBUG
void
test_random(void)
{
    double vald;
    size_t vali;
    size_t i;
    for (i = 0; i < 200; i ++) {
        vali = my_irrand (1000);
        printf ("vali = %d\n", vali);
        assert (vali < 1000);
    }
    for (i = 0; i < 200; i ++) {
        vald = my_drrand (10.0);
        printf ("vald = %f\n", vald);
        assert ((0.0 <= vald) && (vald < 10.0));
    }
}
#endif // DEBUG

#define hash_revert_rand() my_irand()

static size_t
hash_revert_get_bits (size_t max)
{
    size_t i;
    size_t val = 0x01;

    // detect the number of the bit of binary code
    for (i = 0; val <= max; i ++) {
        val <<= 1;
    }
    if (i < 1) {
        i = 1;
    }
    return i;
}

int
hash_revert_init0 (size_t max, void **ret_ref)
{
    if (NULL == ret_ref) {
        return -1;
    }
    *ret_ref = (void *)hash_revert_get_bits(max);
    return 0;
}

int
hash_revert_clear0 (void *ret_ref)
{
    return 0;
}

//bits:  1   1   2   2   3   3   3   3   4
//max:   0   1   2   3   4   5   6   7   8
  //i:   0   1   2   2   3   3   3   3   4
//val:  01  02  04  04  08  08  08  08  10

size_t
hash_revert0 (void *ret_ref, size_t val_in, size_t maxval)
{
    // revert the value
    size_t i;
    size_t val = 0;
    size_t mask = 0x01;
    size_t maxbits = (size_t)ret_ref;
    for (i = 0; i < maxbits; i ++) {
        val <<= 1;
        if (mask & val_in) {
            val |= 0x01;
        }
        mask <<= 1;
    }
    if (val > maxval) {
        //assert (val == maxval + 1);
        return val_in;
    }
    return val;
}

typedef struct _hash_revert_t {
    size_t maxbits;
    size_t maxval;
    size_t *buffer;
} hash_revert_t;

int
hash_revert_init (size_t max, void **ret_ref)
{
    void *hash_ref0;
    size_t i;
    size_t j;
    size_t cnt;
    size_t val;
    hash_revert_t *phr;
    if (NULL == ret_ref) {
        return -1;
    }
    phr = (hash_revert_t *) malloc (sizeof (*phr));
    if (NULL == phr) {
        return -1;
    }
    //phr->maxbits = (void *)hash_revert_get_bits(max);
    phr->maxval = max;
    phr->buffer = (size_t *) malloc (sizeof (size_t) * (max + 1));
    if (NULL == phr->buffer) {
        free (phr);
        return -1;
    }
    memset (phr->buffer, 0, sizeof (size_t) * (max + 1));
    hash_revert_init0 (max, &hash_ref0);
    for (i = 0; i <= max; i ++) {
        val = hash_revert0 (hash_ref0, i, max);
        if (val == i) {
            // skip
            continue;
        }
        assert ((0 <= val) && (val <= max));
        assert (phr->buffer[val] == 0);
        phr->buffer[val] = i + 1;
    }
    hash_revert_clear0 (hash_ref0);
    for (i = 0; i <= max; i ++) {
        val = hash_revert0 (hash_ref0, i, max);
        if (val != i) {
            // skip
            continue;
        }
        cnt = 0;
        for (j = hash_revert_rand (); cnt <= max; j ++) {
            j %= (max + 1);
            if (phr->buffer[j] < 1) {
                phr->buffer[j] = i + 1;
                break;
            }
            cnt ++;
        }
        if (cnt > max) {
            assert (cnt <= max);
        }
    }
    *ret_ref = phr;
    return 0;
}

int
hash_revert_clear (void *hash_ref)
{
    hash_revert_t *phr = (hash_revert_t *)hash_ref;
    if (phr->buffer) {
        free (phr->buffer);
    }
    return 0;
}

size_t
hash_revert (void *hash_ref, size_t val_in, size_t maxval)
{
    hash_revert_t *phr = (hash_revert_t *)hash_ref;
    assert (phr->buffer[val_in] > 0);
    return (phr->buffer[val_in] - 1);
}

#if DEBUG
#define MAXVAL 100000
void
test_revert (void)
{
    void *hashref;
    size_t i;
    size_t j;
    size_t ret;
    char buf[MAXVAL];

    for (i = 0; i < MAXVAL; i ++) {
        fprintf (stdout, "check max %d\n", i);
        memset (buf, 0, sizeof (buf));
        hash_revert_init (i, &hashref);
        for (j = 0; j <= i; j ++) {
            ret = hash_revert (hashref, j, i);
            //fprintf (stdout, "revert (val=%d, maxval=%d, maxbit=%d) = %d\n", j, i, maxbit, ret);
            assert (ret <= i);
            assert (buf[ret] == 0);
            buf[ret] = 1;
        }
        for (j = 0; j < i; j ++) {
            assert (buf[ret] == 1);
        }
        hash_revert_clear (hashref);
    }
    exit (0);
}
#endif // DEBUG
