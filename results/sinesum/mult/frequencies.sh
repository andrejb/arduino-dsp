#!/bin/sh

BS="${1}"
OP="${2}"
CYCLE="${3}"
CEIL="${4}"

gnuplot <<EOF
set terminal postscript enhanced eps color
set output "img/frequencies-${BS}-${OP}.eps"
set size 0.5,0.55
set key below
set rmargin 3

set title "Sampling rate in sinesum (bs: ${BS}, op: ${OP})"
set xlabel "Number of oscilators"
set ylabel "Synth time (ms)"

#set xtics (        \
#  "..." 64,      \
#  "" 128,          \
#  "" 256,          \
#  "" 512,          \
#  "" 1024,         \
#  "2048" 2048,         \
#  "4096" 4096,   \
#  "8192" 8192 )
set grid
set xtics 1
set ytics 1

set yrange [0:10]
set xrange [0:8]
#set log x 2

plot "blocksize-${BS}-15625-${OP}op.txt" using 1:2 title "15.625 Hz" with linespoints ls 1 pt 2 lc 8 lw 3, \
     (${BS}.0/15.625) notitle with lines ls 2 lc 8 lw 3, \
     "blocksize-${BS}-31250-${OP}op.txt" using 1:2 title "31.250 Hz" with linespoints ls 1 pt 2 lc 0 lw 3, \
     (${BS}.0/31.250) notitle with lines ls 2 lc 0 lw 3
EOF

#     (${BS}.0/15625) notitle with linespoints lc 3 lw 3 ls 2, \
