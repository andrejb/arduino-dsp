#!/bin/sh

COMP=$1
NAME=$2
KEY=$3
HEIGHT=$4

gnuplot <<EOF
set terminal postscript enhanced eps monochrome
set output "img/sinesum-comparison${COMP}.eps"
set size 0.5,${HEIGHT}
set key ${KEY}
set rmargin 3

set title "Sintese Aditiva em Arduino (${NAME})"
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

set xrange [0:15]
set yrange [0:5]
#set yrange [0:5]
#set xrange [0:15]
#set log x 2

#plot "blocksize-32-31250${COMP}.txt"  using 1:2 title "sintese com bloco de 32 amostras"  with linespoints lt 1 pt 7 ps 0.7 pt 2 lc 0 lw 3, \
#     1.024 title "rt per. 32" with lines ls 2 lw 3 lc 0, \
#     "blocksize-64-31250${COMP}.txt"  using 1:2 title "sintese com bloco de 64 amostras"  with linespoints lt 1 pt 7 ps 0.7 pt 2 lc 1 lw 3, \
#     2.048 title "rt per. 64" with lines ls 2 lw 3 lc 1, \
#     "blocksize-128-31250${COMP}.txt" using 1:2 title "sintese com bloco de 128 amostras" with linespoints lt 1 pt 7 ps 0.7 pt 2 lc 3 lw 3, \
#     4.096 title "rt per. 128" with lines ls 2 lw 3 lc 3

#set arrow from 1.024,-0.5 to 1.024,15.5 lt 3 lw 3 lc 0 nohead
#set arrow from 2.048,-0.5 to 2.048,15.5 lt 3 lw 3 lc 1 nohead
#set arrow from 4.096,-0.5 to 4.096,15.5 lt 3 lw 3 lc 3 nohead

plot "blocksize-32-31250${COMP}.txt"  using 1:2 title "bloco de 32 amostras"  with linespoints lt 1 pt 7.5 ps 0.7 lc 0 lw 3, \
     1.024 notitle with lines lt 2 lw 3 lc 0, \
     "blocksize-64-31250${COMP}.txt"  using 1:2 title "bloco de 64 amostras"  with linespoints lt 1 pt 7.5 ps 0.7 lc 1 lw 3, \
     2.048 notitle with lines lt 2 lw 3 lc 1, \
     "blocksize-128-31250${COMP}.txt" using 1:2 title "bloco de 128 amostras" with linespoints lt 1 pt 7.5 ps 0.7 lc 3 lw 3, \
     4.096 notitle with lines lt 2 lw 3 lc 3
EOF
