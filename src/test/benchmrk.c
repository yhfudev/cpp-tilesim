/******************************************************************************
 * Name:        benchmrk.c
 * Purpose:     calculate the runing time of the function.
 * Author:      Yunhui Fu
 * Created:     2009-09-05
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/
#include <sys/timeb.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#include "benchmrk.h"

static double
difftimeb (struct timeb * t1, struct timeb * t2)
{
  double ret;
  double tmp;

  if((0 == t1) || (0 == t2)) return 0;

  ret = t1->millitm;
  ret /= 1000.0;
  ret += (double)(t1->time);

  tmp = t2->millitm;
  tmp /= 1000.0;
  tmp += (double)(t2->time);

  return(ret - tmp);
}

int
bm_start (benchmark_t * pbm, unsigned long int loop)
{
  assert(pbm != 0);
  pbm->time_loop = loop;
  ftime(&(pbm->start));
  return 0;
}

int
bm_end (benchmark_t * pbm, const char *msg)
{
  double time_interval;
  double time_average;

  assert(pbm != 0);

  ftime (&((pbm)->end));
  
  // calculating ...
  time_interval = difftimeb(&(pbm->end), &(pbm->start));

  time_average = (time_interval) / ((double)(pbm->time_loop));

//  printf("RESULT: test time: %d sec @ loop %u; %f sec/1\n", time_interval, pbm->time_loop, time_average);
  printf("%s: testtime: %f sec ", ((NULL == msg)? "RESULT": msg), time_interval);
  printf("@ loop %lu; ", pbm->time_loop);
  printf("%f s/times; ", time_average);
  time_average = 1 / time_average;
  printf("%f times/s\n", time_average);

  return 0;
}
