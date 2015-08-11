/* Arduino Audio Loopback Tes
 *
 * Arduino Realtime Audio Processing
 * 2 ADC 8-Bit Mode
 * analog input 1 is used to sample the audio signal
 * analog input 0 is used to control an audio effect
 * PWM DAC with Timer2 as analog output
 
 
 
 * KHM 2008 / Lab3/  Martin Nawrath nawrath@khm.de
 * Kunsthochschule fuer Medien Koeln
 * Academy of Media Arts Cologne
 
 */

#include <Arduino.h>
#include <HardwareSerial.h>

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

#define BLOCK_SIZE 64
#define BUFFER_SIZE (2*BLOCK_SIZE)
#define MIN_DELAY (2*BLOCK_SIZE)

int ledPin = 13;                 // LED connected to digital pin 13
int testPin = 7;


boolean div16;
// interrupt variables accessed globally
volatile boolean dsp_block;
volatile byte badc0;
volatile byte badc1;
volatile byte ibb;
int cnta;

unsigned long time;
unsigned long ltime;
unsigned long exptime;
unsigned long convtime;
unsigned long convs;

unsigned long icnt;
unsigned long icnt1;

float pi = 3.141592;
//float dx = 2 * pi / 512;

uint8_t x[BUFFER_SIZE];  // Audio Memory Array 8-Bit
uint8_t y[BUFFER_SIZE];
int wind = 0;
int rind = 0;


#define NUM_FREQS 2
float freqs[NUM_FREQS];

#define SINETABLE_SIZE 512
byte sine[SINETABLE_SIZE];

void fill_sinewave() {
  float arg = 0;
  float dx = 2 * pi / SINETABLE_SIZE;      // fill the 1024 byte bufferarry
  for (int i = 0; i < SINETABLE_SIZE; i++) { // with  1 period of a sinewawe
    float fd= 127 * sin(arg);           // fundamental tone
    arg += dx;                // in the range of 0 to 2xpi  and 1/512 increments
    sine[i] = 127 + fd;              // add dc offset to sinewawe 
  }
}

void setup()
{

  pinMode(ledPin, OUTPUT);      // sets the digital pin as output
  pinMode(testPin, OUTPUT);
  Serial.begin(57600);        // connect to the serial port
  Serial.println("Arduino Audio Block DSP");

  fill_sinewave();

  for (int i = 0; i < NUM_FREQS; i++)
    freqs[i] = (i+1)*110;

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
  // set adc prescaler to 64 for 19kHz sampling frequency
  cbi(ADCSRA, ADPS2);
  sbi(ADCSRA, ADPS1);
  sbi(ADCSRA, ADPS0);

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
  sbi(ADMUX,MUX0);
  cbi(ADMUX,MUX1);
  cbi(ADMUX,MUX2);
  cbi(ADMUX,MUX3);

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
  

  //--------------------------------------------------------------------------
  // Perform function
  //--------------------------------------------------------------------------
  //  15.9.1 TCCR0A - Timer/Counter Control Register A
  //  Bits 1:0 - WGM01:0: Waveform Generation Mode
  //  Table 15-8.    Waveform Generation Mode Bit Description
  //                                          Timer/Counter
  //                                          Mode of                   Update of      TOV Flag
  //  Mode     WGM02      WGM01     WGM00     Operation         TOP      OCRx at      Set on(1)(2)
  //    0         0         0          0      Normal            0xFF    Immediate        MAX
  //                                          PWM, Phase
  //    1         0         0          1                        0xFF       TOP         BOTTOM
  //                                          Correct
  //    2         0         1          0      CTC              OCRA     Immediate        MAX
  //    3         0         1          1      Fast PWM          0xFF     BOTTOM          MAX
  //    4         1         0          0      Reserved            -         -              -
  //                                          PWM, Phase
  //    5         1         0          1                       OCRA        TOP         BOTTOM
  //                                          Correct
  //    6         1         1          0      Reserved            -         -              -
  //    7         1         1          1      Fast PWM         OCRA      BOTTOM          TOP
  //cbi(TCCR0A, WGM02);
  //cbi(TCCR0A, WGM01);
  //cbi(TCCR0A, WGM00);

  //  Table 15-2.     Compare Output Mode, non-PWM Mode
  //      COM0A1         COM0A0     Description
  //            0            0      Normal port operation, OC0A disconnected.
  //            0            1      Toggle OC0A on Compare Match
  //            1            0      Clear OC0A on Compare Match
  //            1            1      Set OC0A on Compare Match
  //cbi(TCCR0A, COM0A1);
  //cbi(TCCR0A, COM0A0);

  //cbi(TCCR0A, COM0B1);
  //cbi(TCCR0A, COM0B0);

  //  15.9.2 TCCR0B - Timer/Counter Control Register B
  //  Bits 2:0 - CS02:0: Clock Select
  //  Table 15-9.   Clock Select Bit Description
  //  CS02    CS01    CS00     Description
  //   0        0       0      No clock source (Timer/Counter stopped)
  //   0        0       1      clkI/O/(No prescaling)
  //   0        1       0      clkI/O/8 (From prescaler)
  //   0        1       1      clkI/O/64 (From prescaler)
  //   1        0       0      clkI/O/256 (From prescaler)
  //   1        0       1      clkI/O/1024 (From prescaler)
  //   1        1       0      External clock source on T0 pin. Clock on falling edge.
  //   1        1       1      External clock source on T0 pin. Clock on rising edge.
  //cbi(TCCR0B, CS02);
  //cbi(TCCR0B, CS01);
  //sbi(TCCR0B, CS00);

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
  cbi (TCCR2B, CS22);
  cbi (TCCR2B, CS21);
  sbi (TCCR2B, CS20);

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
  sbi(TCCR1B, CS10);

  // Timer1 - Overflow Interrupt: Enable
  //sbi (TIMSK1,TOIE1);

  // Timer1 - Waveform Generation Mode: normal
  //cbi (TCCR1B, WGM13);
  //cbi (TCCR1B, WGM12);
  //cbi (TCCR1A, WGM11);
  //cbi (TCCR1A, WGM10);

  // Timer1 - Compare Output Mode: normal port operation
  //cbi(TCCR1A, COM1A1);
  //cbi(TCCR1A, COM1A0);
  //cbi(TCCR1A, COM1B1);
  //cbi(TCCR1A, COM1B0);


  //--------------------------------------------------------------------------
  // Variables initialization
  //--------------------------------------------------------------------------

  ltime = micros();
  exptime = ltime;

  // interrupt count on timer 2
  icnt = 0;
  icnt1 = 0;

  // conversion timing
  convtime = 0;
  convs = 0;
  wind = 0;

  dsp_block = false;


}

