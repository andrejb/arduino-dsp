/*
 * operacao: convolucao
 * tipo: ponto flutuante
 * prescaler adc: 8
 */

#include <Arduino.h>
#include <HardwareSerial.h>

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))


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
unsigned long count = 0;

unsigned long icnt;
unsigned long icnt1;

float pi = 3.141592;

int16_t writeind = 0;
int16_t readind = 0;


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

  dsp_block = false;


}

/**
 * limit(x)
 * Limita `x` ao intervalo [-127,127].
 */
#define limit(x) \
  if (x < -127)  \
    x = -127;    \
  if (x > 127)   \
    x = 127;

/**
 * tabmod(x, index, len)
 * Retorna x[index mod len].
 */
#define tabmod(x, index) x[(index)&(BUFFER_SIZE_1)]

/**
 * 
 */

#define BLOCK_SIZE 128
#define BLOCK_SIZE_1 127
#define BUFFER_SIZE 256
#define BUFFER_SIZE_1 255
int16_t x[BUFFER_SIZE];  // Audio Memory Array 8-Bit
int16_t y[BUFFER_SIZE];

//#define MEASUREFIRST
#define LOOP_NOVECTOR

uint8_t ord = 0;
/*****************************************************************************
 * Funcao loop() da convolucao no dominio do tempo.
 * 
 * prescaler adc: 8
 * Frequencia de amostragem: 31250 Hz.
 ****************************************************************************/
void loop()
{
    /*
     * Coeficientes do filtro.
     */
    float a[] = {0.23, 0.43, 0.63, 0.73, 0.83 };
    int8_t b[] = {2, 3, 4, 2, 3, 4, 2, 3, 4, 2, 3, 4, 2, 3, 4};
    int8_t c[] = {10, 10, 10, 10, 10, 10};
    uint8_t d[] = {2,3,4,2,3,4,2,3,4,2,3,4};

    static int8_t div = 1;
    int8_t divi = div;

    /* Aguarda um novo bloco de amostras. */
    while (!dsp_block);

    /*
     * Variaves para contagem do tempo.
     */
    uint8_t order = 1;   // <<-------------------------------- ORDER
    static unsigned long elapsed_time = 0;
    unsigned long start_time = micros();

    #ifdef MEASUREFIRST
        static unsigned long begin = 0;
        static unsigned long end = 0;
        if (begin == 0)
            begin = micros();
    #endif

    /*
     * Processa o bloco.
     */
    uint16_t maxind = readind+BLOCK_SIZE;
    for (uint16_t n = readind; n < maxind; n++) {

        int16_t yn = 0;
        /* laco da convolucao */
        #ifdef LOOP
        for (uint8_t i = 0; i < order; i++) {
            uint8_t ind = n-i;
            // mem lookup
            yn += tabmod(x, n) * a[ind];  // float
            //yn += tabmod(x, n) * b[ind] / c[ind];  // 2 integer ops
            //yn += tabmod(x, n) / c[ind];  // 1 integer op
            //yn += tabmod(x, n) >> d[ind];  // vpad
            // direct calc
            //yn += tabmod(x, n) * a[ind];  // float
            //yn += tabmod(x, n) * b[ind] / c[ind];  // 2 integer ops
            //yn += tabmod(x, n) / c[ind];  // 1 integer op
            //yn += tabmod(x, n) >> d[ind];  // vpad
        }
        #endif

        /* laco da convolucao */
        #ifdef LOOP_NOVECTOR
        for (uint8_t i = 0; i < order; i++) {
            uint8_t ind = n-i;
            // mem lookup
            //yn += tabmod(x, n) * 0.453;  // float
            //yn += tabmod(x, n) *3  / 5 ;  // 2 integer ops
            //yn += tabmod(x, n) / 33;  // 1 integer op
            //yn += tabmod(x, n) * 2;  // 1 integer op
            yn += tabmod(x, n) << 5;  // vpad
            // direct calc
            //yn += tabmod(x, n) * a[ind];  // float
            //yn += tabmod(x, n) * b[ind] / c[ind];  // 2 integer ops
            //yn += tabmod(x, n) / * c[ind];  // 1 integer op
            //yn += tabmod(x, n) >> d[ind];  // vpad
        }
        #endif

        /* convolucao inline */
        #ifdef INLINE
            uint8_t ind = n;

            //yn += tabmod(x, n) * a[n-0];

            //yn += tabmod(x, n) * b[ind] / c[ind];  // 2 integer ops
            //ind--;
            //yn += tabmod(x, ind) * b[ind] / c[ind];  // 2 integer ops

            //yn += tabmod(x, ind) >> d[ind];  // vpad
            //ind--;
            //yn += tabmod(x, ind) >> d[ind];  // vpad

            //yn += tabmod(x, ind) / c[ind];  // 1 integer op
            //ind--;
            //yn += tabmod(x, ind) / c[ind];  // 1 integer op
        #endif

        /*
         * Limita o resultado ao intervalo [-127,127] e escreve no buffer de
         * saida.
         */
        limit(yn);  // limita 
        tabmod(y, n) = yn;  // escreve no buffer de saida.
    }
    elapsed_time += micros() - start_time;
    count++;

    #ifdef MEASUREFIRST
        if (end == 0)
          end = micros();
    #endif

    dsp_block = false;

  //PORTD = PORTD  | 128;       //  Test Output on pin 7

  if (count == 1000) {
    //unsigned long now = micros();
    Serial.println("");
    Serial.println("=============================");
    Serial.print(" order: ");
    Serial.print(order);
    #ifdef MEASUREFIRST
        Serial.print(" begin: ");
        Serial.print(begin);
        Serial.print(" end: ");
        Serial.print(end);
    #endif
    //Serial.print(" elapsed total: ");
    //Serial.print(now - begin);
    Serial.print(" elapsed_time mean: ");
    Serial.println((unsigned long) (elapsed_time/(unsigned long)count));
    Serial.print(" div: ");
    Serial.println(div);
    Serial.println("=============================");
    elapsed_time = 0;
    count = 0;
    div = div+1;
    //ord += 1;
  }

  time = micros();
  if ((time - ltime) >= 1000000) {
    Serial.print(".");
    //Serial.print(" micros: ");
    //Serial.print(micros());
    //Serial.print(" start_time: ");
    //Serial.println(start_time);
    //Serial.print(" elapsed_time: ");
    //Serial.print(elapsed_time);
    //Serial.print(" count: ");
    //Serial.print(count);
    //Serial.print(" elapsed_time mean: ");
    //Serial.println((unsigned long) (elapsed_time/(unsigned long)count));
    ltime = time;
  }

} // loop

