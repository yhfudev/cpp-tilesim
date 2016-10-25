/******************************************************************************
 * Name:        src/pfrandom.h
 * Purpose:     Random init
 * Author:      Yunhui Fu
 * Created:     2008-03-30
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2007 Yunhui Fu
 * Licence:     LGPL licence
 ******************************************************************************/
#ifndef PFRANDOM_H
#define PFRANDOM_H
#ifdef __cplusplus
extern "C" {
#endif

void init_seed (void);

size_t my_irand_sec (void);
#define my_irrand_sec(max) (my_irand_sec () % (max))

// return value between (0.0 ~ 1.0)
double my_drand_sec (void);
#define my_drrand_sec(max) my_drand_sec() * ((double)(max))

#ifdef WIN32
#define my_irand_weak() my_irand_sec ()
#else
size_t my_irand_weak (void);
#endif

double my_drand_weak (void);
#define my_irrand_weak(max) (my_irand_weak () % (max))
#define my_drrand_weak(max) (my_drand_weak () * ((double)(max)))

#define my_irand() my_irand_weak()
#define my_drand() my_drand_weak()
#define my_irrand(max) my_irrand_weak(max)
#define my_drrand(max) my_drrand_weak(max)

int hash_revert_init (size_t max, void **ret_ref);
int hash_revert_clear (void *hash_ref);
size_t hash_revert (void *hash_ref, size_t val_in, size_t maxval);

#ifdef __cplusplus
}
#endif
#endif /* PFRANDOM_H */
