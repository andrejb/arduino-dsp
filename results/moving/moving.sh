#!/bin/sh


gnuplot <<EOF
set terminal postscript enhanced eps color
set output "img/moving.eps"
set size 0.5,0.55
set key under
set rmargin 3

set title "Moving Average Frequency Response"
set xlabel "Frequency"
set ylabel "Amplitude"

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

set yrange [0:1]
set xrange [0:3.15]
#set log x 2

plot "moving.txt"  using 1:2 title "order 2"  with lines ls 1 lc 0 lw 3, \
     "moving.txt"  using 1:3 title "order 4"  with lines ls 1 lc 1 lw 3, \
     "moving.txt"  using 1:4 title "order 8"  with lines ls 1 lc 3 lw 3, \
     "moving.txt"  using 1:5 title "order 16"  with lines ls 1 lc 8 lw 3
EOF
