#!/bin/bash
source  ../build.sh
make 
timer ../results.csv c libcsv fieldcount "./csv /tmp/hello.csv"
timer ../results.csv c libcsv empty "./csv /tmp/empty.csv"
