#!/bin/sh
# Name:        data2plot.sh
# Purpose:     extract all of the percentage into seperate csv file
#              for the importing of qtiplot
# Author:      Yunhui Fu
# Created:     2009-03-01
# Modified by:
# RCS-ID:      $Id: $
# Copyright:   (c) 2009 Yunhui Fu
# Licence:     GPL licence version 3
##############################################################################

SELF_DN_BASE=`dirname "$0"`
if [ "${SELF_DN_BASE}" = "" ]; then
  SELF_FN=`which "$0"`
  if [ "${SELF_FN}" = "" ]; then
    echo "Unable to the directory name of myself"
    exit 0
  fi
  SELF_DN_BASE=`dirname "${SELF_FN}"`
fi

FN_BASE=`echo $1 | awk -F. '{print $1}'`
FN_SUFFIX=`echo $1 | awk -F. '{print $2}'`

#echo "TYP	N	M	P	T	BIRTH"
cal_average ()
{
  CA_FN_INPUT=$1

  CA_FN_BASE=`echo $1 | awk -F. '{print $1}'`
  CA_FN_SUFFIX=`echo $1 | awk -F. '{print $2}'`

  rm -f ${CA_FN_BASE}_avg.${CA_FN_SUFFIX}
  rm -f tmpavg
  rm -f tmpdata
  rm -f tmpname
  touch tmpname
  echo "--------------------- ${CA_FN_BASE}_avg.${CA_FN_SUFFIX} ---------------------------"
  {
    while :; do
      read TMPFN
      if [ ! $? = 0 ]; then
        break
      fi
      if [ "${TMPFN}" = "" ]; then
        continue
      fi
      #TYP=`echo "${TMPFN}" | awk '{print $1}'`
      #P=`echo "${TMPFN}" | awk '{print $4}'`
      #T=`echo "${TMPFN}" | awk '{print $5}'`
      #BIRTH=`echo "${TMPFN}" | awk '{print $6}'`

      N=`echo "${TMPFN}" | awk '{print $2}'`
      if [ "${N}" = "N" ]; then
        continue;
      fi
      M=`echo "${TMPFN}" | awk '{print $3}'`
      grep "${N} ${M}" tmpname > /dev/null
      if [ "$?" = "0" ]; then
        continue;
      fi
      echo "${N} ${M}" >> tmpname

      awk -v N="${N}" -v M="${M}" '{ if ( $2 == N && $3 == M && $6 > 0) print $0 }' ${CA_FN_INPUT} > tmpdata
      #awk 'BEGIN { N=N } BEGIN { M=M } { if ( $2 == N && $3 == M && $6 > 0) print $0 }' ${CA_FN_INPUT} > tmpdata

      TOTAL=0
      NUM=0
      MIN=0
      MAX=0
      {
        while :; do
          read TMPVAL2
          if [ ! $? = 0 ]; then
            break
          fi
          if [ "${TMPVAL2}" = "" ]; then
            continue
          fi

          VAL=`echo "${TMPVAL2}" | awk '{print $6}'`

          if [ `expr ${MIN} \< 1` = 1 ]; then
            MIN=${VAL}
          fi
          if [ `expr ${VAL} \< ${MIN}` = 1 ]; then
            MIN=${VAL}
          fi
          if [ `expr ${VAL} \> ${MAX}` = 1 ]; then
            MAX=${VAL}
          fi

          TOTAL=`expr ${TOTAL} + ${VAL}`
          NUM=`expr ${NUM} + 1`
          #echo "NUM=${NUM}"; echo "VAL=${VAL}"; echo "TOTAL=${TOTAL}"
        done
      } < tmpdata

      #echo "total,num= ${TOTAL},${NUM}"
      expr ${NUM} \> 0 > /dev/null
      if [ $? = 0 ]; then
        AVERGE=`expr ${TOTAL} / ${NUM}`
        RADIUS=`expr ${MAX} - ${AVERGE}`
        RADIUS2=`expr ${AVERGE} - ${MIN}`
        if [ `expr ${RADIUS2} \> ${RADIUS}` = 1 ]; then
          RADIUS=${RADIUS2}
        fi
        echo "${N} ${M} ${MIN} ${MAX} ${AVERGE} ${RADIUS}" >> tmpavg

        echo "(${N},${M}) total(${TOTAL})/num(${NUM})=${AVERGE}; min=(${MIN}),max=(${MAX}) radius=${RADIUS}"
      fi

    done

  } < ${CA_FN_INPUT}

  # group the data for gnuplot:
  #${CA_FN_BASE}_avg_NB_M.${CA_FN_SUFFIX}
  # x axis: N; y axis: Birth; layer: M
  rm -f tmpavg2
  sort -n tmpavg | sort -n -s -k 2 -o tmpavg2
  {
    VALPRE=0

    while :; do
      read TMPVAL2
      if [ ! $? = 0 ]; then
        break
      fi
      if [ "${TMPVAL2}" = "" ]; then
        continue
      fi
      VAL=`echo "${TMPVAL2}" | awk '{print $2}'`

      if [ "${VALPRE}" = "0" ]; then
        VALPRE=${VAL}
      fi

      if [ ! "${VAL}" = "${VALPRE}" ]; then
        echo "" >> ${CA_FN_BASE}_avg_NB_M.${CA_FN_SUFFIX}
        VALPRE=${VAL}
      fi
      echo ${TMPVAL2} >> ${CA_FN_BASE}_avg_NB_M.${CA_FN_SUFFIX}
    done
  } < tmpavg2

  #${CA_FN_BASE}_avg_MB_N.${CA_FN_SUFFIX}
  # x axis: M; y axis: Birth; layer: N
  rm -f tmpavg2
  sort -n -k 2 -s tmpavg | sort -n -s -o tmpavg2
  {
    VALPRE=0

    while :; do
      read TMPVAL2
      if [ ! $? = 0 ]; then
        break
      fi
      if [ "${TMPVAL2}" = "" ]; then
        continue
      fi
      VAL=`echo "${TMPVAL2}" | awk '{print $1}'`

      if [ "${VALPRE}" = "0" ]; then
        VALPRE=${VAL}
      fi

      if [ ! "${VAL}" = "${VALPRE}" ]; then
        echo "" >> ${CA_FN_BASE}_avg_MB_N.${CA_FN_SUFFIX}
        VALPRE=${VAL}
      fi
      echo ${TMPVAL2} >> ${CA_FN_BASE}_avg_MB_N.${CA_FN_SUFFIX}
    done
  } < tmpavg2

}

