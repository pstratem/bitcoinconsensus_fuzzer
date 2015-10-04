#!/bin/sh

export LD_LIBRARY_PATH="/usr/local/lib"
export AFL_SKIP_CPUFREQ=1

./consolidate.sh

mkdir outputs;

nohup afl-fuzz -i corpus/ -o outputs -M fuzzer ./run_script @@ >/dev/null 2>/dev/null &
for i in $(seq 1 $(nproc)); do
    nohup afl-fuzz -i corpus/ -o outputs -S fuzzer$i ./run_script @@ >/dev/null 2>/dev/null &
done;
