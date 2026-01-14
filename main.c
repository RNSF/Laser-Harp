#include "driverlib.h"
#include "intrinsics.h"
#include <msp430.h>




#define NOTE_ON 0xffff
#define NOTE_OFF 0x0000
#define SAMPLE_RATE 512






#define C7_DELTA 5689
#define D7_DELTA 6386
#define E7_DELTA 7168
#define F7_DELTA 7594
#define Fs7_DELTA 8046
#define G7_DELTA 8524
#define A7_DELTA 9568
#define B7_DELTA 10740




unsigned int counts[8];
unsigned int isPlaying[8];
unsigned int scaleIndex = 0;
unsigned int waveIndex = 0;


#define SCALE_COUNT 4
unsigned int SCALES[SCALE_COUNT][8] = {
    { // c4 major
        C7_DELTA >> 3,
        D7_DELTA >> 3,
        E7_DELTA >> 3,
        F7_DELTA >> 3,
        G7_DELTA >> 3,
        A7_DELTA >> 3,
        B7_DELTA >> 3,
        C7_DELTA >> 2,
    },
    { // a4 minor
        A7_DELTA >> 3,
        B7_DELTA >> 3,
        C7_DELTA >> 2,
        D7_DELTA >> 2,
        E7_DELTA >> 2,
        F7_DELTA >> 2,
        G7_DELTA >> 2,
        A7_DELTA >> 2,
    },
    { // a4 dorian
        A7_DELTA >> 3,
        B7_DELTA >> 3,
        C7_DELTA >> 2,
        D7_DELTA >> 2,
        E7_DELTA >> 2,
        Fs7_DELTA >> 2,
        G7_DELTA >> 2,
        A7_DELTA >> 2,
    },
    { // g4 mixolydian
        G7_DELTA >> 3,
        A7_DELTA >> 3,
        B7_DELTA >> 3,
        C7_DELTA >> 2,
        D7_DELTA >> 2,
        E7_DELTA >> 2,
        F7_DELTA >> 2,
        G7_DELTA >> 2,
    }
};




