/* --COPYRIGHT--,BSD
 * Copyright (c) 2014, Texas Instruments Incorporated
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
 * --/COPYRIGHT--*/
/*******************************************************************************
 *
 * main.c
 *
 * Simple skeleton project to use MSP430FR4133
 * Main loop, initialization, and interrupt service routines
 * Adapted from TI's out of box sample codes
 *
 * August 2018
 * Ben Wai-Kong Lee
 *
 ******************************************************************************/
#include "main.h"
#include "hal_LCD.h"
#include "TempSensorMode.h"
#include "aes256.h"

#include <stdio.h>
#include <msp430.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>

unsigned char tempSensorRunning = 0;      // Temp Sensor running flag
static unsigned long long ctr = 0;
static int isinit = 0;

unsigned short test = 0;

void Init_GPIO();
void Init_Clock();
void Init_LCD();
void Init_UART();
// TimerA0 UpMode Configuration Parameter
Timer_A_initUpModeParam initUpParam_A0 =
{
     TIMER_A_CLOCKSOURCE_SMCLK,              // SMCLK Clock Source
     TIMER_A_CLOCKSOURCE_DIVIDER_1,          // SMCLK/1 = 2MHz
     30000,                                  // 15ms debounce period
     TIMER_A_TAIE_INTERRUPT_DISABLE,         // Disable Timer interrupt
     TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE ,    // Enable CCR0 interrupt
     TIMER_A_DO_CLEAR,                       // Clear value
     true                                    // Start Timer
};

static void init_ctra()
{
  TA0CTL = TACLR;
  TA0CTL = TASSEL_2 + MC_2 + TAIE + ID_3;

  TA0CCTL0 |= CCIE;                             // TACCR0 interrupt enabled
//  TA0CCR0 = 50000;
//  TA0CTL |= TASSEL__SMCLK | MC__CONTINOUS;      // SMCLK, continuous mode
//  eint();
  isinit = 1;
}

unsigned long long cpucycles()
{
  if(!isinit) init_ctra();
  return (ctr | TA0R) << 3;
}

void printString(char *str, int len)
{
    unsigned int i=0;
    for(i=0; i<len; i++)
    {
        while(!(UCA0IFG & UCTXIFG));
        UCA0TXBUF = str[i];                   // Load data onto buffer
        __delay_cycles(1000);
    }
}

