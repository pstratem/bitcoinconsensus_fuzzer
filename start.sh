#!/bin/sh

export LD_LIBRARY_PATH="/usr/local/lib"
export AFL_SKIP_CPUFREQ=1

./consolidate.sh

mkdir outputs;

while afl-gotcpu; do
    nohup afl-fuzz -i corpus/ -o outputs -M fuzzer$i ./run_script @@ >/dev/null 2>/dev/null &
done;
