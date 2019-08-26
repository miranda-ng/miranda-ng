#!/bin/bash
if ! which make cc c++ tee lz4 >/dev/null; then
	echo "Please install the following prerequisites: make cc c++ tee lz4" >&2
	exit 1
fi

set -euo pipefail

UNAME="$(uname -s 2>/dev/null || echo Unknown)"
case ${UNAME} in
	Linux)
		MAKE=make
		if [[ ! -v TESTDB_DIR || -z "$TESTDB_DIR" ]]; then
			TESTDB_DIR="/dev/shm/mdbx-test.$$"
		fi
		mkdir -p $TESTDB_DIR && rm -f $TESTDB_DIR/*
		if LC_ALL=C free | grep -q -i available; then
			ram_avail_mb=$(($(LC_ALL=C free | grep -i Mem: | tr -s [:blank:] ' ' | cut -d ' ' -f 7) / 1024))
		else
			ram_avail_mb=$(($(LC_ALL=C free | grep -i Mem: | tr -s [:blank:] ' ' | cut -d ' ' -f 4) / 1024))
		fi
	;;
	FreeBSD)
		MAKE=gmake
		if [[ ! -v TESTDB_DIR || -z "$TESTDB_DIR" ]]; then
			for old_test_dir in $(ls -d /tmp/mdbx-test.[0-9]*); do
				umount $old_test_dir && rm -r $old_test_dir
			done
			TESTDB_DIR="/tmp/mdbx-test.$$"
			rm -rf $TESTDB_DIR && mkdir -p $TESTDB_DIR && mount -t tmpfs tmpfs $TESTDB_DIR
		else
			mkdir -p $TESTDB_DIR && rm -f $TESTDB_DIR/*
		fi
		ram_avail_mb=$(($(LC_ALL=C vmstat -s | grep -ie '[0-9] pages free$' | cut -d p -f 1) * ($(LC_ALL=C vmstat -s | grep -ie '[0-9] bytes per page$' | cut -d b -f 1) / 1024) / 1024))
	;;
	*)
		echo "FIXME: ${UNAME} not supported by this script"
		exit 2
	;;
esac

echo "=== ${ram_avail_mb}M RAM available"
ram_reserve4logs_mb=1234
if [ $ram_avail_mb -lt $ram_reserve4logs_mb ]; then
	echo "=== At least ${ram_reserve4logs_mb}Mb RAM required"
	exit 3
fi

#
# В режимах отличных от MDBX_WRITEMAP изменения до записи в файл
# будут накапливаться в памяти, что может потребовать свободной
# памяти размером с БД. Кроме этого, в тест входит сценарий
# создания копия БД на ходу. Поэтому БД не может быть больше 1/3
# от доступной памяти. Однако, следует учесть что malloc() будет
# не сразу возвращать выделенную память системе, а также
# предусмотреть места для логов.
#
# In non-MDBX_WRITEMAP modes, updates (dirty pages) will
# accumulate in memory before writing to the disk, which may
# require a free memory up to the size of a whole database. In
# addition, the test includes a script create a copy of the
# database on the go. Therefore, the database cannot be more 1/3
# of available memory. Moreover, should be taken into account
# that malloc() will not return the allocated memory to the
# system immediately, as well some space is required for logs.
#
db_size_mb=$(((ram_avail_mb - ram_reserve4logs_mb) / 4))
if [ $db_size_mb -gt 3072 ]; then
	db_size_mb=3072
fi
echo "=== use ${db_size_mb}M for DB"

${MAKE} TESTDB=${TESTDB_DIR}/smoke.db TESTLOG=${TESTDB_DIR}/smoke.log check
rm -f ${TESTDB_DIR}/*

###############################################################################

function rep9 { printf "%*s" $1 '' | tr ' ' '9'; }
function join { local IFS="$1"; shift; echo "$*"; }
function bit2option { local -n arr=$1; (( ($2&(1<<$3)) != 0 )) && echo -n '+' || echo -n '-'; echo "${arr[$3]}"; }

options=(writemap coalesce lifo)

function bits2list {
	local -n arr=$1
	local i
	local list=()
	for ((i=0; i<${#arr[@]}; ++i)) do
		list[$i]=$(bit2option $1 $2 $i)
	done
	join , "${list[@]}"
}

function probe {
	echo "=============================================== $(date)"
	echo "${caption}: $*"
	rm -f ${TESTDB_DIR}/* \
		&& ./mdbx_test --ignore-dbfull --repeat=42 --pathname=${TESTDB_DIR}/long.db "$@" | lz4 > ${TESTDB_DIR}/long.log.lz4 \
		&& ./mdbx_chk -nvvv ${TESTDB_DIR}/long.db | tee ${TESTDB_DIR}/long-chk.log \
		&& ([ ! -e ${TESTDB_DIR}/long.db-copy ] || ./mdbx_chk -nvvv ${TESTDB_DIR}/long.db-copy | tee ${TESTDB_DIR}/long-chk-copy.log) \
		|| (echo "FAILED"; exit 1)
}

###############################################################################

count=0
for nops in $(seq 2 6); do
	for ((wbatch=nops-1; wbatch > 0; --wbatch)); do
		loops=$(((111 >> nops) / nops + 3))
		for ((rep=0; rep++ < loops; )); do
			for ((bits=2**${#options[@]}; --bits >= 0; )); do
				seed=$(($(date +%s) + RANDOM))
				caption="Probe #$((++count)) int-key,w/o-dups, repeat ${rep} of ${loops}" probe \
					--pagesize=min --size-upper=${db_size_mb}M --table=+key.integer,-data.dups --keylen.min=min --keylen.max=max --datalen.min=min --datalen.max=1111 \
					--nops=$( rep9 $nops ) --batch.write=$( rep9 $wbatch ) --mode=$(bits2list options $bits) \
					--keygen.seed=${seed} basic
				caption="Probe #$((++count)) int-key,with-dups, repeat ${rep} of ${loops}" probe \
					--pagesize=min --size-upper=${db_size_mb}M --table=+key.integer,+data.dups --keylen.min=min --keylen.max=max --datalen.min=min --datalen.max=max \
					--nops=$( rep9 $nops ) --batch.write=$( rep9 $wbatch ) --mode=$(bits2list options $bits) \
					--keygen.seed=${seed} basic
				caption="Probe #$((++count)) int-key,int-data, repeat ${rep} of ${loops}" probe \
					--pagesize=min --size-upper=${db_size_mb}M --table=+key.integer,+data.integer --keylen.min=min --keylen.max=max --datalen.min=min --datalen.max=max \
					--nops=$( rep9 $nops ) --batch.write=$( rep9 $wbatch ) --mode=$(bits2list options $bits) \
					--keygen.seed=${seed} basic
				caption="Probe #$((++count)) w/o-dups, repeat ${rep} of ${loops}" probe \
					--pagesize=min --size-upper=${db_size_mb}M --table=-data.dups --keylen.min=min --keylen.max=max --datalen.min=min --datalen.max=1111 \
					--nops=$( rep9 $nops ) --batch.write=$( rep9 $wbatch ) --mode=$(bits2list options $bits) \
					--keygen.seed=${seed} basic
				caption="Probe #$((++count)) with-dups, repeat ${rep} of ${loops}" probe \
					--pagesize=min --size-upper=${db_size_mb}M --table=+data.dups --keylen.min=min --keylen.max=max --datalen.min=min --datalen.max=max \
					--nops=$( rep9 $nops ) --batch.write=$( rep9 $wbatch ) --mode=$(bits2list options $bits) \
					--keygen.seed=${seed} basic
			done
		done
	done
done

echo "=== ALL DONE ====================== $(date)"
