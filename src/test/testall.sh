#!/bin/sh
# Name:        testall.sh
# Purpose:     test all of test cases. this script could be used when some changes occurs in the source directory.
# Author:      Yunhui Fu
# Created:     2009-08-29
# Modified by:
# RCS-ID:      $Id: $
# Copyright:   (c) 2009 Yunhui Fu
# Licence:     GPL licence version 3
##############################################################################
TOPDIR="../.."

testcase () {
    index=0
    for param in "-b" "-g" "-h" "-s" "-g -h" "-g -s" "-s -h" ; do
        echo ${param}
        ${TOPDIR}/src/uicli/tscli ${param} -i $1 -o aaa
        if [ ! $? = 0 ]; then
            echo "Error in test ${param} -i '$1' "
        fi
    done
}

# create the test case xml data files
${TOPDIR}/src/utils/cubecreator -n 2 -m 100 -o ${TOPDIR}/src/test/testcube_2_100
${TOPDIR}/src/utils/cubecreator -n 4 -m 100 -o ${TOPDIR}/src/test/testcube_4_100
${TOPDIR}/src/utils/cubecreator -n 8 -m 100 -o ${TOPDIR}/src/test/testcube_8_100
${TOPDIR}/src/utils/cubecreator -n 16 -m 100 -o ${TOPDIR}/src/test/testcube_16_100
${TOPDIR}/src/utils/cubecreator -n 32 -m 100 -o ${TOPDIR}/src/test/testcube_32_100

${TOPDIR}/src/utils/squarecreator -n 2 -m 100 -o ${TOPDIR}/src/test/testsquare_2_100
${TOPDIR}/src/utils/squarecreator -n 4 -m 100 -o ${TOPDIR}/src/test/testsquare_4_100
${TOPDIR}/src/utils/squarecreator -n 8 -m 100 -o ${TOPDIR}/src/test/testsquare_8_100
${TOPDIR}/src/utils/squarecreator -n 16 -m 100 -o ${TOPDIR}/src/test/testsquare_16_100
${TOPDIR}/src/utils/squarecreator -n 32 -m 100 -o ${TOPDIR}/src/test/testsquare_32_100

# run test cases
testcase ${TOPDIR}/src/test/testcube_2_100_skeleton.xml
testcase ${TOPDIR}/src/test/testcube_4_100_skeleton.xml
testcase ${TOPDIR}/src/test/testcube_8_100_skeleton.xml
testcase ${TOPDIR}/src/test/testcube_16_100_skeleton.xml
testcase ${TOPDIR}/src/test/testcube_32_100_skeleton.xml

testcase ${TOPDIR}/src/test/testsquare_2_100_skeleton.xml
testcase ${TOPDIR}/src/test/testsquare_4_100_skeleton.xml
testcase ${TOPDIR}/src/test/testsquare_8_100_skeleton.xml
testcase ${TOPDIR}/src/test/testsquare_16_100_skeleton.xml
testcase ${TOPDIR}/src/test/testsquare_32_100_skeleton.xml


