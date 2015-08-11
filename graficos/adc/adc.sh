#!/bin/sh

FNAME=arduino-measured-adc

gnuplot <<EOF
set terminal postscript enhanced eps color
set output "${FNAME}.eps"
set size 1,1
#set key 

set title "Arduino ADC maximum conversion frequencies"
set xlabel "Prescaler value"
set ylabel "Frequency (KHz)"

set style line 12 lc rgb '#888888' lt 3 lw 3
set style line 13 lc rgb '#ddccdd' lt 1 lw 0.5
set grid ls 12
set xtics (2,4,8,16,32,64,128)

set ytics 100
set ytics add (44.1)


set pointsize 1.4

set yrange [0:600]
set xrange [2:128]
set log x 2

plot "values2.txt" using 1:(\$3/1000.) title "Advertised ADC frequency"  with linespoints lt 1 pt 7 lc 1 lw 6, \
     "values2.txt" using 1:(\$2/1000.) title "Measured ADC frequency"  with linespoints lt 1 pt 7 lc 3 lw 6
EOF

epspdf ${FNAME}.eps
