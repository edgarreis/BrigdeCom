/*

		 COLUMN (Coluna)

                                           
             P6.0  P6.1  P6.2  P6.3                               
               ^     ^     ^     ^                                 
               |     |     |     |                                 
            |-----|-----|-----|-----|           
  P6.4 ---> |  1  |  2  |  3  |  A  |       
            |-----|-----|-----|-----|       
  P6.5 ---> |  4  |  5  |  6  |  B  |                 
            |-----|-----|-----|-----|      ROW (linha)                    
  P6.6 ---> |  7  |  8  |  9  |  C  |                              
            |-----|-----|-----|-----|                              
  P6.7 ---> |  *  |  0  |  #  |  D  |                              
            |-----|-----|-----|-----|                              


unsigned int count = {'1','2','3','A',                                          Teclado                              
                      '4','5','6','B',
                      '7','8','9','C',
                      '*','0','#','D'};

        
                                                                                Anotações Gerias 

// UCAxIFG -> Registrador que sinaliza se há interrupção pendente.              - Pág 918 User Guide Slau208n
// UCRXIFG -> 0b - Sem Interrupção pendente / 1b - Interrupção pendente         - Pág 918 User Guide Slau208n
// UCAxIFG -> Registrador que habilita interrupção na Uart.



                                           
                                                                                                                                                                                                                                                                                          
*/

#include "msp430f5529.h"
#include "lcd_Msp430F5529.h"

// Keypad Port 
#define COL1 	(0x10 & P6IN) 		// 0001 0000
#define COL2 	(0x20 & P6IN)		// 0010 0000
#define COL3 	(0x40 & P6IN)		// 0100 0000
#define COL4 	(0x80 & P6IN)		// 1000 0000
#define keyport P6OUT

// Variáveis
unsigned int count = 26, k=0, i=0, j=0, n=0, t=0, Rec = 50;
char Id_Cart[]={"Id. Card:           "};
unsigned char data_rec;         	// Dado recebido pela UART
int flag_pisca = 0;               	// Usado para verificar se o buffer esta cheio                   

int int_buffer_ADC10;                   // Variavel Int para o Buffer do Conversor ADC10
char char_buffer_ADC10[1];              // Variavel Char para o Buffer do Conversor ADC10
//unsigned int Results_A0;              // Variavel Int para o Buffer do Conversor ADC10


