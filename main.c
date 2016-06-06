


#include "msp430f5529.h"

// Variaveis Globais
unsigned char data_rec;         // Dado recebido pela UART
unsigned int int_rec;           // Dado recebido pela UART
//unsigned char data_send = 0x06; // Dado enviado pela UART
int flag_pisca = 0;             // Usado para verificar se o buffer esta cheio
int x;                          // Flag x
int int_buffer_ADC10;               // Variavel para o Buffer do Conversor ADC10
//unsigned char buffer;           // Variavel para o Buffer
char char_buffer_ADC10[5];


void itoa(long unsigned int inteiro, char* string, int base){
	// por http://www.strudel.org.uk/itoa/

    // checa se a base é válida
    if (base < 2 || base > 36) {
        *string = '\0';
    }

    char* ptr = string, *ptr1 = string, tmp_char;
    int tmp_inteiro;

    do {
        tmp_inteiro = inteiro;
        inteiro /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_inteiro - inteiro * base)];
    } while ( inteiro );

    // Aplica sinal negativo
    if (tmp_inteiro < 0) *ptr++ = '-';

    *ptr-- = '\0';

    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
}

/* Envia Byte */
void send_byte( unsigned char byte_send )
{
    while(!(UCA0IFG & UCTXIFG)); // UCA0TXIFG seta quando o buffer esta cheio
    UCA0TXBUF = byte_send;      // Enviar Bytes
}


/* Enviar Texto (Utilizando Ponteiro) */
//void send_text(const int8_t *ptr){
void send_text(const char  *ptr){
    while (*ptr){
        send_byte(*ptr);        // Correr ponteiro no byte a ser enviado
        ptr++;                  // Proxima letra da palavra
    }
}

void config_uC (void){

    WDTCTL = WDTPW + WDTHOLD;   // Stop watchdog.

    // Configura Saídas
    P1DIR |=  BIT0;             // Output P1.0
    P4DIR |=  BIT7;             // Output P4.7
    P1OUT &= ~BIT0;             // Forcar iniciar off
    P4OUT &= ~BIT7;             // Forcar iniciar off

    // Configura Entradas
    P2REN |= BIT1;       		// Habilita resistor de pullup/down no pino P2.1
    P2OUT |= BIT1;       		// Define resistor de pull up no pino P2.1
    P2IE  |= BIT1;   	 		// Habilita interrupção do botão P1.3
    P2IES |= BIT1;				// Habilita interrupção na borda de descida
    P2IFG |= BIT1; 				// Zera a flag de interrupçãoo da porta 1

    // Inicialização da Uart 0
    UCA0CTL1    |= UCSWRST;
    UCA0CTL1    |= UCSSEL_2;            // SMCLK
    UCA0BR0     = 0x68;                 // 1MHz 9600
    UCA0BR1     = 0;                    // 1MHz 9600
    UCA0MCTL    |= UCBRS_1 + UCBRF_0;	// = 1 Modulação UCBRSx
    UCA0CTL1    &= ~UCSWRST;
    UCA0IE      |= UCRXIE;

    // Inicialização da Uart 1
    UCA1CTL1    |= UCSWRST;
    UCA1CTL1    |= UCSSEL_2;            // SMCLK
    UCA1BR0     = 0x68;                 // 1MHz 9600
    UCA1BR1     = 0;                    // 1MHz 9600
    UCA1MCTL    |= UCBRS_1 + UCBRF_0;   // = 1 Modulação UCBRSx
    UCA1CTL1    &= ~UCSWRST;
    UCA1IE      |= UCRXIE;

    __bis_SR_register (GIE);            // Enter LPM0, interrupções habilitado

}


int main(){

	config_uC();

    while(1){

    }                                                                           // --> Fechamento do while(1)
}                                                                               // --> Fechamento do main()

/*

#pragma vector = USCI_A1_VECTOR                                                 // Interrupção de recebimento - UART1 --> Pinos P4.4 (Tx) e  P4.5 (Rx).
__interrupt void USCI_A1_ISR (void){                                            // Buffer responsável pelo recebimento: UCA1RXBUF

    UCA1IFG &= 0;

}
 */

#pragma vector = USCI_A0_VECTOR                                                 // Interrupção de recebimento - UART0 --> Pinos P3.4 (Rx) e  P3.5 (Tx).
__interrupt void USCI_A0_ISR (void){                                            // Buffer responsável pelo recebimento: UCA0RXBUF

	data_rec = UCA0RXBUF;     // Buffer recebe byte recebido

    if(data_rec == 0x31){
        P1OUT |= BIT0;        // P1.0 ON
    }
    if(data_rec == 0x32){
        P1OUT &= ~BIT0;       // P1.0 OFF
    }
    if(data_rec == 0x33){
        P4OUT |= BIT7;        // P4.7 ON
    }
    if(data_rec == 0x34){
        P4OUT &= ~BIT7;       // P4.7 OFF
    }

    UCA0IFG &= 0;

}


/* Interrupção do P2.1 */
#pragma vector = PORT2_VECTOR
__interrupt void interr_P2(void){

    //P1OUT ^= 0x01;              // Sinalizar interrupção

    flag_pisca ^= 1;		// Inverte flag_pisca
    if (flag_pisca == 1){
        // Enviar msg pela serial
        send_text("Botton ON\n");
    }
    else{
        // Enviar msg pela serial
        send_text("Botton OFF\n");
    }

    P2IFG = 0x00;		// Zera a flag de interrupção da porta 1

    /*
    // DEBAUNCING
    __delay_cycles(8000);	// delay de 1ms
    x = 1;			// garante que irá entrar no while

    while(x == 1)
    if((P1IN & 0x08) == 0){     // testa se o botão p1.3 está pressionado
        x = 0;		        // muda estatus de x para sair do while
    }
    else{			// se não estiver pressionado volta a conferir
        x = 1;
    }
    __delay_cycles(8000);	// delay de 1ms
    */

}