/*****************************************************************************
 * Funcao de interrupcao por overflow do Timer 2.
 *
 * Os vetores `x` e `y` correspondem respectivamente aos buffers de amostras
 * de entrada e saida, e sao vetores de inteiros de 8 bits positivos.
 *
 * As variaves `writeind` e `readind` sao inteiros de 8 bits, portanto seu
 * valor maximo e' 255. Essa propriedade e' utilizada para fazer a leitura e
 * escrita circular dos vetores de entrada e saida, `x` e `y`, cujo tamanho e'
 * extamente 256.
 ****************************************************************************/
ISR(TIMER2_OVF_vect)
{
    /*
     * Divisao da frequencia de amostragem por 2.
     */
    static boolean div = false;
    div = !div;  
    if (div){ 

        /*
         * Leitura da entrada com conversao de tipo. ADCH e' o registrador que
         * contem o resultado da ultima conversao ADC.
         */
        x[writeind] = (int16_t) 127 - ADCH;

        /*
         * Escrita da amostra de saida no registrador PWM, com ajuste de
         * offset e conversao de tipo.
         */
        OCR2A = (uint8_t) 127 + y[(writeind-BLOCK_SIZE)];

        /*
         * Verificacao de um novo bloco de amostras esta cheio, calculando se
         * (writeind mod BLOCK_SIZE) == 0.
         */
        if ((writeind & (BLOCK_SIZE_1)) == 0) {  
          readind = (writeind - BLOCK_SIZE) & BUFFER_SIZE_1;
          dsp_block = true;
        }

        /*
         * Incremento do indice de escrita.
         */
        writeind++;
        writeind &= BUFFER_SIZE_1;

        /*
         * Inicia a proxima conversao.
         */
        sbi(ADCSRA,ADSC);
    }
}

//ISR(TIMER0_OVF_vect) {
//
//}
