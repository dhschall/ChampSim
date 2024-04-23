#!/bin/bash

declare -a bench
declare -a prefetch
declare -a btb

ROOT=/users/dschall/ChampSim/
BINARY=/users/dschall/bpu-analysis/build/ChampSim/champsim
BINARY=/users/dschall/bpu-analysis/ChampSim/tracer/cvp/ChampSim/bin/champsim
BINARY=$ROOT/bin/champsim-pc256
BINARY=$ROOT/bin/champsim
# BINARY=/users/dschall/bpu-analysis/ChampSimNew/ChampSim/bin/champsim
# BINARY=/users/dschall/bpu-analysis/ChampSimNew/ChampSim/bin/champsim-spr







####################################################
### General settings



BPUS=""
BPUS="${BPUS} extage64kscl"
BPUS="${BPUS} extage64kinfscl"
# BPUS="${BPUS} extage64kscll2"
# BPUS="${BPUS} extage64ksclA"


# WARMUP_INST=5
# SIM_INST=5
WARMUP_INST=1
SIM_INST=2
WARMUP_INST=100
SIM_INST=500
WARMUP_INST=1
SIM_INST=2


ONLY_GENERATE_COMMANDS=1
CORES=$(nproc)
# CORES=20

RES_DIR="results/CHN_SPR4_4G_16k_M2G6_0c_${SIM_INST}M"
mkdir -p ${RES_DIR}


commands=()

IPC_TRACES=0
GOOGLE_TRACE=0
OWN_TRACE=0
CVP1_TRACE=0
SPEC_TRACE=0

# IPC_TRACES=1
GOOGLE_TRACE=1
OWN_TRACE=1
# CVP1_TRACE=1
# SPEC_TRACE=1




### OWN ###########################################

TRACE_DIR="${ROOT}/traces/own"

bench=()

bench+=( nodeapp-nodeapp )
# bench+=( nodeapp-nodeapp-1 )

bench+=( mwnginxfpm-wiki )
# bench+=( mwnginxfpm-nginx )
bench+=( dacapo-kafka )
bench+=( dacapo-tomcat )
bench+=( dacapo-spring )

bench+=( renaissance-finagle-chirper )
bench+=( renaissance-finagle-http )

bench+=( benchbase-tpcc )
bench+=( benchbase-twitter )
bench+=( benchbase-wikipedia )