// Converte Int para String
void itoa(long unsigned int inteiro, char* string, int base){
    // por http://www.strudel.org.uk/itoa/

    // checa se a base é válida
    if (base < 2 || base > 36) {
        *string = '\0';
    }

    char *ptr = string, *ptr1 = string, tmp_char;
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

// Envia Byte p/ UART
void send_byte( unsigned char byte_send ){                       
   
    while(!(UCA0IFG & UCTXIFG));                                // UCA0TXIFG seta quando o buffer esta cheio
    UCA0TXBUF = byte_send;                                      // Enviar Bytes
}

// Envia Texto p/ UART
void send_text(const char  *ptr){ 
    
    while (*ptr)       
    send_byte(*(ptr++));                                        // Correr ponteiro no byte a ser enviado      
}

// Configurações do uC 
void config_uC (void){
  
    WDTCTL = WDTPW + WDTHOLD;               // Para o watch dog.
    
	// Configura Saídas
    P1DIR |=  BIT0;                         // Output P1.0
    P3DIR |=  BIT0 + BIT1;	            // Output P3.0 e P3.1
    P4DIR |=  BIT7;                         // Output P4.7
    
    P1OUT &= ~BIT0;                         // Forcar iniciar off
    P3OUT &= ~BIT0;                         // Forcar iniciar off
    P3OUT &= ~BIT1;                         // Forcar iniciar off
    P4OUT &= ~BIT7;                         // Forcar iniciar off

    // Configura Entradas
    P1REN |= BIT1;       		    // Habilita resistor de pullup/down no pino P1.1
    P1OUT |= BIT1;       		    // Define resistor de pull up no pino P1.1
    P1IE  |= BIT1;   	 		    // Habilita interrupção do botão P1.1
    P1IES |= BIT1;		            // Habilita interrupção na borda de descida
    P1IFG &=~BIT1; 			    // Zera a flag de interrupçãoo da porta 1
    
    // Conf_pins_Keypad
    P6DIR = 0x0F; 			    // Set P6.0 to 6.3 Output ,Set P6.4 to 6.7 Input.        1111 | 0000
    P6REN = 0xFF; 			    // Set P6.0 to 6.7 Pull up Register enable. 	     1111 | 1111
    P6OUT = 0xF0; 			    // Set P6.0 to 6.7 Out Register.                         1111 | 0000    

    
    // Setando os pinos para usar as Uart's
    P3SEL |= BIT3 + BIT4; 		    // P3.3,4 = USCI_A0 - Uart 0
    P4SEL |= BIT4 + BIT5; 	            // P4.4,5 = USCI_A1 - Uart 1
    
    // Inicialização da Uart 0
    UCA0CTL1    |= UCSWRST;		    // Inicialização da Uart 0
    UCA0CTL1    |= UCSSEL_2;                // SMCLK
    UCA0BR0     = 0x6;                      // 1MHz 9600
    UCA0BR1     = 0;                        // 1MHz 9600
    UCA0MCTL    |= UCBRS_0 + UCBRF_13 + UCOS16;		// = 1 Modulação UCBRSx
    UCA0CTL1    &= ~UCSWRST;                 // UCS Set
    UCA0IE      |= UCRXIE;                   // Transmit interrupt enable
    
    // Inicialização da Uart 1
    UCA1CTL1    |= UCSWRST;                  // UCS Reset
    UCA1CTL1    |= UCSSEL_2;		     // SMCLK
    UCA1BR0     = 0x68;                      // 1MHz 9600
    UCA1BR1     = 0;                         // 1MHz 9600
    UCA1MCTL    |= UCBRS_1 + UCBRF_0;	     // = 1 Modulação UCBRSx
    UCA1CTL1    &= ~UCSWRST;                 // UCS Set
    UCA1IE      |= UCRXIE;                   // Transmit interrupt enable
    

    // Config Timer
    TA0CTL = TASSEL_2 + ID_2 + MC_1 + TACLR;
      // SMCLK = fonte de clock do timer, MC_1 = contagem progressiva
      // ou seja de 0 até o valor de TACCR0, TACLR = reseta o timer A,
      // contagem inicia em zero de novo
    TA0CCTL0 |= CCIE;
      // CCIE = habilita interrupção de comparação do canal
    TA0CCR0 = 0x0FFF; 
      // TA0CCR1 = registrador de captura e comparação, esta relacionado com o TACCR0
      // Valor Máximo = 0xFFFE

    
    // Config. ADC
    P7SEL |= BIT0;           
      // Habilita canal de interrupção do A/D - P7.0
    ADC12CTL0 = ADC12ON + ADC12MSC + ADC12SHT0_8;
      // ADC12ON = liga o ADC,
      // ADC12MSC = seta multiplas conversões
      // ADC12SHT0_8 = seleção de amostragem
	
    ADC12CTL1 = ADC12SHP + ADC12CONSEQ_3 + ADC12SHS_0;
      // ADC12SHP = seleção de amostragem (definido pelos bits ADC12SHT0_8),
      // ADC12CONSEQ_3 = Sequencia de canais, repetitivo, SHS_1 = disparo através do timer

    //ADC12MCTL0 |= ADC12INCH_0;              // 000 = referencia AVcc, ADC12INCH_0 = seleção do canal de entrada - A0
    ADC12MCTL12 |= ADC12INCH_12 + ADC12EOS;    // A12 P7.0, EOS = Esse canal é o ultimo da sequencia da conversão.

    ADC12IE = 0x08;                           // Enable ADC12IFG.3

    __bis_SR_register (GIE);        // Enter LPM0, interrupções habilitado
}

// Impressão LCD
void Imp_Lcd(void){                                                             // Bloco de impressão padrão - Teste 
                                                                   
                if ( count == 26 ){
                 
                  Line1;
                  string("       UFPR         ");
                  Line2;
                  string("Eng. Eletrica - P.I.");
                  Line3;
                  string("Edgar R. - Marcos F.");
                  Line2;
                  string("                    ");

                }                                                                                        
                else if ( count == 1 ){                                         // Bloco de impressão - Teste 

                  Line2; string("     Tecla  = 1     ");

                }
                else if ( count == 2 ){

                 Line2; string("     Tecla  = 2     ");

                }
                else if ( count == 3 ){

                  Line2; string("     Tecla  = 3     ");

                }
                else if ( count == 123 ){                                       // A = 123
             
                  Line2; string("     Tecla  = A     ");

                } 
                else if ( count == 4 ){

                  Line2; string("     Tecla  = 4     ");

                }
                else if ( count == 5 ){

                  Line2; string("     Tecla  = 5     ");

                }
                else if ( count == 6 ){

                  Line2; string("     Tecla  = 6     ");

                }
                else if ( count == 456 ){                                       // B = 456
                
                  Line2; string("     Tecla  = B     ");

                } 
                else if ( count == 7 ){

                  Line2; string("     Tecla  = 7     ");

                }
                else if ( count == 8 ){

                  Line2; string("     Tecla  = 8     ");

                }
                else if ( count == 9 ){

                  Line2; string("     Tecla  = 9     ");

                }
                else if ( count == 789 ){                                       // C = 789
               
                  Line2; string("     Tecla  = C     ");

                }
                else if ( count == 10 ){                                        // * = 10
                
                  Line2; string("     Tecla  = *     ");

                }
                else if ( count == 0 ){
                
                  Line2;  string("     Tecla  = 0     ");

                }
                else if ( count == 11 ){                                        // # = 11 
               
                  Line2; string("     Tecla  = #     ");

                }
                else if ( count == 101 ){                                       // D = 101
               
                  Line2; string("     Tecla  = D     ");

                }
                else if ( count == 200 ){                                        // Id = 200
                  
                  if(t == 1){
                    
                      Line2; string(Id_Cart);
                      t=0;
                   
                  }
                }
}

// Varredura Teclado
void Var_Keypad(void){                                                          // Bloco de varredura do teclado - Teste 
  
  for(k=0;k<4;k++){	

		keyport = ((0x01<<k) ^ 0xff);					// Varredura das linhas, da seguinte forma: 	p/ i=0 -> 1111 1110,  = 0xFE
										// Lembrando  q P6.4, P6.5 P6.5 e P6.7 são:	p/ i=1 -> 1111 1101,  = 0xFD
										// entradas.					p/ i=2 -> 1111 1011 e = 0xFB
										//						p/ i=3 -> 1111 0111.  = 0xF7

			if(!COL1){						// So vai entrar nesse if, quando qualquer tecla da coluna 1 (COL1) - 0001 0000 	
			 							// for pressionado.Os n da coluna 1 são: 1, 2, 3, e A.
         
                                if (P6IN == 0xEE)				// - 1
                              
                                    count = 1;
                                
				if (P6IN == 0xED)				// - 2
                               
                                    count = 2;
                                  
				if (P6IN == 0xEB)				// - 3
                                
                                    count = 3;
                                
				if (P6IN == 0xE7)				// - A
                                     
                                    count = 123;                                
                                
				while(!COL1);					// Prende o programa enquanto o botão estiver pressionado.				
                                
			} 		
			if(!COL2){						// So vai entrar nesse if, quando qualquer tecla da coluna 2 (COL2) - 0010 0000
			 							// for pressionado.Os n da coluna 2 são: 4, 5, 6 e B. 

                                if (P6IN == 0xDE)				// - 4
                               
                                  count = 4;
                                  
				if (P6IN == 0xDD)				// - 5
                               
                                  count = 5;
                                
				if (P6IN == 0xDB)				// - 6
                            
                                  count = 6; 
                                
				if (P6IN == 0xD7)				// - B = 456
                              
                                  count = 456;
                                                                
				while(!COL2);					// Prende o programa enquanto o botão estiver pressionado. 								
                                
 			}			
			if(!COL3){						// So vai entrar nesse if, quando qualquer tecla da coluna 3 (COL3) - 0100 0000
			 							// for pressionado.Os n da coluna 3 são: 7, 8, 9, e C.

                                if (P6IN == 0xBE)				// - 7
                                
                                  count = 7;  
                                
				if (P6IN == 0xBD)				// - 8
                              
                                  count = 8;
                                
				if (P6IN == 0xBB)				// - 9
                               
                                  count = 9;                                    
                                
				if (P6IN == 0xB7)				// - C = 789
                               
                                  count = 789;                                     
                                
                                
				while(!COL3);					// Prende o programa enquanto o botão estiver pressionado. 
                                
			}
			if(!COL4){						// So vai entrar nesse if, quando qualquer tecla da coluna 4 (COL4) - 1000 0000
			 							// for pressionado.Os n da coluna 4 são: *, 0, # e D.

                                if (P6IN == 0x7E)				// - * = 10
                             
                                  count = 10;                                   
                                
				if (P6IN == 0x7D)				// - 0
                               
                                  count = 0;                                    
                                 
				if (P6IN == 0x7B)				// - # = 11
                             
                                  count = 11;                                   
                                
				if (P6IN == 0x77)				// - D = 101
                              
                                  count = 101;                                     
                                                        
				while(!COL4);					// Prende o programa enquanto o botão estiver pressionado.
                        
			}                        
        }
 }


int main(){

    config_uC();					// Configurações do uC    
    Inic_ports();					// Inicia as portas do display Lcd, Porta 2.
    lcd_init();						// Seta a s configurações do Lcd.   
 
    while(1){    
	Imp_Lcd();					// Impressão LCD
	Var_Keypad();   				// Varredura Teclado   
    }                                                                           
}                                                                               

// Interrupção UART0
#pragma vector = USCI_A0_VECTOR    	                // Interrupção de recebimento - UART0 --> Pinos P3.4(Rx) e  P3.3(Tx).
__interrupt void USCI_A0_ISR (void){			// Buffer responsável pelo recebimento: UCA0RXBUF

    data_rec = UCA0RXBUF;			        // Buffer recebe byte recebido

    if(data_rec == 0x31)
        P3OUT |= BIT0;              // P3.0 ON    
    if(data_rec == 0x32)
        P3OUT &= ~BIT0;             // P3.0 OFF    
    if(data_rec == 0x33)
        P3OUT |= BIT1;              // P3.1 ON
    if(data_rec == 0x34)
        P3OUT &= ~BIT1;             // P3.1 OFF   

    //UCA0IFG &= 0;
}

// Interrupção UART1
#pragma vector = USCI_A1_VECTOR                                                 // Interrupção de recebimento - UART1 --> Pinos P4.4 (Tx) e  P4.5 (Rx).
__interrupt void USCI_A1_ISR (void){                                            // Buffer responsável pelo recebimento: UCA1RXBUF
  
   if( (UCA1RXBUF == 2 && i ==0) || Rec == 1){                                  //Enquanto a Serial receber dados (ID)
	if ( i > 0 && i <=10 ){                                                 // Recolhe o somente a ID do cartão.
          Id_Cart[i+9] = UCA1RXBUF;                
        }        
   }
   
   i++;                      
   Rec = 1;                                                                 // Indica que esta sendo recebido um cartão.
  
   if ( i == 14){
    
       Rec = 0;
       i   = 0;       
       count = 200;
       t=1;
       
       // Enviar msg pela serial
       send_text(Id_Cart);
       send_byte('\n');      
    
   }   

   UCA1IFG &= 0;   
}


// Interrupção Pino 1.1
#pragma vector = PORT1_VECTOR       // Interrupção do botton
__interrupt void interr_P1(void){

    P4OUT ^= BIT7;          // Sinaliza Interrupção !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    
    flag_pisca ^= 1;		        // Inverte flag_pisca
    
    if (flag_pisca == 1){        
        send_text("LED ON\n");      // Enviar msg pela serial
    }    
    else{
        send_text("LED OFF\n");     // Enviar msg pela serial
    }
	
    P1IFG &= ~BIT1 ;	            // Zera a flag de interrupção da porta 2
}


// Interrupção Timer A0
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void){
  
    P1OUT ^= BIT0;          // Sinaliza Interrupção !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    ADC12CTL0 |= ADC12ENC + ADC12SC;
      // ADC12ENC = Habilita a conversão
      // ADC12SC  = Inicia a conversão do ADC12

}


// Interrupção ADC12
#pragma vector = ADC12_VECTOR
__interrupt void ADC12ISR (void){
      
      //Results_A0 = ADC12MEM0;         // Move A0 results, IFG is cleared
      int_buffer_ADC10 = ADC12MEM12;	        // Move A4 results, IFG is cleared         

      // Envia ACD para UART
      itoa(int_buffer_ADC10, char_buffer_ADC10, 10);          // int to char base 10
      send_text(char_buffer_ADC10);
      send_byte('\n');
      __delay_cycles(3200000);	        // delay de 1ms !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                  // 4000000
      TA0CCTL0 &=~ CCIFG;                 //Limpar flag de interrupção 
// PAREI AQUIIIIIIIIIIIIIIIII - ARRUMAR TEMPO DE ESTOURO DO TIMER PARA NÃO USAR O DELAY CYCLE
}

