/*
 * Real Time DSP benchmarking on Arduino
 *
 * Author: ajb@ime.usp.br
 *
 * This program performs a series of routines for stressing the
 * arduino board with common DSP tasks and then generates reports
 * with results of measures of computational intensity.
 */

#include "common.h"

void loop()
{
  while (!f_sample) {         // wait for Sample Value from ADC
  }                           // Cycle 15625 KHz = 64uSec 
  PORTD = PORTD  | 128;       //  Test Output on pin 7
  f_sample=0;

  OCR2A=badc1;                // output audio to PWM port (pin 11)

  // variable delay controlled by potentiometer    
  // when distortion then delay / processing time is too long   
  for (cnta=0; cnta <= badc0; cnta++) { 
    ibb = ibb * 5;              
  }

  PORTD = PORTD  ^ 128;       //  Test Output on pin 7
} // loop

