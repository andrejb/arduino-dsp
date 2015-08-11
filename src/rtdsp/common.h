#ifndef LOCAL_COMMON_H
#define LOCAL_COMMON_H

#include <interrupt.h>
#include <common.h>
#include <Arduino.h>

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))


int ledPin = 13;                 // LED connected to digital pin 13
int testPin = 7;

//typedef int boolean;
//typedef uint8_t byte;

boolean div32;
boolean div16;

// interrupt variables accessed globally
volatile boolean f_sample;
volatile byte badc0;
volatile byte badc1;
volatile byte ibb;
int cnta;



int ii;
byte dd[511];  // Audio Memory Array 8-Bit


void setup()
{
  pinMode(ledPin, OUTPUT);      // sets the digital pin as output
  pinMode(testPin, OUTPUT);
  Serial.begin(57600);        // connect to the serial port
  Serial.println("Arduino Audio Loopback");

  // set adc prescaler  to 64 for 19kHz sampling frequency
  cbi(ADCSRA, ADPS2);
  sbi(ADCSRA, ADPS1);
  sbi(ADCSRA, ADPS0);




  sbi(ADMUX,ADLAR);  // 8-Bit ADC in ADCH Register
  sbi(ADMUX,REFS0);  // VCC Reference
  cbi(ADMUX,REFS1);
  cbi(ADMUX,MUX0);   // Set Input Multiplexer to Channel 0
  cbi(ADMUX,MUX1);
  cbi(ADMUX,MUX2);
  cbi(ADMUX,MUX3);


  // Timer2 PWM Mode set to fast PWM 
  cbi (TCCR2A, COM2A0);
  sbi (TCCR2A, COM2A1);
  sbi (TCCR2A, WGM20);
  sbi (TCCR2A, WGM21);

  cbi (TCCR2B, WGM22);




  // Timer2 Clock Prescaler to : 1 
  sbi (TCCR2B, CS20);
  cbi (TCCR2B, CS21);
  cbi (TCCR2B, CS22);

  // Timer2 PWM Port Enable
  sbi(DDRB,3);                    // set digital pin 11 to output

  //cli();                         // disable interrupts to avoid distortion
  cbi (TIMSK0,TOIE0);              // disable Timer0 !!! delay is off now
  sbi (TIMSK2,TOIE2);              // enable Timer2 Interrupt


}


//******************************************************************
// Timer2 Interrupt Service at 62.5 KHz
// here the audio and pot signal is sampled in a rate of:  16Mhz / 256 / 2 / 2 = 15625 Hz
// runtime : xxxx microseconds
ISR(TIMER2_OVF_vect) {

  PORTB = PORTB  | 1 ;

  div32=!div32;                      // divide timer2 frequency / 2 to 31.25kHz
  if (div32){ 
    div16=!div16;  // 
    if (div16) {                     // sample channel 0 and 1 alternately so each channel is sampled with 15.6kHz
      badc0=ADCH;                    // get ADC channel 0
      sbi(ADMUX,MUX0);               // set multiplexer to channel 1
    }
    else
    {
      badc1=ADCH;                    // get ADC channel 1
      cbi(ADMUX,MUX0);               // set multiplexer to channel 0
      f_sample=true;
    }
    ibb++;                          // short delay before start conversion
    ibb--; 
    ibb++; 
    ibb--;    
    sbi(ADCSRA,ADSC);               // start next conversion
  }

}


#endif
