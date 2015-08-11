#!/bin/sh

OP=$1
TITLE=$2
XLIMIT=$3

gnuplot <<EOF
set terminal postscript enhanced eps color
set output "img/convolution-comparison-${OP}.eps"
set size 0.5,0.55
set key under
set rmargin 3

set title "Time-domain convolution on Arduino (${TITLE})"
set xlabel "Order of the filter"
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

set yrange [0:9]
set xrange [0:${XLIMIT}]
#set log x 2

plot "blocksize-32-${OP}.txt"  using 1:2 title "bl. size 32"  with linespoints ls 1 pt 2 lc 0 lw 3, \
     1.024 notitle with lines ls 2 lw 3 lc 0, \
     "blocksize-64-${OP}.txt"  using 1:2 title "bl. size 64"  with linespoints ls 1 pt 2 lc 1 lw 3, \
     2.048 notitle with lines ls 2 lw 3 lc 1, \
     "blocksize-128-${OP}.txt" using 1:2 title "bl. size 128" with linespoints ls 1 pt 2 lc 3 lw 3, \
     4.096 notitle with lines ls 2 lw 3 lc 3, \
     "blocksize-256-${OP}.txt" using 1:2 title "bl. size 256" with linespoints ls 1 pt 2 lc 8 lw 3, \
     8.192 notitle with lines ls 2 lw 3 lc 8
EOF