#define WAVE_COUNT 3
int WAVES[WAVE_COUNT][256] = {
    {   // sine wave
           0,    3,    6,    9,   12,   15,   18,   21,   24,   27,   30,   33,   36,   39,   42,   45,
          48,   51,   54,   57,   59,   62,   65,   67,   70,   73,   75,   78,   80,   82,   85,   87,
          89,   91,   94,   96,   98,  100,  102,  103,  105,  107,  108,  110,  112,  113,  114,  116,
         117,  118,  119,  120,  121,  122,  123,  123,  124,  125,  125,  126,  126,  126,  126,  126,
         127,  126,  126,  126,  126,  126,  125,  125,  124,  123,  123,  122,  121,  120,  119,  118,
         117,  116,  114,  113,  112,  110,  108,  107,  105,  103,  102,  100,   98,   96,   94,   91,
          89,   87,   85,   82,   80,   78,   75,   73,   70,   67,   65,   62,   59,   57,   54,   51,
          48,   45,   42,   39,   36,   33,   30,   27,   24,   21,   18,   15,   12,    9,    6,    3,
           0,   -3,   -6,   -9,  -12,  -15,  -18,  -21,  -24,  -27,  -30,  -33,  -36,  -39,  -42,  -45,
         -48,  -51,  -54,  -57,  -59,  -62,  -65,  -67,  -70,  -73,  -75,  -78,  -80,  -82,  -85,  -87,
         -89,  -91,  -94,  -96,  -98, -100, -102, -103, -105, -107, -108, -110, -112, -113, -114, -116,
        -117, -118, -119, -120, -121, -122, -123, -123, -124, -125, -125, -126, -126, -126, -126, -126,
        -127, -126, -126, -126, -126, -126, -125, -125, -124, -123, -123, -122, -121, -120, -119, -118,
        -117, -116, -114, -113, -112, -110, -108, -107, -105, -103, -102, -100,  -98,  -96,  -94,  -91,
         -89,  -87,  -85,  -82,  -80,  -78,  -75,  -73,  -70,  -67,  -65,  -62,  -59,  -57,  -54,  -51,
         -48,  -45,  -42,  -39,  -36,  -33,  -30,  -27,  -24,  -21,  -18,  -15,  -12,   -9,   -6,   -3
    },
    {   // square wave
        -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128,
        -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128,
        -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128,
        -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128,
        -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128,
        -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128,
        -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128,
        -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128,
        -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128,
         127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,
         127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,
         127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,
         127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,
         127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,
         127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,
         127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127
    },
    {   // saw wave
        -128, -127, -126, -125, -124, -123, -122, -121, -120, -119, -118, -117, -116, -115, -114, -113,
        -112, -111, -110, -109, -108, -107, -106, -105, -104, -103, -102, -101, -100,  -99,  -98,  -97,
         -96,  -95,  -94,  -93,  -92,  -91,  -90,  -89,  -88,  -87,  -86,  -85,  -84,  -83,  -82,  -81,
         -80,  -79,  -78,  -77,  -76,  -75,  -74,  -73,  -72,  -71,  -70,  -69,  -68,  -67,  -66,  -65,
         -64,  -63,  -62,  -61,  -60,  -59,  -58,  -57,  -56,  -55,  -54,  -53,  -52,  -51,  -50,  -49,
         -48,  -47,  -46,  -45,  -44,  -43,  -42,  -41,  -40,  -39,  -38,  -37,  -36,  -35,  -34,  -33,
         -32,  -31,  -30,  -29,  -28,  -27,  -26,  -25,  -24,  -23,  -22,  -21,  -20,  -19,  -18,  -17,
         -16,  -15,  -14,  -13,  -12,  -11,  -10,   -9,   -8,   -7,   -6,   -5,   -4,   -3,   -2,   -1,
           0,    1,    2,    3,    4,    5,    6,    7,    8,    9,   10,   11,   12,   13,   14,   15,
          16,   17,   18,   19,   20,   21,   22,   23,   24,   25,   26,   27,   28,   29,   30,   31,
          32,   33,   34,   35,   36,   37,   38,   39,   40,   41,   42,   43,   44,   45,   46,   47,
          48,   49,   50,   51,   52,   53,   54,   55,   56,   57,   58,   59,   60,   61,   62,   63,
          64,   65,   66,   67,   68,   69,   70,   71,   72,   73,   74,   75,   76,   77,   78,   79,
          80,   81,   82,   83,   84,   85,   86,   87,   88,   89,   90,   91,   92,   93,   94,   95,
          96,   97,   98,   99,  100,  101,  102,  103,  104,  105,  106,  107,  108,  109,  110,  111,
         112,  113,  114,  115,  116,  117,  118,  119,  120,  121,  122,  123,  124,  125,  126,  127
    },
};


// Write value to the DAC
void write(int val) {


    // MSB 6.4, 6.2, 6.1, 6.0, 3.4, 3.3, 6.6, 6.5  LSB


    // clear bits
    P6OUT &= ~(BIT0 | BIT1 | BIT2 | BIT4 | BIT5 | BIT6);
    P3OUT &= ~(BIT3 | BIT4);


    // mask val to only read the bottom 8 bits
    val &= 0b11111111;


    // update outputs
    P6OUT |= (val & 0b00000011) << 5;
    P6OUT |= (val & 0b01110000) >> 4;
    P6OUT |= (val & 0b10000000) >> 3;
    P3OUT |= (val & 0b00001100) << 1;


    // set write flag
    P4OUT = 0;
    P4OUT = BIT2;
}


