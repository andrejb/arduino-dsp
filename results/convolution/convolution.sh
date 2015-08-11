#!/bin/sh

OP=$1
TITLE=$2
XLIMIT=$3
KEY=$4
HEIGHT=$5

gnuplot <<EOF
set terminal postscript enhanced eps color
set output "img/convolution-comparison-${OP}.eps"
set size 0.5,${HEIGHT}
set key ${KEY}
set rmargin 3

set title "Convolucao no dominio do tempo (${TITLE})"
set xlabel "Ordem do filtro"
set ylabel "Tempo de sintese (ms)"

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

plot "blocksize-32-${OP}.txt"  using 1:2 title "bloco de 32 amostras" with linespoints lt 1 pt 7 ps 0.7 lc 0 lw 3, \
     1.024 notitle with lines ls 2 lw 3 lc 0, \
     "blocksize-64-${OP}.txt"  using 1:2 title "bloco de 64 amostras" with linespoints lt 1 pt 7 ps 0.7 lc 1 lw 3, \
     2.048 notitle with lines ls 2 lw 3 lc 1, \
     "blocksize-128-${OP}.txt" using 1:2 title "bloco de 128 amostras" with linespoints lt 1 pt 7 ps 0.7 lc 3 lw 3, \
     4.096 notitle with lines ls 2 lw 3 lc 3, \
     "blocksize-256-${OP}.txt" using 1:2 title "bloco de 256 amostras" with linespoints lt 1 pt 7 ps 0.7 lc 13 lw 3, \
     8.192 notitle with lines ls 2 lw 3 lc 13
EOF
