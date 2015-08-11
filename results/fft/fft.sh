#!/bin/sh


gnuplot <<EOF
set terminal postscript enhanced eps color
set output "img/fft.eps"
set size 0.5,0.6
set key under
set rmargin 3

set title "FFT no Arduino (R=1953 Hz)"
set xlabel "Tamanho do bloco de amostras"
set ylabel "Tempo de analise (ms)"

set xtics ( \
  "..." 32,       \
  64,       \
  128,      \
  256,      \
  512 )
set grid
#set xtics 1
#set ytics 1

set yrange [0:320]
set xrange [0:550]
#set log x 2

set pointsize 0.7

plot "results.txt"  using 1:2 title "fft usando sin()"  with linespoints lw 3 lt 1 pt 7 lc 1, \
     "results.txt"  using 1:4 title "fft com consulta a tabela"  with linespoints lw 3 lt 1 pt 7 lc 3, \
     "results.txt"  using 1:3 title "periodo teorico do ciclo DSP"  with lines lt 2 lc 0 lw 3
EOF

epspdf img/fft.eps img/fft.pdf