// Read Harp input
unsigned int read() {
    // High Note 2.5, 2.4, 2.2, 7.4, 3.0, 1.2, 4.3, 4.0 Low Note
    unsigned int output = 0;
    output |= (P7IN & 0b00010000);
    output |= (P2IN & 0b00110000) << 2;
    output |= (P1IN & 0b00000100);
    output |= (P4IN & 0b00001000) >> 2;
    output |= (P4IN & 0b00000001);
    output |= (P3IN & 0b00000001) << 3;
    output |= (P2IN & 0b00000100) << 3;


    return output;
}






// This function was taken from the Texas Instruments sample code. See the copyright notice inside.
// Sets the Clock speed to 12 Mhz
void configureClock() {


/* --COPYRIGHT--,BSD_EX
 * Copyright (c) 2012, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************************
 *
 *                       MSP430 CODE EXAMPLE DISCLAIMER
 *
 * MSP430 code examples are self-contained low-level programs that typically
 * demonstrate a single peripheral function or device feature in a highly
 * concise manner. For this the code may rely on the device's power-on default
 * register values and settings such as the clock configuration and care must
 * be taken when combining code from several examples to avoid potential side
 * effects. Also see www.ti.com/grace for a GUI- and www.ti.com/msp430ware
 * for an API functional library-approach to peripheral configuration.
 *
 * --/COPYRIGHT--*/
 


    UCSCTL3 |= SELREF_2;                      // Set DCO FLL reference = REFO
    UCSCTL4 |= SELA_2;                        // Set ACLK = REFO


    __bis_SR_register(SCG0);                  // Disable the FLL control loop
    UCSCTL0 = 0x0000;                         // Set lowest possible DCOx, MODx
    UCSCTL1 = DCORSEL_5;                      // Select DCO range 24MHz operation
    UCSCTL2 = FLLD_1 + 374;                   // Set DCO Multiplier for 12MHz
                                            // (N + 1) * FLLRef = Fdco
                                            // (374 + 1) * 32768 = 12MHz
                                            // Set FLL Div = fDCOCLK/2
    __bic_SR_register(SCG0);                  // Enable the FLL control loop


    // Worst-case settling time for the DCO when the DCO range bits have been
    // changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
    // UG for optimization.
    // 32 x 32 x 12 MHz / 32,768 Hz = 375000 = MCLK cycles for DCO to settle
    __delay_cycles(375000);


    // Loop until XT1,XT2 & DCO fault flag is cleared
    do
    {
        UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
                                                // Clear XT2,XT1,DCO fault flags
        SFRIFG1 &= ~OFIFG;                      // Clear fault flags
    }while (SFRIFG1&OFIFG);                   // Test oscillator fault flag
   
}