# parameters:
# $1: the file name
#avg_data2plot ()

#if [ 0 = 1 ]; then
awk '{if ( $1 == "#TYP" || ( $1 == "nxn" && $4 == 0.000000 && $6 > 0 ) ) print $0 }' $1 > ${FN_BASE}_nxn_P0.${FN_SUFFIX}
awk '{if ( $1 == "#TYP" || ( $1 == "nxn" && $4 == 10.000000 && $6 > 0 ) ) print $0 }' $1 > ${FN_BASE}_nxn_P10.${FN_SUFFIX}
awk '{if ( $1 == "#TYP" || ( $1 == "nxn" && $4 == 20.000000 && $6 > 0 ) ) print $0 }' $1 > ${FN_BASE}_nxn_P20.${FN_SUFFIX}
awk '{if ( $1 == "#TYP" || ( $1 == "nxn" && $4 == 30.000000 && $6 > 0 ) ) print $0 }' $1 > ${FN_BASE}_nxn_P30.${FN_SUFFIX}
awk '{if ( $1 == "#TYP" || ( $1 == "nxn" && $4 == 40.000000 && $6 > 0 ) ) print $0 }' $1 > ${FN_BASE}_nxn_P40.${FN_SUFFIX}
awk '{if ( $1 == "#TYP" || ( $1 == "nxn" && $4 == 50.000000 && $6 > 0 ) ) print $0 }' $1 > ${FN_BASE}_nxn_P50.${FN_SUFFIX}
awk '{if ( $1 == "#TYP" || ( $1 == "nxn" && $4 == 60.000000 && $6 > 0 ) ) print $0 }' $1 > ${FN_BASE}_nxn_P60.${FN_SUFFIX}
awk '{if ( $1 == "#TYP" || ( $1 == "nxn" && $4 == 70.000000 && $6 > 0 ) ) print $0 }' $1 > ${FN_BASE}_nxn_P70.${FN_SUFFIX}
awk '{if ( $1 == "#TYP" || ( $1 == "nxn" && $4 == 80.000000 && $6 > 0 ) ) print $0 }' $1 > ${FN_BASE}_nxn_P80.${FN_SUFFIX}

