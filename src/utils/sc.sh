#!/bin/sh
# Name:        sc.sh
# Purpose:     Test script for Supertile Simulation System
# Author:      Yunhui Fu
# Created:     2009-03-01
# Modified by:
# RCS-ID:      $Id: $
# Copyright:   (c) 2009 Yunhui Fu
# Licence:     GPL licence version 3
##############################################################################

EXEC_CREATOR=../utils/squarecreator
EXEC_SIMULATOR=../uicli/tscli

FN_LOG=sc.log
FN_RESULT=result.txt

##############################################################################
trap "user_cancel" 2
user_cancel () {
    echo "User canceled @ `date`" >> ${FN_LOG}
    exit 1
}

##############################################################################
# The test times for each of the case:
TEST_TIMES=10

##############################################################################
# Some default value
# the length of the edge of the square
N_MIN=2
N_MAX=16
# the multiply of the target supertiles
M_MIN=128
M_MAX=4096
# percentage of the tiles of the target supertiles vs total number of the tiles
P_MIN=0
P_MAX=80
# the test number
TSTNUM_MIN=0
TSTNUM_MAX=10

##############################################################################
sss_test_group () {
    PARAM_TEST_TYPE=$1
    shift
    PARAM_TIME_STAMP=$1
    shift
    PARAM_TSTNUM_MIN=$1
    shift
    PARAM_TSTNUM_MAX=$1
    shift
    PARAM_N_MIN=$1
    shift
    PARAM_N_MAX=$1
    shift
    PARAM_M_MIN=$1
    shift
    PARAM_M_MAX=$1
    shift
    PARAM_PERCENT_LIST=$1
    shift
    PARAM_P_MIN=0
    PARAM_P_MAX=0

    # other argument parameters for the ${EXEC_SIMULATOR}
    ARG_STG_OTHER=$@

    DN_RESULT=result_${PARAM_TIME_STAMP}
    FN_RESULT=${DN_RESULT}.txt

    TIMES_TILES=${PARAM_M_MIN}
    until [ `expr ${TIMES_TILES} \<= ${PARAM_M_MAX}` = 0 ]; do
        N=${PARAM_N_MIN}
        until [ `expr ${N} \<= ${PARAM_N_MAX}` = 0 ]; do
            FN_HEAD="testcase_N${N}_M${TIMES_TILES}"

            if [ ! -f "${FN_HEAD}_${PARAM_TEST_TYPE}.xml" ]; then
                ${EXEC_CREATOR} -n ${N} -m ${TIMES_TILES} -t 2 -o "${FN_HEAD}"
                if [ ! $? = 0 ]; then
                    echo "Error in ${EXEC_CREATOR}  -n ${N} -m ${TIMES_TILES} -t 2 -o '${FN_HEAD}'" >> ${FN_LOG}
                    exit 1
                fi
            fi

            CUR_TIMES=${PARAM_TSTNUM_MIN}
            until [ `expr ${CUR_TIMES} \<= ${PARAM_TSTNUM_MAX}` = 0 ]; do
                PERCENT=${PARAM_P_MIN}
                until [ `expr ${PERCENT} \<= ${PARAM_P_MAX}` = 0 ]; do

                    echo "${FN_HEAD}_T${CUR_TIMES} P '${PARAM_PERCENT_LIST}'... @ `date`" >> ${FN_LOG}
                    TIME_START=`date +%s`
                    ${EXEC_SIMULATOR} ${ARG_STG_OTHER} -p ${PARAM_PERCENT_LIST} -i "${FN_HEAD}_${PARAM_TEST_TYPE}.xml" -o "result_${FN_HEAD}_${PARAM_TEST_TYPE}_TS${PARAM_TIME_STAMP}_T${CUR_TIMES}" > /dev/null 2>&1
                    RET=$?
                    TIME_END=`date +%s`
                    if [ ! ${RET} = 0 ]; then
                        echo "Error in ${EXEC_SIMULATOR} ${ARG_STG_OTHER} -p '${PARAM_PERCENT_LIST}' -i '${FN_HEAD}_${PARAM_TEST_TYPE}.xml' -o 'result_${FN_HEAD}_${PARAM_TEST_TYPE}_TS${PARAM_TIME_STAMP}_T${CUR_TIMES}'; cost time: `expr ${TIME_END} - ${TIME_START}` Sec" >> ${FN_LOG}
                        exit 1
                    fi
                    echo "cost time: `expr ${TIME_END} - ${TIME_START}` Sec: ${EXEC_SIMULATOR} ${ARG_STG_OTHER} -p '${PARAM_PERCENT_LIST}' -i '${FN_HEAD}_${PARAM_TEST_TYPE}.xml' -o 'result_${FN_HEAD}_${PARAM_TEST_TYPE}_TS${PARAM_TIME_STAMP}_T${CUR_TIMES}'" >> ${FN_LOG}
                    ls result_${FN_HEAD}_${PARAM_TEST_TYPE}_TS${PARAM_TIME_STAMP}_T${CUR_TIMES}*.xml | xargs grep -Hrn "targetsupertile" | grep birth >> ${FN_RESULT}
                    #tar -cjf "result_${FN_HEAD}_${PARAM_TEST_TYPE}_TS${PARAM_TIME_STAMP}_T${CUR_TIMES}.tar.bz2" result_${FN_HEAD}_${PARAM_TEST_TYPE}_TS${PARAM_TIME_STAMP}_T${CUR_TIMES}*.xml
                    ls result_${FN_HEAD}_${PARAM_TEST_TYPE}_TS${PARAM_TIME_STAMP}_T${CUR_TIMES}*.xml | xargs bzip2
                    rm -f result_${FN_HEAD}_${PARAM_TEST_TYPE}_TS${PARAM_TIME_STAMP}_T${CUR_TIMES}*.xml

                    mkdir -p ${DN_RESULT}
                    mv result_${FN_HEAD}*.bz2 ${DN_RESULT}/

                    echo "${FN_HEAD}_T${CUR_TIMES} P '${PARAM_PERCENT_LIST}' DONE! @ `date`" >> ${FN_LOG}
                    PERCENT=`expr ${PERCENT} + 10`
                done

                CUR_TIMES=`expr ${CUR_TIMES} + 1`
            done

            N=`expr ${N} \* 2`
        done

        TIMES_TILES=`expr ${TIMES_TILES} \* 2`
    done
}

