#!/bin/sh

export LD_LIBRARY_PATH="/usr/local/lib"
export AFL_SKIP_CPUFREQ=1

killall afl-fuzz;

./consolidate.sh

mkdir outputs;

nohup afl-fuzz -i corpus/ -o outputs -M fuzzer ./run_script @@ >/dev/null 2>/dev/null &
declate -i NPROC;
NPROC=$(nproc)*2
for i in $(seq 1 $NPROC); do
    nohup afl-fuzz -i corpus/ -o outputs -S fuzzer$i ./run_script @@ >/dev/null 2>/dev/null &
done;