#define LIMIT(x) \
  if (x < -127)             \
    x = -127;               \
  if (x > 127)             \
    x = 127;

#define TMOD(x, index, limit) x[(index)&(limit-1)]


float dx = 2 * pi / BLOCK_SIZE;

/*****************************************************************************
 *  _     ___   ___  ____  
 * | |   / _ \ / _ \|  _ \
 * | |  | | | | | | | |_) |
 * | |__| |_| | |_| |  __/ 
 * |_____\___/ \___/|_|    
 ****************************************************************************/
void loop()
{
  //   while (!f_sample) {         // wait for Sample Value from ADC
  //   }                           // Cycle 15625 KHz = 64uSec 
     PORTD = PORTD  | 128;       //  Test Output on pin 7
  //   f_sample=false;
  // 
  //   OCR2A=badc1;                // output audio to PWM port (pin 11)

  // variable delay controlled by potentiometer    
  // when distortion then delay / processing time is too long   
  //for (cnta=0; cnta <= badc0; cnta++) { 
  //  ibb = ibb * 5;              
  //}
  

  //while (!f_sample);        // wait for Sample Value from ADC
  //OCR2A=badc1;              // output audio to PWM port (pin 11)

  /* aguarda um novo bloco de amostras */
  while (!dsp_block);

  /* processa o bloco */

  static float cum[10];

  int maxind = rind+BLOCK_SIZE;

  static int order = 1;
  static int pad = 0;

  long a[][3] = {
    {0, 0, 0},
    {1414214, 0, 0},
    {2, 2, 0},
    {2613126, 3414214, 2613126}
  };

  static unsigned long cumi = 0;
  /* timekeeping variables */
  unsigned long start_time = micros();
  static unsigned long elapsed_time;
  for (int n = rind; n < maxind; n++) {
    //TMOD(y, n, BUFFER_SIZE) = TMOD(x, n, BUFFER_SIZE);
    //for (int i = 0; i < NUM_FREQS; i++) {
    //  cum[i] = (cum[i] + 
    //           (unsigned long) (freque[i] * SINETABLE_SIZE / 31250))
    //           & (SINETABLE_SIZE-1);
    //  TMOD(y, n, BUFFER_SIZE) += sine[cum[i]]>>1;
    //}

    int yn, xtmp;
    yn = 127 - TMOD(x, n, BUFFER_SIZE);
    int xn1 = 127 - TMOD(x, n-1, BUFFER_SIZE);
    int xn2 = 127 - TMOD(x, n-2, BUFFER_SIZE);
    int xn3 = 127 - TMOD(x, n-3, BUFFER_SIZE);
    int xn4 = 127 - TMOD(x, n-4, BUFFER_SIZE);
    int xn5 = 127 - TMOD(x, n-5, BUFFER_SIZE);
    int xn6 = 127 - TMOD(x, n-6, BUFFER_SIZE);
    int xn7 = 127 - TMOD(x, n-7, BUFFER_SIZE);
    int yn1 = 127 - TMOD(y, n-1, BUFFER_SIZE);
    int yn2 = 127 - TMOD(y, n-2, BUFFER_SIZE);
    if (order == 4) {
      yn >>= 2;
      yn += xn1 >> 2;
      yn += xn2 >> 2;
      yn += xn3 >> 2;
    } else if (order == 8) {
      yn >>= 3;
      yn += xn1 >> 3;
      yn += xn2 >> 3;
      yn += xn3 >> 3;
      yn += xn4 >> 3;
      yn += xn5 >> 3;
      yn += xn6 >> 3;
      yn += xn7 >> 3;
    }
    //for (int i = 1; i < order; i++) {
    //  xtmp = (127 - TMOD(y, n-i, BUFFER_SIZE));
    //  yn += (int) xtmp * a[order-2][i] / 1000000;
    //}
    LIMIT(yn);
    TMOD(y, n, BUFFER_SIZE) = 127 + yn;
  }
  elapsed_time += micros() - start_time;
  convs++;

    //y[(n-BLOCK_SIZE)&(BUFFER_SIZE-1)] = x[n&(BUFFER_SIZE-1)];
  //Serial.println("received block");
  dsp_block = false;

   PORTD = PORTD  | 128;       //  Test Output on pin 7


  time = micros();
  if ((time - ltime) >= 1000000) {
    //Serial.print("ltime: ");
    //Serial.print(ltime);
    Serial.print(" micros: ");
    Serial.print(micros());
  //  Serial.print("  convtime: ");
  //  Serial.print(convtime);
    Serial.print("  elapsed time: ");
    Serial.print(elapsed_time);
    Serial.print("  order: ");
    Serial.print(order);
//     Serial.print(" valora[0][0] ");
//     Serial.print(a[0][0]);
//     Serial.print(" valora[1][0] ");
//     Serial.print(a[1][0]);
//     Serial.print(" valora[1][1] ");
//     Serial.print(a[0][1]);
//     Serial.print(" valora[2][0] ");
//     Serial.print(a[2][0]);
//     Serial.print(" valora[2][1] ");
//     Serial.print(a[2][1]);
//     Serial.print(" valora[2][2] ");
//     Serial.print(a[3][2]);
    Serial.print("  mean time: ");
    Serial.println(((double)elapsed_time)/convs);
  //  Serial.print("  val: ");
  //  Serial.println(badc0);
  //  //Serial.print("  counter: ");
  //  //Serial.println(counter);
    ltime = time;

    if ((time - exptime) >= 5000000) {
      if (order == 1)
        order = 4;
      else if (order == 4)
        order = 8;
      else
        order = 1;
      exptime = time;
      elapsed_time = 0;
      convs = 0;
      pad += 1;
    }
  }

} // loop

