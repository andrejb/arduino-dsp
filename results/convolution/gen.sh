#!/bin/sh

for op in cpad vpad div; do
  if [ "${op}" == "cpad" ]; then
    title="bit-shifting const."
    xlimit=15
    key="above horizontal"
    height=0.64
  elif [ "${op}" == "vpad" ]; then 
    title="bit-shifting var."
    xlimit=15
    key="off"
    height=0.5
    #key="below horizontal"
    #height=0.635
  else
    title="mult/div"
    xlimit=15
    key="off"
    height=0.5
    #key="below horizontal"
    #height=0.635
  fi
  ./convolution.sh "${op}" "${title}" ${xlimit} "${key}" ${height}
  epspdf img/convolution-comparison-${op}.eps 
done
