/* Real time DSP with Arduino
 *
 * 1.adc-freq.c: This file measures the maximum ADC frequency of the arduino,
 * by configuring the ADC mechanism to run in "free-running-mode".
 *
 * Andre J. Bianchi <ajb@ime.usp.br> - http://www.ime.usp.br/~ajb/arduino
 * Computer Music Group at IME/USP - http://compmus.ime.usp.br/ 2010-2013
 */

#include <Arduino.h>
#include <HardwareSerial.h>

// Used to set and clear register bits.
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

// interrupt variables accessed globally
unsigned long time;
unsigned long ltime;
unsigned long convtime;
unsigned long convs;

unsigned long icnt;
unsigned long icnt1;
unsigned long counter;

int ii;
byte dd[511];  // Audio Memory Array 8-Bit

void setup()
{
  Serial.begin(57600);  // connect to the serial port
  Serial.println("Arduino Audio Loopback");

  //--------------------------------------------------------------------------
  // INPUT - ADC
  //--------------------------------------------------------------------------

  // Table 24-5. ADC Prescaler selections
  //
  // ADPS2 ADPS1 ADPS0 Division Factor
  //   0     0     0           2
  //   0     0     1           2
  //   0     1     0           4
  //   0     1     1           8
  //   1     0     0          16
  //   1     0     1          32
  //   1     1     0          64
  //   1     1     1         128
  cbi(ADCSRA, ADPS2);
  sbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);

  // Enable ADC
  sbi(ADCSRA, ADEN);
  // Enable ADC interrupt
  sbi(ADCSRA, ADIE);
  // Set auto triggering
  sbi(ADCSRA, ADATE);

  // Select ADC auto trigger source
  //   Table 24-6. ADC Auto Trigger Source Selections
  //       ADTS2         ADTS1             ADTS0    Trigger Source
  //         0              0                 0     Free Running mode
  //         0              0                 1     Analog Comparator
  //         0              1                 0     External Interrupt Request 0
  //         0              1                 1     Timer/Counter0 Compare Match A
  //         1              0                 0     Timer/Counter0 Overflow
  //         1              0                 1     Timer/Counter1 Compare Match B
  //         1              1                 0     Timer/Counter1 Overflow
  //         1              1                 1     Timer/Counter1 Capture Event
  cbi(ADCSRB, ADTS2);
  cbi(ADCSRB, ADTS1);
  cbi(ADCSRB, ADTS0);

  // ADMUX - ADC Multiplexer Selection Register
  sbi(ADMUX,ADLAR);  // 8-Bit ADC in ADCH Register

  //   Table 24-3.  Voltage Reference Selections for ADC
  //   REFS1     REFS0   Voltage Reference Selection
  //     0         0     AREF, Internal Vref turned off
  //     0         1     AVCC with external capacitor at AREF pin
  //     1         0     Reserved
  //     1         1     Internal 1.1V Voltage Reference with external cap. at AREF pin
  cbi(ADMUX,REFS1);
  sbi(ADMUX,REFS0);  // VCC Reference

  // Table 24-4. Input Channel Selections
  //  MUX3...0                     Single Ended Input
  //  0000                         ADC0
  //  0001                         ADC1
  //  0010                         ADC2
  //  0011                         ADC3
  //  0100                         ADC4
  //  0101                         ADC5
  //  0110                         ADC6
  //  0111                         ADC7
  //  1000                         ADC8(1)
  //  1001                         (reserved)
  //  1010                         (reserved)
  //  1011                         (reserved)
  //  1100                         (reserved)
  //  1101                         (reserved)
  //  1110                         1.1V (VBG)
  //  1111                         0V (GND)
  // Set Input Multiplexer to Channel 0
  cbi(ADMUX,MUX0);
  cbi(ADMUX,MUX1);
  cbi(ADMUX,MUX2);
  cbi(ADMUX,MUX3);

  //--------------------------------------------------------------------------
  // OUTPUT - PWM
  //--------------------------------------------------------------------------

  // TCCR2A - Timer/Counter Control Register A
  //   Table 18-3. Compare Output Mode, Fast PWM Mode(1)
  //   COM2A1      COM2A0    Description
  //       0         0       Normal port operation, OC2A disconnected.
  //       0         1       WGM22 = 0: Normal Port Operation, OC0A Disconnected.
  //                         WGM22 = 1: Toggle OC2A on Compare Match.
  //       1         0       Clear OC2A on Compare Match, set OC2A at BOTTOM,
  //                         (non-inverting mode).
  //       1         1       Set OC2A on Compare Match, clear OC2A at BOTTOM,
  //                         (inverting mode).
  // Timer2 PWM Mode set to fast PWM 
  sbi (TCCR2A, COM2A1);
  cbi (TCCR2A, COM2A0);

  // Table 18-8.   Waveform Generation Mode Bit Description
  //                                     Timer/Counter
  //                                     Mode of                 Update of  TOV Flag
  //  Mode     WGM2    WGM1     WGM0     Operation           TOP  OCRx at  Set on(1)(2)
  //    0        0        0       0      Normal             0xFF Immediate    MAX
  //    1        0        0       1      PWM, Phase         0xFF    TOP     BOTTOM
  //                                     Correct
  //    2        0        1       0      CTC                OCRA Immediate    MAX
  //    3        0        1       1      Fast PWM           0xFF  BOTTOM      MAX
  //    4        1        0       0      Reserved           ----   ----      ----
  //    5        1        0       1      PWM, Phase         OCRA    TOP     BOTTOM
  //                                     Correct
  //    6        1        1       0      Reserved           ----   ----      ----
  //    7        1        1       1      Fast PWM           OCRA  BOTTOM      TOP
  cbi (TCCR2B, WGM22);
  sbi (TCCR2A, WGM21);
  sbi (TCCR2A, WGM20);

  // TCCR2B - Timer/Counter Control Register B
  // Table 18-9. Clock Select Bit Description
  //     CS22        CS21          CS20       Description
  //       0          0              0        No clock source (Timer/Counter stopped).
  //       0          0              1        clkT2S/(No prescaling)
  //       0          1              0        clkT2S/8 (From prescaler)
  //       0          1              1        clkT2S/32 (From prescaler)
  //       1          0              0        clkT2S/64 (From prescaler)
  //       1          0              1        clkT2S/128 (From prescaler)
  //       1          1              0        clkT2S/256 (From prescaler)
  //       1          1              1        clkT2S/1024 (From prescaler)
  // Timer2 Clock Prescaler to : 1 
  cbi (TCCR2B, CS22);
  cbi (TCCR2B, CS21);
  cbi (TCCR2B, CS20);

  // 1 is output, 0 is input
  // Timer2 PWM Port Enable
  sbi(DDRB,3);                    // set digital pin 11 to output

  //cli();                         // disable interrupts to avoid distortion
  // TIMSK0 - Timer/Counter Interrupt Mask Register
  // - Bit 0 - TOIE0: Timer/Counter0 Overflow Interrupt Enable
  //cbi (TIMSK0,TOIE0);              // disable Timer0 !!! delay is off now
  // TIMSK1 - Timer/Counter1 Interrupt Mask Register
  // - Bit 0 - TOIE1: Timer/Counter1, Overflow Interrupt Enable
  sbi (TIMSK2,TOIE2);              // enable Timer2 Interrupt


  //--------------------------------------------------------------------------
  // Timer 1
  //--------------------------------------------------------------------------
  
  // Timer1 - Clock Select: No prescaling
  //   Table 16-5.  Clock Select Bit Description
  //   CS12      CS11    CS10      Description
  //     0        0        0       No clock source (Timer/Counter stopped).
  //     0        0        1       clkI/O/1 (No prescaling)
  //     0        1        0       clkI/O/8 (From prescaler)
  //     0        1        1       clkI/O/64 (From prescaler)
  //     1        0        0       clkI/O/256 (From prescaler)
  //     1        0        1       clkI/O/1024 (From prescaler)
  //     1        1        0       External clock source on T1 pin. Clock on falling edge.
  //     1        1        1       External clock source on T1 pin. Clock on rising edge.
  cbi(TCCR1B, CS12);
  cbi(TCCR1B, CS11);
  cbi(TCCR1B, CS10);

  // Timer1 - Overflow Interrupt: Enable
  sbi (TIMSK1,TOIE1);

  // Timer1 - Waveform Generation Mode: normal
  cbi (TCCR1B, WGM13);
  cbi (TCCR1B, WGM12);
  cbi (TCCR1A, WGM11);
  cbi (TCCR1A, WGM10);

  // Timer1 - Compare Output Mode: normal port operation
  cbi(TCCR1A, COM1A1);
  cbi(TCCR1A, COM1A0);
  cbi(TCCR1A, COM1B1);
  cbi(TCCR1A, COM1B0);


  //--------------------------------------------------------------------------
  // Variables initialization
  //--------------------------------------------------------------------------

  ltime = micros();

  // interrupt count on timer 2
  icnt = 0;
  icnt1 = 0;

  // conversion timing
  convtime = 0;
  convs = 0;

  // Start first conversion
  counter = 0;
  sbi(ADCSRA,ADSC);               // start next conversion
}



// counter to count number of ADC interrupts
unsigned long adccnt = 0;

ISR(ADC_vect)
{
  counter++;
}


void loop()
{
  // run loop every 1 second
  delay(1000);

  Serial.print("elapsed ms: ");
  Serial.print(micros());
  Serial.print("  mean adc freq: ");
  Serial.print((float)counter/micros()*1000000);
  Serial.print("  icnt1: ");
  Serial.print(icnt1);
  Serial.print("  counter: ");
  Serial.println(counter);
} // loop