void main(void){
    WDTCTL = WDTPW|WDTHOLD; // Stop WDT


    // Set clock to 12 Mhz
    configureClock();


    //////////////////////
    // INTERUPT TIMER
    //////////////////////
    TA1CCR0 = SAMPLE_RATE;
        // run interupt every SAMPLE_RATE = 512 cycles
    TA1CTL = TASSEL_2 + MC_1 + ID_0 + TAIE;
        // use the SMCLK (Sub-Main Clock) as our clock source
        // use timer mode 1 (continually count up to the value in TA1CCR0)
        // use no input divider
    TA1CCTL0 = CCIE;
        // enable the interupt




    //////////////////////
    // INIT OUTPUT
    //////////////////////
    P6DIR |= BIT0 | BIT1 | BIT2 | BIT4 | BIT5 | BIT6;
    P4DIR |= BIT1 | BIT2;
    P3DIR |= BIT3 | BIT4;


    P3OUT = 0;
    P4OUT = BIT2;
    P6OUT = 0;


    //////////////////////
    // INIT INPUT
    //////////////////////
    // P1.1 and P2.1 Button Inputs
    P1DIR &= ~(BIT1);
    P1REN |= BIT1;
    P1OUT |= BIT1;
   
    P2DIR &= ~(BIT1);
    P2REN |= BIT1;
    P2OUT |= BIT1;


   
    // Harp Input Pins
    // High note 2.5, 2.4, 2.2, 7.4, 3.0, 1.2, 4.3, 4.0 Low note
    P2REN |= BIT2 | BIT4 | BIT5;
    P1REN |= BIT3;
    P4REN |= BIT0 | BIT3;
    P3REN |= BIT0;
    P7REN |= BIT4;


    P2DIR &= ~(BIT2 | BIT4 | BIT5);
    P1DIR &= ~(BIT3);
    P4DIR &= ~(BIT0 | BIT3);
    P3DIR &= ~(BIT0);
    P7DIR &= ~(BIT7);


    P2OUT &= ~(BIT2 | BIT4 | BIT5);
    P1OUT &= ~(BIT3);
    P4OUT &= ~(BIT0 | BIT3);
    P3OUT &= ~(BIT0);
    P7OUT &= ~(BIT7);


    // Enable Interupts
    _bis_SR_register(GIE);




    //////////////////////
    // MAIN LOOP
    //////////////////////    
    while (1) {


        // Read Input
        unsigned int reading = read();


        // Inverse the reading
        reading = ~reading;


        // Update which notes are playing
        isPlaying[0] = (reading & (1 << 0)) ? NOTE_ON : NOTE_OFF;
        isPlaying[1] = (reading & (1 << 1)) ? NOTE_ON : NOTE_OFF;
        isPlaying[2] = (reading & (1 << 2)) ? NOTE_ON : NOTE_OFF;
        isPlaying[3] = (reading & (1 << 3)) ? NOTE_ON : NOTE_OFF;
        isPlaying[4] = (reading & (1 << 4)) ? NOTE_ON : NOTE_OFF;
        isPlaying[5] = (reading & (1 << 5)) ? NOTE_ON : NOTE_OFF;
        isPlaying[6] = (reading & (1 << 6)) ? NOTE_ON : NOTE_OFF;
        isPlaying[7] = (reading & (1 << 7)) ? NOTE_ON : NOTE_OFF;
       


        // Check for P1.1 input, and change the sound wave type (instrument) if pressed
        if (! (P1IN & BIT1)) {
            waveIndex++;
            if (waveIndex >= WAVE_COUNT) waveIndex = 0;
            while(!(P1IN & BIT1));
        }


        // Check for P2.1 input, and change the scale if pressed
        if (! (P2IN & BIT1)) {
            scaleIndex++;
            if (scaleIndex >= SCALE_COUNT) scaleIndex = 0;
            while(!(P2IN & BIT1));
        }
    }
}




// Sound mixing and sampling interupt routine. Runs once every SAMPLE_RATE cycles
void __attribute__ ((interrupt(TIMER1_A0_VECTOR))) PORT1_ISR(void) {
   
    // get current scale and waveform
    unsigned int* scale = SCALES + scaleIndex;
    int* wave = WAVES + waveIndex;


    // increment waveform positions for each note
    counts[0] += scale[0];
    counts[1] += scale[1];
    counts[2] += scale[2];
    counts[3] += scale[3];
    counts[4] += scale[4];
    counts[5] += scale[5];
    counts[6] += scale[6];
    counts[7] += scale[7];


    // calculate sample value
    int value =
        (wave[counts[0] >> 8] & isPlaying[0]) +
        (wave[counts[1] >> 8] & isPlaying[1]) +
        (wave[counts[2] >> 8] & isPlaying[2]) +
        (wave[counts[3] >> 8] & isPlaying[3]) +
        (wave[counts[4] >> 8] & isPlaying[4]) +
        (wave[counts[5] >> 8] & isPlaying[5]) +
        (wave[counts[6] >> 8] & isPlaying[6]) +
        (wave[counts[7] >> 8] & isPlaying[7]);
   


    // write sample value
    unsigned int writeValue = (value + 1024) >> 3;
    write(writeValue);


    // disable interupt flag
    TA1CTL &= ~TAIFG;
}