awk '{if ( $1 == "#TYP" || ( $1 == "skeleton" && $4 == 0.000000 && $6 > 0 ) ) print $0 }' $1 > ${FN_BASE}_skeleton_P0.${FN_SUFFIX}
awk '{if ( $1 == "#TYP" || ( $1 == "skeleton" && $4 == 10.000000 && $6 > 0 ) ) print $0 }' $1 > ${FN_BASE}_skeleton_P10.${FN_SUFFIX}
awk '{if ( $1 == "#TYP" || ( $1 == "skeleton" && $4 == 20.000000 && $6 > 0 ) ) print $0 }' $1 > ${FN_BASE}_skeleton_P20.${FN_SUFFIX}
awk '{if ( $1 == "#TYP" || ( $1 == "skeleton" && $4 == 30.000000 && $6 > 0 ) ) print $0 }' $1 > ${FN_BASE}_skeleton_P30.${FN_SUFFIX}
awk '{if ( $1 == "#TYP" || ( $1 == "skeleton" && $4 == 40.000000 && $6 > 0 ) ) print $0 }' $1 > ${FN_BASE}_skeleton_P40.${FN_SUFFIX}
awk '{if ( $1 == "#TYP" || ( $1 == "skeleton" && $4 == 50.000000 && $6 > 0 ) ) print $0 }' $1 > ${FN_BASE}_skeleton_P50.${FN_SUFFIX}
awk '{if ( $1 == "#TYP" || ( $1 == "skeleton" && $4 == 60.000000 && $6 > 0 ) ) print $0 }' $1 > ${FN_BASE}_skeleton_P60.${FN_SUFFIX}
awk '{if ( $1 == "#TYP" || ( $1 == "skeleton" && $4 == 70.000000 && $6 > 0 ) ) print $0 }' $1 > ${FN_BASE}_skeleton_P70.${FN_SUFFIX}
awk '{if ( $1 == "#TYP" || ( $1 == "skeleton" && $4 == 80.000000 && $6 > 0 ) ) print $0 }' $1 > ${FN_BASE}_skeleton_P80.${FN_SUFFIX}

#fi

cal_average ${FN_BASE}_nxn_P0.${FN_SUFFIX}
cal_average ${FN_BASE}_nxn_P10.${FN_SUFFIX}
cal_average ${FN_BASE}_nxn_P20.${FN_SUFFIX}
cal_average ${FN_BASE}_nxn_P30.${FN_SUFFIX}
cal_average ${FN_BASE}_nxn_P40.${FN_SUFFIX}
cal_average ${FN_BASE}_nxn_P50.${FN_SUFFIX}
cal_average ${FN_BASE}_nxn_P60.${FN_SUFFIX}
cal_average ${FN_BASE}_nxn_P70.${FN_SUFFIX}
cal_average ${FN_BASE}_nxn_P80.${FN_SUFFIX}

cal_average ${FN_BASE}_skeleton_P0.${FN_SUFFIX}
cal_average ${FN_BASE}_skeleton_P10.${FN_SUFFIX}
cal_average ${FN_BASE}_skeleton_P20.${FN_SUFFIX}
cal_average ${FN_BASE}_skeleton_P30.${FN_SUFFIX}
cal_average ${FN_BASE}_skeleton_P40.${FN_SUFFIX}
cal_average ${FN_BASE}_skeleton_P50.${FN_SUFFIX}
cal_average ${FN_BASE}_skeleton_P60.${FN_SUFFIX}
cal_average ${FN_BASE}_skeleton_P70.${FN_SUFFIX}
cal_average ${FN_BASE}_skeleton_P80.${FN_SUFFIX}
#fi

${SELF_DN_BASE}/gnuplot3d.sh ${FN_BASE}_nxn_P0.${FN_SUFFIX}  "NxN - first one"
${SELF_DN_BASE}/gnuplot3d.sh ${FN_BASE}_nxn_P10.${FN_SUFFIX} "NxN - 10%"
${SELF_DN_BASE}/gnuplot3d.sh ${FN_BASE}_nxn_P20.${FN_SUFFIX} "NxN - 20%"
${SELF_DN_BASE}/gnuplot3d.sh ${FN_BASE}_nxn_P30.${FN_SUFFIX} "NxN - 30%"
${SELF_DN_BASE}/gnuplot3d.sh ${FN_BASE}_nxn_P40.${FN_SUFFIX} "NxN - 40%"
${SELF_DN_BASE}/gnuplot3d.sh ${FN_BASE}_nxn_P50.${FN_SUFFIX} "NxN - 50%"
${SELF_DN_BASE}/gnuplot3d.sh ${FN_BASE}_nxn_P60.${FN_SUFFIX} "NxN - 60%"
${SELF_DN_BASE}/gnuplot3d.sh ${FN_BASE}_nxn_P70.${FN_SUFFIX} "NxN - 70%"
${SELF_DN_BASE}/gnuplot3d.sh ${FN_BASE}_nxn_P80.${FN_SUFFIX} "NxN - 80%"

