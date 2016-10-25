#!/bin/sh
FN_SDB=$1
FN_SRC=$2

#sqlinit.sh tsim.sdb ../utils/tilesim_init.sql
sqlite3 -batch ${FN_SDB} < ${FN_SRC}

