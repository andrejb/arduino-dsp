#!/bin/sh

BS="${1}"
FREQ="${2}"
CYCLE="${3}"
CEIL="${4}"

gnuplot <<EOF
set terminal postscript enhanced eps color
set output "img/operations-${BS}-${FREQ}.eps"
set size 0.5,0.6
set key below
set rmargin 3

#set title "Sintese aditiva usando diferentes tipos de operacao (R=${FREQ}/N=${BS})"
set title "Sintese aditiva usando diferentes tipos de operacao"
set xlabel "Numero de osciladores"
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

set yrange [0:${CEIL}]
set xrange [0:15]
#set log x 2

plot "blocksize-${BS}-${FREQ}-2op.txt" using 1:2 title "usando multiplicacao e divisao" with linespoints lt 1 pt 7 ps 0.7 lc 7 lw 3, \
     "blocksize-${BS}-${FREQ}-1op.txt" using 1:2 title "usando somente divisao" with linespoints lt 1 pt 7 ps 0.7 lc 1 lw 3, \
     "../pad/blocksize-${BS}-${FREQ}.txt" using 1:2 title "usando bit shifting variavel" with linespoints lt 1 pt 7 ps 0.7 lc 3 lw 3, \
     ${CYCLE} title "periodo teorico do ciclo DSP" with lines lt 2 lc 0 lw 3
#     ${CYCLE} notitle with lines lt 2 pt 7 ps 0.7 lc 0 lw 3
EOF
