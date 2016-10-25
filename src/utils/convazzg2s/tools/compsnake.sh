#!/bin/sh
EXEC_CONVKTAM=../convktam
EXEC_CONVZZG2S=../convzzg2s
EXEC_XGROW=../../../../../reference/xgrow/xgrow

FLG_GEN_DATA=1
#PARAMS_RUN="Gmc=13.6 Gse=7.0"
PARAMS_RUN="Gmc=13.6 Gse=12"
PARAMS_K_SNAKE=6
PARAMS_K_2D1T=4

if [ ${FLG_GEN_DATA} = 1 ]; then
rm *.tdp *.tds *.tiles
# generate parity data
${EXEC_CONVKTAM} -e -w -k ${PARAMS_K_SNAKE} -s 10101000

# convert 2d2t to 2d1t
${EXEC_CONVZZG2S} -1 -w -k ${PARAMS_K_2D1T} -i parity_example.tdp -o parity_example_2d1t_${PARAMS_K_2D1T}

fi

# run the 2t snake version of the parity system
${EXEC_XGROW} parity_example_snake_${PARAMS_K_SNAKE}.tiles ${PARAMS_RUN} &

# run the 1t version of the parity system
${EXEC_XGROW} parity_example_2d1t_${PARAMS_K_2D1T}_2d1t.tiles ${PARAMS_RUN} &