void exercise1(void)
{
    char buffer[64] = {0};
    aes256_context ctx;
    uint8_t key[32];
    uint8_t buf[16], i;
    unsigned short temps[8];
    unsigned int start, end;
    /* put a test vector */
    for(i=0;i<8;i++) temps[i] = tempSensorExercise1();
    for (i = 0; i < sizeof(buf);i+=2)
        {
            buf[i] = (uint8_t)temps[i/2]&0xFF; //lower byte
            buf[i+1] = (uint8_t)((temps[i/2]&0xFF00)>>8); //upper byte
        }
    for (i = 0; i < sizeof(key);i++) key[i] = i;

    // view the key in serial port (only first 16 bytes)
    sprintf(buffer, "Key: %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\r\n", key[0], key[1], key[2], key[3], key[4], key[5], key[6], key[7], key[8], key[9], key[10], key[11], key[12], key[13], key[14], key[15]);
    printString(buffer, sizeof(buffer));

    // view the plaintext in serial port
    sprintf(buffer, "PText: %d %d %d %d %d %d %d %d \r\n", (uint16_t)(buf[0]+(buf[1]<<8)),(uint16_t)(buf[2]+(buf[3]<<8)),(uint16_t)(buf[4]+(buf[5]<<8)),(uint16_t)(buf[6]+(buf[7]<<8)),(uint16_t)(buf[8]+(buf[9]<<8)),(uint16_t)(buf[10]+(buf[11]<<8)),(uint16_t)(buf[12]+(buf[13]<<8)),(uint16_t)(buf[14]+(buf[15]<<8)));
    printString(buffer, sizeof(buffer));
    aes256_init(&ctx, key);
    memset(buffer, 0, sizeof(buffer));

    // start timing CPU cycles
    start=cpucycles();
    aes256_encrypt_ecb(&ctx, buf);
    end=cpucycles();
    sprintf(buffer, "Encryption takes %d clock cycles\r\n", end-start);
    printString(buffer, sizeof(buffer));

    // view the ciphertext in serial port
    sprintf(buffer, "CText: %d %d %d %d %d %d %d %d \r\n", (uint16_t)(buf[0]+(buf[1]<<8)),(uint16_t)(buf[2]+(buf[3]<<8)),(uint16_t)(buf[4]+(buf[5]<<8)),(uint16_t)(buf[6]+(buf[7]<<8)),(uint16_t)(buf[8]+(buf[9]<<8)),(uint16_t)(buf[10]+(buf[11]<<8)),(uint16_t)(buf[12]+(buf[13]<<8)),(uint16_t)(buf[14]+(buf[15]<<8)));
    printString(buffer, sizeof(buffer));

    // expected ciphertext: 8e a2 b7 ca 51 67 45 bf ea fc 49 90 4b 49 60 89

    aes256_init(&ctx, key);
    start = cpucycles();
    aes256_decrypt_ecb(&ctx, buf);
    end = cpucycles();
    aes256_done(&ctx);
    
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "Decryption takes %d clock cycles\r\n", end-start);
    printString(buffer, sizeof(buffer));


    sprintf(buffer, "PText: %d %d %d %d %d %d %d %d \r\n", (uint16_t)(buf[0]+(buf[1]<<8)),(uint16_t)(buf[2]+(buf[3]<<8)),(uint16_t)(buf[4]+(buf[5]<<8)),(uint16_t)(buf[6]+(buf[7]<<8)),(uint16_t)(buf[8]+(buf[9]<<8)),(uint16_t)(buf[10]+(buf[11]<<8)),(uint16_t)(buf[12]+(buf[13]<<8)),(uint16_t)(buf[14]+(buf[15]<<8)));
    printString(buffer, sizeof(buffer));
}

void verify_aes256()
{
    char buffer[64] = {0};
    aes256_context ctx;
    uint8_t key[32];
    uint8_t buf[16], i;
    unsigned int start, end;
    /* put a test vector */
    for (i = 0; i < sizeof(buf);i++) buf[i] = i * 16 + i;
    for (i = 0; i < sizeof(key);i++) key[i] = i;

    // view the key in serial port (only first 16 bytes)
    sprintf(buffer, "Key: %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\r\n", key[0], key[1], key[2], key[3], key[4], key[5], key[6], key[7], key[8], key[9], key[10], key[11], key[12], key[13], key[14], key[15]);
    printString(buffer, sizeof(buffer));

    // view the plaintext in serial port
    sprintf(buffer, "PText: %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\r\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
    printString(buffer, sizeof(buffer));
    aes256_init(&ctx, key);
    memset(buffer, 0, sizeof(buffer));

    // start timing CPU cycles
    start=cpucycles();
    aes256_encrypt_ecb(&ctx, buf);
    end=cpucycles();
    sprintf(buffer, "Encryption takes %d clock cycles\r\n", end-start);
    printString(buffer, sizeof(buffer));

    // view the ciphertext in serial port
    sprintf(buffer, "CText: %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\r\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
    printString(buffer, sizeof(buffer));

    // expected ciphertext: 8e a2 b7 ca 51 67 45 bf ea fc 49 90 4b 49 60 89

    aes256_init(&ctx, key);
    aes256_decrypt_ecb(&ctx, buf);
    aes256_done(&ctx);
}

/*
 * main.c
 */
int main(void) {
    // Stop Watchdog timer
    WDT_A_hold(__MSP430_BASEADDRESS_WDT_A__);     // Stop WDT

      // Initializations
    Init_GPIO();
    Init_Clock();
    Init_LCD();
    Init_UART();

    GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN2);
    GPIO_clearInterrupt(GPIO_PORT_P2, GPIO_PIN6);

    __enable_interrupt();

    while(1)
    {
        LCD_E_selectDisplayMemory(LCD_E_BASE, LCD_E_DISPLAYSOURCE_MEMORY);
        clearLCD();              // Clear all LCD segments
        tempSensorModeInit();    // initialize temperature mode

        exercise1();

        while(1);
        //verify_aes256();
        //tempSensor();

        __no_operation();
    }
}

/*
 * GPIO Initialization
 */
void Init_GPIO()
{
    // Set all GPIO pins to output low to prevent floating input and reduce power consumption
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P7, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);

    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P6, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P7, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P8, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);


    // Set P4.1 and P4.2 as Secondary Module Function Input, LFXT.
    GPIO_setAsPeripheralModuleFunctionInputPin(
           GPIO_PORT_P4,
           GPIO_PIN1 + GPIO_PIN2,
           GPIO_PRIMARY_MODULE_FUNCTION
           );

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PMM_unlockLPM5();
}

