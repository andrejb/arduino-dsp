#!/bin/sh

for bs in 32 64 128; do
  for freq in 15625 31250; do
    cycle=`python -c "print ${bs}.0 / ${freq} * 1000"`
    ceil=`python -c "import math; print math.ceil(${cycle})"`
    ./operations.sh ${bs} ${freq} ${cycle} ${ceil}
    epspdf img/operations-${bs}-${freq}.eps
  done
done

#for bs in 32 64 128; do
#  for op in 1 2; do
#    ./frequencies.sh ${bs} ${op}
#    epspdf img/frequencies-${bs}-${op}.eps
#  done
#done

#for bs in 32 64 128; do
#  ./frequencies-all.sh ${bs}
#  epspdf img/frequencies-all-${bs}.eps
#done
