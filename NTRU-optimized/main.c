#include <stdio.h>
#include <msp430.h>
#include "main.h"
#include "ntru.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>

void Init_GPIO();
void printString(char *str, int len);
void Init_Clock();
static void init_ctra();
static uint64_t ctr = 0;
static int isinit = 0;

// Transmit a string to serial port for debug purpose
void printString(char *str, int len)
{
    unsigned int i=0;
    for(i=0; i<len; i++)
    {

        while(!(UCA0IFG & UCTXIFG));
        UCA0TXBUF = str[i];                   // Load data onto buffer
//        __delay_cycles(1500);
        __no_operation();
    }
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

static void init_ctra()
{
  TA0CTL = TACLR;
  TA0CTL = TASSEL_2 + MC_2 + TAIE + ID_3;
  TA0CCTL0 |= CCIE;                             // TACCR0 interrupt enabled
  isinit = 1;
}

uint64_t cpucycles()
{
  if(!isinit) init_ctra();
  return (ctr | TA0R) << 3;
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

// The data needed to convert a number mod 3 to a number in the range: -1..1.
static short neg_mod_3_data_cpu[5] = { 1, -1, 0, 1, -1 };
// The variable used to convert a number mod 3 to a number in the range: -1..1.
static short *neg_mod_3_cpu = &neg_mod_3_data_cpu[2];
static unsigned char data_dec_data[3] = { 0, 1, 1 };    // for data encode/decode
static unsigned char *data_dec = &data_dec_data[1];


uint16_t ind(uint16_t x, uint16_t y, uint16_t deg)
{
    if (x>y)    return (x - y);
    else if (x == y) return 0;
    else return (deg + (x - y));
}

void simplePolyMulModQ(int16_t *res, int16_t *polyA, int8_t *polyB, uint16_t deg,
    uint16_t q)
{
    uint16_t i=0, j=0;
    for (i = 0; i<deg; i++)
    {
        for (j = 0; j<deg; j++)
        {
            // truncated polynomial multiplication
        }
    }
}


//polyB must be 8-bit (msg)
void simplePolyAddModQ(int16_t *res, int16_t *polyA, int8_t *polyB, uint16_t deg,
    uint16_t q)
{
    uint16_t i=0;
    for (i = 0; i<deg; i++)
    {
        // polynomial addition
    }

}

// Simple NTRU-112 enc/dec
#pragma NOINIT (e)
int16_t e[NTRU_N_112];
#pragma NOINIT (a)
int16_t a[NTRU_N_112];
void test_NTRU_112()
{
    uint16_t i=0;
    char buffer[50]={0};
    uint64_t start, end;
    memset(a, 0, sizeof(a));
    memset(e, 0, sizeof(e));
    sprintf(buffer, "Start NTRU\r\n");
    printString(buffer, sizeof(buffer));
    //  Encrypt e = r*h + m
    start=cpucycles();



    end=cpucycles();
    sprintf(buffer, "Encrypted Message\r\n");
    printString(buffer, sizeof(buffer));
    sprintf(buffer, "NTRU encryption takes %lu clock cycles\r\n", end-start);
    printString(buffer, sizeof(buffer));
    for (i = 0; i<NTRU_N_112; i=i+8)
    {
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\r\n", e[i], e[i+1], e[i+2], e[i+3], e[i+4], e[i+5], e[i+6], e[i+7] );
        printString(buffer, sizeof(buffer));
    }
    memset(buffer, 0, sizeof(buffer));
    start=cpucycles();
    //  Decrypt a = f*e



    //  Calculate mod p to isolate the message/key.
    for (i = 0; i<NTRU_N_112; i++) a[i] = neg_mod_3_cpu[a[i]%3];
    end=cpucycles();
    sprintf(buffer, "Decrypted Message\r\n");
    printString(buffer, sizeof(buffer));
    sprintf(buffer, "NTRU decryption takes %lu clock cycles\r\n", end-start);
    printString(buffer, sizeof(buffer));

    for (i = 0; i<NTRU_N_112; i=i+8)
    {
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\r\n", a[i], a[i+1], a[i+2], a[i+3], a[i+4], a[i+5], a[i+6], a[i+7]);
        printString(buffer, sizeof(buffer));
    }

}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                // Stop watchdog timer

  // Configurations
    Init_GPIO();
    init_ctra();
    Init_Clock();
    Init_UART();
    PM5CTL0 &= ~LOCKLPM5;                    // Disable the GPIO power-on default high-impedance mode
                                           // to activate 1previously configured port settings
    __enable_interrupt();
    test_NTRU_112();
    __no_operation();                         // For debugger
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
    case USCI_UART_UCRXIFG: break;
    case USCI_UART_UCTXIFG: break;
    case USCI_UART_UCSTTIFG: break;
    case USCI_UART_UCTXCPTIFG: break;
    default: break;
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

void Init_GPIO()
{
    P1DIR = 0xFF; P2DIR = 0xFF; P3DIR = 0xFF; P4DIR = 0xFF;
    P5DIR = 0xFF; P6DIR = 0xFF; P7DIR = 0xFF; P8DIR = 0xFF;
    P1REN = 0xFF; P2REN = 0xFF; P3REN = 0xFF; P4REN = 0xFF;
    P5REN = 0xFF; P6REN = 0xFF; P7REN = 0xFF; P8REN = 0xFF;
    P1OUT = 0x00; P2OUT = 0x00; P3OUT = 0x00; P4OUT = 0x00;
    P5OUT = 0x00; P6OUT = 0x00; P7OUT = 0x00; P8OUT = 0x00;
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

