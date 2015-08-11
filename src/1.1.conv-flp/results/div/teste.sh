#!/bin/sh

FILE="128.txt"

gnuplot <<EOF
set terminal postscript enhanced eps color
set output "basic-integer-ops.eps"
set size 0.5,0.55
set key below
set rmargin 3

set title "Tempo de diferentes operacoes sobre inteiros"
set xlabel "Valor do argumento da operacao"
set ylabel "Tempo de calculo (ms)"

#set xtics (        \
#  "..." 64,      \
#  "" 128,          \
#  "" 256,          \
#  "" 512,          \
#  "" 1024,         \
#  "2048" 2048,         \
#  "4096" 4096,   \
#  "8192" 8192 )
#set grid
#set xtics 1
#set ytics 1

#set yrange [0:${CEIL}]
#set xrange [0:15]
#set log x 2

plot "128.txt" using 1:2 title "div" with linespoints lt 1 pt 7 ps 0.7 lc 7 lw 3, \
  "128.txt" using 1:3 title "mult" with linespoints lt 1 pt 7 ps 0.7 lc 1 lw 3, \
  "128.txt" using 1:4 title "div-pad" with linespoints lt 1 pt 7 ps 0.7 lc 8 lw 3, \
  "128.txt" using 1:5 title "mult-pad" with linespoints lt 0 pt 7 ps 0.7 lc 3 lw 3
EOF
