#!/bin/sh
# Name:        gnuplot3d.sh
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

gnuplot << EOF
set terminal postscript eps color enhanced
#set dgrid3d
#set contour both
#set style data points
#set style data impulses
#set style data boxerrorbars
#set style data fsteps
#set border -1 front linetype -1 linewidth 1.000
#set autoscale
#set parametric

#set zrange [ 1 : 100000000.000 ] noreverse nowriteback

#set logscale z
set logscale x
set logscale y

set ticslevel 0
set grid xtics ytics ztics
set xtics (128,256,512,1024,2048,4096,8192,16384,32768,65536)
set ytics (2,3,4,5,6,7,8,16)
set xyplane at 0.0

set output "${FN_BASE}.eps"
set xlabel "M (Population)"
set ylabel "N (Edge Len)"
set zlabel "Birth"
set title "$2"
set view 65, 335
#set view 80, 335
#set size 0.5,0.5
#set origin 0.0,0.0

splot "$1" using 3:2:6 t 'Birth'

# save to png
set terminal png small
set output "${FN_BASE}.png"
splot "$1" using 3:2:6 t 'Birth'


EOF
