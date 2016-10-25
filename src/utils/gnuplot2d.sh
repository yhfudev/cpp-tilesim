#!/bin/sh
# Name:        gnuplot2d.sh
# Purpose:     draw the picture by gnuplot
# Author:      Yunhui Fu
# Created:     2009-03-01
# Modified by:
# RCS-ID:      $Id: $
# Copyright:   (c) 2009 Yunhui Fu
# Licence:     GPL licence version 3
##############################################################################

FN_BASE=`echo $1 | awk -F. '{print $1}'`
FN_SUFFIX=`echo $1 | awk -F. '{print $2}'`

FN_AVG_NB_M=${FN_BASE}_avg_NB_M.${FN_SUFFIX}
FN_AVG_MB_N=${FN_BASE}_avg_MB_N.${FN_SUFFIX}

gnuplot << EOF

set terminal postscript eps color enhanced
set output "${FN_BASE}_avg_NB_M.eps"
#set terminal png small
#set output "${FN_BASE}_avg_NB_M.png"

set grid nopolar
set grid xtics nomxtics ytics nomytics noztics nomztics \
 nox2tics nomx2tics noy2tics nomy2tics nocbtics nomcbtics
set grid layerdefault   linetype 0 linewidth 1.000,  linetype 0 linewidth 1.000
set ytics border in scale 1,0.5 mirror norotate  offset character 0, 0, 0 

set title "$2"
set lmargin 9
set rmargin 2

#set xlabel "M (Population)"
set xlabel "N (Edge Len)"
set ylabel "Birth"

set xtics (2,3,4,5,6,7,8,16)
#set ytics   (80.0000, 85.0000, 90.0000, 95.0000, 100.000, 105.000)
#set xrange [ 9.0 : 1100.000 ] noreverse nowriteback
#set yrange [ 0.0000 : 30.000 ] noreverse nowriteback
#set yrange [ 0.01 : 100.000 ] noreverse nowriteback

#set logscale x
#set logscale y
#set logscale xy

set key left top

plot "$1" using 2:6 t 'Birth' with points, \
  "${FN_AVG_NB_M}" index 0 using 1:5 t "Avg Birth" with lp

# save to png
set terminal png small
set output "${FN_BASE}_avg_NB_M.png"
plot "$1" using 2:6 t 'Birth' with points, \
  "${FN_AVG_NB_M}" index 0 using 1:5 t "Avg Birth" with lp

EOF

gnuplot << EOF

set terminal postscript eps color enhanced
set output "${FN_BASE}_avg_MB_N.eps"
#set terminal png small
#set output "${FN_BASE}_avg_MB_N.png"

set grid nopolar
set grid xtics nomxtics ytics nomytics noztics nomztics \
 nox2tics nomx2tics noy2tics nomy2tics nocbtics nomcbtics
set grid layerdefault   linetype 0 linewidth 1.000,  linetype 0 linewidth 1.000
set ytics border in scale 1,0.5 mirror norotate  offset character 0, 0, 0 

set title "$2"
set lmargin 9
set rmargin 2

set xlabel "M (Population)"
#set xlabel "N (Edge Len)"
set ylabel "Birth"

#set xtics (10,100,1000)
#set ytics   (80.0000, 85.0000, 90.0000, 95.0000, 100.000, 105.000)
#set xrange [ 9.0 : 1100.000 ] noreverse nowriteback
#set yrange [ 0.0000 : 30.000 ] noreverse nowriteback
#set yrange [ 0.01 : 100.000 ] noreverse nowriteback

#set logscale x
#set logscale xy

set key left top

plot "$1" using 3:6 t 'Birth' with points, "${FN_AVG_MB_N}" using 2:5 t 'Avg Birth' with lp

set terminal png small
set output "${FN_BASE}_avg_MB_N.png"
plot "$1" using 3:6 t 'Birth' with points, "${FN_AVG_MB_N}" using 2:5 t 'Avg Birth' with lp

EOF