max_bench=${#bench[@]}



commands=()

mkdir -p ${RES_DIR}

if [ $OWN_TRACE -eq 1 ]; then

	echo Own max_bench: $max_bench

	for BP in $BPUS; do

		for ((i=0;i<$max_bench;i=i+1)); do

			TRACE=${bench[i]}.champsim.trace.gz

			CMD="${BINARY} --warmup-instructions ${WARMUP_INST}000000 \
						--simulation-instructions ${SIM_INST}000000 \
						--json=${RES_DIR}/${TRACE}-${BP}.json \
						--model=${BP} \
						${TRACE_DIR}/${TRACE} \
									> ${RES_DIR}/${TRACE}-${BP}.txt"

			commands+=("$CMD")
		done
	done
fi



### GOOGLE ########################################

TRACE_DIR="${ROOT}/traces/google"
bench=()

bench+=( charlie.1006511 )
bench+=( charlie.1006518 )
bench+=( charlie.1006522 )
bench+=( charlie.1006531 )
bench+=( charlie.1006555 )
bench+=( charlie.1006618 )
bench+=( charlie.1006704 )
bench+=( charlie.934023 )
bench+=( charlie.940210 )
bench+=( delta.507250 )
bench+=( delta.507251 )
bench+=( delta.507252 )
bench+=( delta.507253 )
bench+=( merced.462778 )
bench+=( merced.467627 )
bench+=( merced.467642 )
bench+=( merced.467652 )
bench+=( merced.467769 )
bench+=( merced.467807 )
bench+=( merced.467822 )
bench+=( merced.467873 )
bench+=( merced.467915 )
bench+=( whiskey.14414 )
bench+=( whiskey.426703 )
bench+=( whiskey.426706 )
bench+=( whiskey.426708 )
bench+=( whiskey.426716 )
bench+=( whiskey.426717 )
bench+=( whiskey.426718 )
bench+=( whiskey.426719 )


bench=()
# bench+=( charlie.1006511 )
bench+=( charlie.1006518 )
# bench+=( charlie.1006522 )
# bench+=( charlie.1006531 )
# bench+=( charlie.1006555 )
# bench+=( charlie.1006618 )
# bench+=( charlie.1006704 )
# bench+=( charlie.934023 )
# bench+=( charlie.940210 )
# bench+=( delta.507250 )
# bench+=( delta.507251 )
bench+=( delta.507252 )
# bench+=( delta.507253 )
# bench+=( merced.462778 )
# bench+=( merced.467627 )
# bench+=( merced.467642 )
# bench+=( merced.467652 )
# bench+=( merced.467769 )
# bench+=( merced.467807 )
# bench+=( merced.467822 )
# bench+=( merced.467873 )
bench+=( merced.467915 )
# bench+=( whiskey.14414 )
# bench+=( whiskey.426703 )
# bench+=( whiskey.426706 )
bench+=( whiskey.426708 )
# bench+=( whiskey.426716 )
# bench+=( whiskey.426717 )
# bench+=( whiskey.426718 )
# bench+=( whiskey.426719 )

max_bench=${#bench[@]}
# max_bench=30
# max_bench=1




if [ $GOOGLE_TRACE -eq 1 ]; then

	echo Google max_bench: $max_bench
	for BP in $BPUS; do

		for ((i=0;i<$max_bench;i=i+1)); do

			TRACE=champsim-${bench[i]}.memtrace.gz

			CMD="${BINARY} --warmup-instructions ${WARMUP_INST}000000 \
						--simulation-instructions ${SIM_INST}000000 \
						--json=${RES_DIR}/${TRACE}-${BP}.json \
						--model=${BP} \
						${TRACE_DIR}/${TRACE} \
									> ${RES_DIR}/${TRACE}-${BP}.txt"

			commands+=("$CMD")
		done
	done
fi





##### CVP-1 ########################################

TRACE_DIR="${ROOT}/traces/CVP1public"
bench=()

# CVP-1 public traces
bench=("srv_0" "srv_1" "srv_2" "srv_3" "srv_4")
bench+=( "srv_5" "srv_6" "srv_7" "srv_8" "srv_9" "srv_10" "srv_11" "srv_12" "srv_13" "srv_14" "srv_15" "srv_16" "srv_17" "srv_18" "srv_19" "srv_20" "srv_21" "srv_22" "srv_23" "srv_24" "srv_25" "srv_26" "srv_27" "srv_28" "srv_29" "srv_30" "srv_31" "srv_32" "srv_33" "srv_34" "srv_35" "srv_36" "srv_37" "srv_38" "srv_39" "srv_40" "srv_41" "srv_42" "srv_43" "srv_44" "srv_45" "srv_46" "srv_47" "srv_48" "srv_49" "srv_50" "srv_51" "srv_52" "srv_53" "srv_54" "srv_55" "srv_56" "srv_57" "srv_58" "srv_59" "srv_60" "srv_61" "srv_62" "srv_63" "srv_64" "srv_65" "srv_66" "srv_67" "srv_68" "srv_69" "srv_70" "srv_71" "srv_72" "srv_73" "srv_74" "srv_75" "srv_76" "compute_fp_1" "compute_fp_2" "compute_fp_3" "compute_fp_4" "compute_fp_5" "compute_fp_6" "compute_fp_7" "compute_fp_8" "compute_fp_9" "compute_fp_10" "compute_fp_11" "compute_fp_12" "compute_fp_13" "compute_int_0" "compute_int_1" "compute_int_2" "compute_int_3" "compute_int_4" "compute_int_5" "compute_int_6" "compute_int_7" "compute_int_8" "compute_int_9" "compute_int_10" "compute_int_11" "compute_int_12" "compute_int_13" "compute_int_14" "compute_int_15" "compute_int_16" "compute_int_17" "compute_int_18" "compute_int_19" "compute_int_21" "compute_int_22" "compute_int_23" "compute_int_24" "compute_int_25" "compute_int_26" "compute_int_27" "compute_int_28" "compute_int_29" "compute_int_31" "compute_int_32" "compute_int_33" "compute_int_34" "compute_int_35" "compute_int_36" "compute_int_37" "compute_int_38" "compute_int_39" "compute_int_40" "compute_int_41" "compute_int_42" "compute_int_43" "compute_int_44" "compute_int_45" "compute_int_46")


max_bench=${#bench[@]}


if [ $CVP1_TRACE -eq 1 ]; then

	echo CVP-1 max_bench: $max_bench

	for BP in $BPUS; do


		for ((i=0;i<$max_bench;i=i+1)); do

			TRACE=${bench[i]}.champsimtrace.xz

			# CMD="(${BINARY} -warmup_instructions ${WARMUP_INST}000000 \
			# 			-simulation_instructions ${SIM_INST}000000 \
			# 			-bpmodel ${BP} \
			# 			-traces ${TRACE_DIR}/${TRACE}) \
			# 						&> ${RES_DIR}/${TRACE}-${BP}.txt"

			CMD="(${BINARY} --warmup-instructions ${WARMUP_INST}000000 \
						--simulation-instructions ${SIM_INST}000000 \
						${TRACE_DIR}/${TRACE}) \
									&> ${RES_DIR}/${TRACE}-${BP}.txt"

			commands+=("$CMD")
		done
	done
fi




##### IPC-1 ########################################

TRACE_DIR="${ROOT}/traces/IPC1"
bench=()

# IPC-1 traces
# bench+=("client_001" "client_002" "client_003" "client_004" "client_005" "client_006" "client_007" "client_008")
# # bench+=("client_001" "client_002" "client_003" "client_006" "client_007" "client_008")
# bench+=("server_001" "server_002" "server_003" "server_004" "server_009" "server_010" "server_011" "server_012" "server_013" "server_014" "server_015" "server_016" "server_017" "server_018" "server_019" "server_020" "server_021" "server_022" "server_023" "server_024" "server_025" "server_026" "server_027" "server_028" "server_029" "server_030" "server_031" "server_032" "server_033" "server_034" "server_035" "server_036" "server_037" "server_038" "server_039")
bench+=("spec_gcc_001" "spec_gcc_002" "spec_gcc_003" "spec_gobmk_001" "spec_gobmk_002" "spec_perlbench_001" "spec_x264_001")


max_bench=${#bench[@]}


if [ $IPC_TRACES -eq 1 ]; then

	echo IPC-1 max_bench: $max_bench

	for BP in $BPUS; do
		for ((i=0;i<$max_bench;i=i+1)); do

			TRACE=${bench[i]}.champsimtrace.xz

			# CMD="(${BINARY} -warmup_instructions ${WARMUP_INST}000000 \
			# 			-simulation_instructions ${SIM_INST}000000 \
			# 			-bpmodel ${BP} \
			# 			-traces ${TRACE_DIR}/${TRACE}) \
			# 						&> ${RES_DIR}/${TRACE}-${BP}.txt"

			CMD="(${BINARY} --warmup-instructions ${WARMUP_INST}000000 \
						--simulation-instructions ${SIM_INST}000000 \
						${TRACE_DIR}/${TRACE}) \
									&> ${RES_DIR}/${TRACE}-${BP}.txt"



			commands+=("$CMD")

		done
	done
fi





### SPEC ########################################

TRACE_DIR="${ROOT}/traces/spec"

# 600.perlbench_s-210B.champsimtrace.xz
# 602.gcc_s-734B.champsimtrace.xz
# 605.mcf_s-665B.champsimtrace.xz
# 620.omnetpp_s-874B.champsimtrace.xz
# 623.xalancbmk_s-700B.champsimtrace.xz
# 625.x264_s-18B.champsimtrace.xz
# 631.deepsjeng_s-928B.champsimtrace.xz
# 641.leela_s-800B.champsimtrace.xz
# 648.exchange2_s-1699B.champsimtrace.xz
# 657.xz_s-3167B.champsimtrace.xz

simp=()
simp+=( 600.perlbench_s-210B )
simp+=( 602.gcc_s-734B )
simp+=( 605.mcf_s-665B )
simp+=( 620.omnetpp_s-874B )
simp+=( 623.xalancbmk_s-700B )
simp+=( 625.x264_s-18B )
simp+=( 631.deepsjeng_s-928B )
simp+=( 641.leela_s-800B )
simp+=( 648.exchange2_s-1699B )
simp+=( 657.xz_s-3167B )

bench=()

bench+=( 600.perlbench_s )
bench+=( 602.gcc_s )
bench+=( 605.mcf_s )
bench+=( 620.omnetpp_s )
bench+=( 623.xalancbmk_s )
bench+=( 625.x264_s )
bench+=( 631.deepsjeng_s )
bench+=( 641.leela_s )
bench+=( 648.exchange2_s )
bench+=( 657.xz_s )


max_bench=${#bench[@]}
# max_bench=30
# max_bench=1



if [ $SPEC_TRACE -eq 1 ]; then

	echo SPEC max_bench: $max_bench

	for BP in $BPUS; do

		for ((i=0;i<$max_bench;i=i+1)); do

			TRACE=champsim-${bench[i]}.memtrace.gz


			CMD="${BINARY} --warmup-instructions ${WARMUP_INST}000000 \
						--simulation-instructions ${SIM_INST}000000 \
						--json=${RES_DIR}/${bench[i]}-${BP}.json \
						--model=${BP} \
						${TRACE_DIR}/${bench[i]}/${simp[i]}.champsimtrace.xz \
									> ${RES_DIR}/${bench[i]}-${BP}.txt"

			commands+=("$CMD")
		done
	done
fi









if [ $ONLY_GENERATE_COMMANDS -eq 1 ]; then

	echo "" > commands.txt

	for ((i=0;i<${#commands[@]};i=i+1)); do
		echo ${commands[i]} >> commands.txt
	done

	exit 0
fi


parallel -j $CORES ::: "${commands[@]}"