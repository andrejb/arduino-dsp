#!/bin/sh

BASENAME="${1}"
IMG="img/${BASENAME}.eps"
TITLE="${2}"
DSPCYCLE="${3}"

gnuplot <<EOF
set terminal postscript enhanced eps monochrome
set output "${IMG}"
set size 0.5,0.5
set key left top
set rmargin 3

set title "Additive Synthesis on Arduino (${TITLE})"
set xlabel "Number of oscilators"
set ylabel "Synth time (us)"

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

set yrange [0:3]
set xrange [0:11]
#set log x 2

plot "${BASENAME}.txt" using 1:2 title "sines sum" with linespoints ls 1 pt 2, \
     ${DSPCYCLE} title "dsp cycle period" with lines ls 0 lw 4
EOF
