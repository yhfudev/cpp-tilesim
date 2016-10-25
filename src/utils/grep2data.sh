#!/bin/sh
# Name:        grep2data.sh
# Purpose:     convert the grep'ed xml data to TAB'ed csv data file
# Author:      Yunhui Fu
# Created:     2009-03-01
# Modified by:
# RCS-ID:      $Id: $
# Copyright:   (c) 2009 Yunhui Fu
# Licence:     GPL licence version 3
##############################################################################

FN_OUT=$2

if [ -f "${FN_OUT}" ] ; then
  echo "file exist: '${FN_OUT}'"
  exit 1
fi

FN_IN=$1

if [ x${FN_OUT} = x ]; then
    FN_BASE=`echo ${FN_IN} | awk -F. '{print $1}'`
    #FN_SUFFIX=`echo ${FN_IN} | awk -F. '{print $2}'`
    FN_OUT=${FN_BASE}.csv
fi

  if [ ! -f "${FN_IN}" ] ; then
    echo "file not exist: '${FN_IN}'"
    exit 1
  fi
  if [ a"${FN_IN}" = a"${FN_OUT}" ] ; then
    echo "out file should not equal to the input file: '${FN_IN}'"
    exit 1
  fi

{
  echo "#TYP	N	M	P	T	BIRTH" > ${FN_OUT}

  read TST_STR
  until [ -z "$TST_STR" ]; do
    S1=`echo ${TST_STR} | awk '{print $1}'`
    S2=`echo ${S1} | awk -F. '{print $1}'`
    S3=`echo ${S1} | awk -F. '{print $2}'`
    S4=${S2}.${S3}

    P1=`echo ${S4} | awk -F_ '{print $3}'`
    P2=`echo ${S4} | awk -F_ '{print $4}'`
    P3=`echo ${S4} | awk -F_ '{print $5}'`
    P4=`echo ${S4} | awk -F_ '{print $7}'`
    P5=`echo ${S4} | awk -F_ '{print $8}'`

    N=`echo ${P1} | awk -FN '{print $2}'`
    M=`echo ${P2} | awk -FM '{print $2}'`
    T=`echo ${P4} | awk -FT '{print $2}'`
    P=`echo ${P5} | awk -FP '{print $2}'`

    S5=`echo ${TST_STR} | awk '{print $2}'`
    S5=`echo ${S5} | awk -F\< '{print $2}'`
    BIRTH=`echo ${S5} | awk -F\> '{print $2}'`

    TYP=${P3}

    echo "TYP=${TYP}	N=${N}	M=${M}	P=${P}	T=${T}	BIRTH=${BIRTH}"
    echo "${TYP}	${N}	${M}	${P}	${T}	${BIRTH}" >> ${FN_OUT}

    read TST_STR
  done
} < "${FN_IN}"

exit 0