//******************************************************************
// Timer2 Interrupt Service at 62.5 KHz
// here the audio and pot signal is sampled in a rate of:  16Mhz / 256 / 2 / 2 = 15625 Hz
// runtime : xxxx microseconds
ISR(TIMER2_OVF_vect) {

  //PORTB = PORTB  | 1 ;
  static boolean div = false;

  //static boolean div2 = false;
  //static boolean div4 = false;
  div = !div;                      // divide timer2 frequency / 2 to 31.25kH
  if (div){ 
    //div2 = !div2;
    //if (div2) {
      //div4 = !div4;
      //if (div4) {
    //sbi(ADCSRA,ADSC);               // start next conversion
    //while ((_SFR_BYTE(ADCSRA) & _BV(ADSC)) == _BV(ADSC)); // wait for sample.
    //dd[ddwrite++] = ADCH;                    // get ADC channel 0
    //ddwrite &= 511;
    //OCR2A = dd[ddread++];               // Sample Value to PWM Output

    //badc1=ADCH;         // get ADC channel 1

    /* leitura da entrada */
    x[wind] = ADCH;

    /* escrita para a saida */
    //OCR2A = y[(wind-MIN_DELAY)&(BUFFER_SIZE-1)];
    OCR2A = y[(wind-MIN_DELAY)&(BUFFER_SIZE-1)];

    /* verificacao de novo bloco */
    if ((wind & (BLOCK_SIZE - 1)) == 0) {
      rind = (wind-BLOCK_SIZE) & (BUFFER_SIZE-1);
      dsp_block = true;
    }

    /* incremento do indice */
    wind++;
    wind &= BUFFER_SIZE - 1;

    //ibb++;
    //ibb--;
    //ibb++;
    //ibb--;
    sbi(ADCSRA,ADSC);               // start next conversion
  }
}

//ISR(TIMER0_OVF_vect) {
//
//}