${SELF_DN_BASE}/gnuplot3d.sh ${FN_BASE}_skeleton_P0.${FN_SUFFIX}  "Skeleton - first one"
${SELF_DN_BASE}/gnuplot3d.sh ${FN_BASE}_skeleton_P10.${FN_SUFFIX} "Skeleton - 10%"
${SELF_DN_BASE}/gnuplot3d.sh ${FN_BASE}_skeleton_P20.${FN_SUFFIX} "Skeleton - 20%"
${SELF_DN_BASE}/gnuplot3d.sh ${FN_BASE}_skeleton_P30.${FN_SUFFIX} "Skeleton - 30%"
${SELF_DN_BASE}/gnuplot3d.sh ${FN_BASE}_skeleton_P40.${FN_SUFFIX} "Skeleton - 40%"
${SELF_DN_BASE}/gnuplot3d.sh ${FN_BASE}_skeleton_P50.${FN_SUFFIX} "Skeleton - 50%"
${SELF_DN_BASE}/gnuplot3d.sh ${FN_BASE}_skeleton_P60.${FN_SUFFIX} "Skeleton - 60%"
${SELF_DN_BASE}/gnuplot3d.sh ${FN_BASE}_skeleton_P70.${FN_SUFFIX} "Skeleton - 70%"
${SELF_DN_BASE}/gnuplot3d.sh ${FN_BASE}_skeleton_P80.${FN_SUFFIX} "Skeleton - 80%"


${SELF_DN_BASE}/gnuplot2d.sh ${FN_BASE}_nxn_P0.${FN_SUFFIX}  "NxN - first one"
${SELF_DN_BASE}/gnuplot2d.sh ${FN_BASE}_nxn_P10.${FN_SUFFIX} "NxN - 10%"
${SELF_DN_BASE}/gnuplot2d.sh ${FN_BASE}_nxn_P20.${FN_SUFFIX} "NxN - 20%"
${SELF_DN_BASE}/gnuplot2d.sh ${FN_BASE}_nxn_P30.${FN_SUFFIX} "NxN - 30%"
${SELF_DN_BASE}/gnuplot2d.sh ${FN_BASE}_nxn_P40.${FN_SUFFIX} "NxN - 40%"
${SELF_DN_BASE}/gnuplot2d.sh ${FN_BASE}_nxn_P50.${FN_SUFFIX} "NxN - 50%"
${SELF_DN_BASE}/gnuplot2d.sh ${FN_BASE}_nxn_P60.${FN_SUFFIX} "NxN - 60%"
${SELF_DN_BASE}/gnuplot2d.sh ${FN_BASE}_nxn_P70.${FN_SUFFIX} "NxN - 70%"
${SELF_DN_BASE}/gnuplot2d.sh ${FN_BASE}_nxn_P80.${FN_SUFFIX} "NxN - 80%"

${SELF_DN_BASE}/gnuplot2d.sh ${FN_BASE}_skeleton_P0.${FN_SUFFIX}  "Skeleton - first one"
${SELF_DN_BASE}/gnuplot2d.sh ${FN_BASE}_skeleton_P10.${FN_SUFFIX} "Skeleton - 10%"
${SELF_DN_BASE}/gnuplot2d.sh ${FN_BASE}_skeleton_P20.${FN_SUFFIX} "Skeleton - 20%"
${SELF_DN_BASE}/gnuplot2d.sh ${FN_BASE}_skeleton_P30.${FN_SUFFIX} "Skeleton - 30%"
${SELF_DN_BASE}/gnuplot2d.sh ${FN_BASE}_skeleton_P40.${FN_SUFFIX} "Skeleton - 40%"
${SELF_DN_BASE}/gnuplot2d.sh ${FN_BASE}_skeleton_P50.${FN_SUFFIX} "Skeleton - 50%"
${SELF_DN_BASE}/gnuplot2d.sh ${FN_BASE}_skeleton_P60.${FN_SUFFIX} "Skeleton - 60%"
${SELF_DN_BASE}/gnuplot2d.sh ${FN_BASE}_skeleton_P70.${FN_SUFFIX} "Skeleton - 70%"
${SELF_DN_BASE}/gnuplot2d.sh ${FN_BASE}_skeleton_P80.${FN_SUFFIX} "Skeleton - 80%"

exit 0

