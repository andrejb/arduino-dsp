#!/bin/sh

for op in cpad vpad mult; do
  if [ "${op}" == "cpad" ]; then
    title="constant pad"
    xlimit=15
  elif [ "${op}" == "vpad" ]; then 
    title="variable pad"
    xlimit=8
  else
    title="mult"
    xlimit=5
  fi
  ./convolution.sh ${op} "${title}" ${xlimit}
  epspdf img/convolution-comparison-${op}.eps 
done
