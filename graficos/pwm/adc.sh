#!/bin/sh

OP=$1
TITLE=$2
XLIMIT=$3
KEY=$4
HEIGHT=$5

gnuplot <<EOF
set terminal pdf color
set output "arduino-measured-adc.pdf"
set size 1,1
#set key 

set title "Advertised and measured ADC conversion frequencies"
set xlabel "Prescaler value"
set ylabel "Sampling frequency (Hz)"

set grid
set xtics (2,4,8,16,32,64,128)


set pointsize 0.5

set yrange [0:600]
set xrange [0:128]
#set log x 2

plot "values.txt" using 1:(\$2/1000.) title "advertised frequency"  with linespoints lt 1 pt 7 lc 1 lw 6, \
     "values.txt" using 1:(\$5/1000.) title "measured frequency"  with linespoints lt 1 pt 7 lc 3 lw 6
EOF
