#!/bin/sh

FILE="128.txt"

gnuplot <<EOF
set terminal pdf
set output "basic-integer-ops.pdf"
#set size 0.5,0.55
set key below
set rmargin 3

set title "Medição do tempo gasto por diferentes operações sobre inteiros"
set xlabel "Valor do 2o. argumento da operação"
set ylabel "Tempo de cálculo (ms)"

set ytics 1

set pointsize 0.3

plot "128.txt" using 1:2 title "divisão por inteiro" with linespoints lt 1 pt 7 lc 7 lw 5, \
  "128.txt" using 1:3 title "multiplicação por inteiro" with linespoints lt 1 pt 7 lc 1 lw 5, \
  "128.txt" using 1:4 title "divisão por potência de 2 usando bit-shifting" with linespoints lt 1 pt 7 lc 8 lw 5, \
  "128.txt" using 1:5 title "multiplicação por potência de 2 usando bit-shifting" with linespoints lt 0 pt 7 lc 3 lw 5
EOF