void Init_UART()
{
    // Configure UART pins
      P1SEL0 |= BIT0 | BIT1;                    // set 2-UART pin as second function

    // Configure UART
      UCA0CTLW0 |= UCSWRST;
      UCA0CTLW0 |= UCSSEL__SMCLK;

    // Baud Rate calculation
    // 8000000/(16*9600) = 52.083
    // Fractional portion = 0.083
    // User's Guide Table 14-4: UCBRSx = 0x49
    // UCBRFx = int ( (52.083-52)*16) = 1
      UCA0BR0 = 52;                             // 8000000/16/9600
      UCA0BR1 = 0x00;
      UCA0MCTLW = 0x4900 | UCOS16 | UCBRF_1;

      UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI
      UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
}


void Init_Clock()
{
    __bis_SR_register(SCG0);                 // disable FLL
    CSCTL3 |= SELREF__REFOCLK;               // Set REFO as FLL reference source
    CSCTL0 = 0;                              // clear DCO and MOD registers
    CSCTL1 &= ~(DCORSEL_7);                  // Clear DCO frequency select bits first
    CSCTL1 |= DCORSEL_3;                     // Set DCO = 8MHz
    CSCTL2 = FLLD_0 + 243;                   // DCODIV = 8MHz
    __delay_cycles(3);
    __bic_SR_register(SCG0);                 // enable FLL
    while(CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1)); // Poll until FLL is locked

    CSCTL4 = SELMS__DCOCLKDIV | SELA__REFOCLK; // set default REFO(~32768Hz) as ACLK source, ACLK = 32768Hz
                                             // default DCODIV as MCLK and SMCLK source}
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A0_VECTOR))) USCI_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch(__even_in_range(UCA0IV,USCI_UART_UCTXCPTIFG))
    {
        case USCI_NONE: break;
        case USCI_UART_UCRXIFG:
        case USCI_UART_UCTXIFG: break;
        case USCI_UART_UCSTTIFG: break;
        case USCI_UART_UCTXCPTIFG: break;
    }
}

// Timer A0 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer_A (void)
#else
#error Compiler not supported!
#endif
{
    ctr += 0x10000ULL;
    TA0IV = 0;
}
/*
 * RTC Interrupt Service Routine
 * Wakes up every ~10 milliseconds to update stowatch
 */
#pragma vector = RTC_VECTOR
__interrupt void RTC_ISR(void)
{

}

/*
 * ADC Interrupt Service Routine
 * Wake up from LPM3 when ADC conversion completes
 */
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=ADC_VECTOR
__interrupt void ADC_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(ADC_VECTOR))) ADC_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch(__even_in_range(ADCIV,ADCIV_ADCIFG))
    {
        case ADCIV_NONE:
            break;
        case ADCIV_ADCOVIFG:
            break;
        case ADCIV_ADCTOVIFG:
            break;
        case ADCIV_ADCHIIFG:
            break;
        case ADCIV_ADCLOIFG:
            break;
        case ADCIV_ADCINIFG:
            break;
        case ADCIV_ADCIFG:
            // Clear interrupt flag
            ADC_clearInterrupt(ADC_BASE, ADC_COMPLETED_INTERRUPT_FLAG);
            __bic_SR_register_on_exit(LPM3_bits);                // Exit LPM3
            break;
        default:
            break;
    }
}
