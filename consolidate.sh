#!/bin/sh

export LD_LIBRARY_PATH="/usr/local/lib"
export AFL_SKIP_CPUFREQ=1

rm -r temp;
mkdir temp;

for f in $(find outputs -name "id*") corpus/*; do
    h=$(sha1sum $f|cut -f 1 -d " ");
    cp $f temp/$h;
done;

rm -r corpus outputs;

afl-cmin -i ./temp -o ./corpus ./run_script @@