##############################################################################

ARG_OTHER=$@

TESTTIME=`date`
echo "+++++++++++++++++ Start SSS testing at '${TESTTIME}' +++++++++++++++++"  >> ${FN_LOG}

TIME_STAMP=`date +%Y%m%d%H%M%S`

#sss_test_group "${TEST_TYPE}" "${TIME_STAMP}" "${TSTNUM_MIN}" "${TSTNUM_MAX}" "${N_MIN}" "${N_MAX}" "${M_MIN}" "${M_MAX}" "${PERCENTAGE_LIST}" "${ARG_OTHER}"

# 1. get one test result for each type of the test
#sss_test_group "skeleton" "${TIME_STAMP}" "1"  "1"     "2"    "16"     "128"    "1024" "0,10,20,30,40,50,60,70,80" ${ARG_OTHER}
sss_test_group "skeleton" "${TIME_STAMP}" "1"  "1"     "2"    "8"     "128"    "1024" "0,10,20,30,40,50,60,70,80" ${ARG_OTHER}
#sss_test_group "skeleton" "${TIME_STAMP}" "1"  "1"     "16"    "16"     "128"    "1024" "0,10,20,30,40,50,60,70,80" ${ARG_OTHER}
sss_test_group "nxn" "${TIME_STAMP}" "1"  "1"     "2"    "16"     "128"    "1024" "0,10,20,30,40,50,60,70,80" ${ARG_OTHER}
sss_test_group "skeleton" "${TIME_STAMP}" "1" "1"     "2"    "16"    "2048"   "65536" "0,10,20,30,40,50,60,70,80" ${ARG_OTHER}
sss_test_group "nxn" "${TIME_STAMP}" "1" "1"     "2"    "16"    "2048"   "65536" "0,10,20,30,40,50,60,70,80" ${ARG_OTHER}
# 2. get other test result of the test loop
sss_test_group "skeleton" "${TIME_STAMP}" "2" "10"     "2"    "16"     "128"    "1024" "0,10,20,30,40,50,60,70,80" ${ARG_OTHER}
sss_test_group "nxn" "${TIME_STAMP}" "2" "10"     "2"    "16"     "128"    "1024" "0,10,20,30,40,50,60,70,80" ${ARG_OTHER}
# 3. get the test result of the N > 16
sss_test_group "skeleton" "${TIME_STAMP}" "1" "10"     "2"    "16"    "2048"   "65536" "0,10,20,30,40,50,60,70,80" ${ARG_OTHER}
sss_test_group "skeleton" "${TIME_STAMP}" "1" "10"    "32"   "128"     "128"   "65536" "0,10,20,30,40,50,60,70,80" ${ARG_OTHER}
sss_test_group "nxn" "${TIME_STAMP}" "1" "10"     "2"    "16"    "2048"   "65536" "0,10,20,30,40,50,60,70,80" ${ARG_OTHER}
sss_test_group "nxn" "${TIME_STAMP}" "1" "10"    "32"   "128"     "128"   "65536" "0,10,20,30,40,50,60,70,80" ${ARG_OTHER}

#grep -Hrn "targetsupertile" *.xml | grep birth > result.txt
#./grep2data.sh result_${TIME_STAMP}.txt input_plot_${TIME_STAMP}.csv && ./data2plot.sh input_plot_${TIME_STAMP}.csv
#The first Birth of the target supertile.
#The Birth of the 10% of target supertile

echo "================= End SSS testing of '${TESTTIME}' at '`date`' ================="  >> ${FN_LOG}
