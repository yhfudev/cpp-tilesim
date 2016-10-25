/******************************************************************************
 * Name:        benchmrk.h
 * Purpose:     calculate the runing time of the function.
 * Author:      Yunhui Fu
 * Created:     2009-09-05
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/
#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <sys/timeb.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _benchmark_t
{
  struct timeb start;
  struct timeb end;
  unsigned long int time_loop;
} benchmark_t;

extern int bm_start (benchmark_t * bm, unsigned long int loop);
extern int bm_end (benchmark_t * bm, const char *msg);

#ifdef __cplusplus
}
#endif

#endif /* BENCHMARK_H */
