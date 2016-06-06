/*

Nessa configuração a Uart esta livre para enviar e receber dados!               Modificado: 13-04-2016 - Marcos Fabiano

-Connections:(pinos de controle) ]

  (4)  RS  -> P2.0 - Controla o tipo de comunicação: 0 para comandos e 1 para dados!

  (6)  EN  -> P2.3- Controle de envio: Quando estiver escrevendo para o display, os dados serão transmitidos apenas a partir de 
                                  uma transição de high para low (H -> L) deste sinal.
                                  No entanto, para ler informações do display, as informações estarão disponíveis imediatamente
                                  após uma transição L -> H e permanecerá lá até que o sinal volte para o nível lógico baixo (0)
                                  novamente.  

  (5)  R/W -> 0V - Controla o tipo de ação: 0 para escrita e 1 para leitura de dados!

        (4) RS  -> P2.0
	(6) EN  -> P2.3
   	(5) R/W -> Terra

 	(14) D7  -> P2.7
 	(13) D6  -> P2.6
 	(12) D5  -> P2.5
 	(11) D4  -> P2.4


						MSP430G2553 				                        MSP430F5529 
				|---------------------------------------|			|---------------------------------------|
				|					|			|				        |						|
				| 	|P1.0|  -> Lcd 			|                       | 	|P2.0|  -> Lcd 			|
				|	|P1.1|	-> MSP430 - ESP | UART	|			|	|P2.1|	-> XXX			|
				|	|P1.2|	-> MSP430 - ESP |	|			|	|P2.2|	-> XXX			|
				|	|P1.3|	-> Lcd			|			|	|P2.3|	-> Lcd			|
            Porta 1		|	|P1.4|	-> Lcd			|	Porta 2		|	|P2.4|	-> Lcd			|
				|	|P1.5|	-> Lcd			|			|	|P2.5|	-> Lcd			|
				|	|P1.6|	-> Lcd			|			|	|P2.6|	-> Lcd			|	
   				|	|P1.7|	-> Lcd			|			|	|P2.7|	-> Lcd			|
				|					|			|					|										|
				|---------------------------------------|			|---------------------------------------|
 
*/

#define EN(X)   P2OUT = ((P2OUT & ~(BIT3)) | (X<<3))
#define LCD_STROBE do{EN(1);EN(0);}while(0)
#define databits P2OUT                                                          // P6.7 - D7, ....., P6.4 - D4
#define Line1 comando_instr(0x80)
#define Line2 comando_instr(0xc0)
#define Line3 comando_instr(0x94)
#define Line4 comando_instr(0xd4)


void Inic_ports()                                                               // Função configura as portas que vai ser usadas para se escrever no Lcd!
{
    P2OUT = 0 ;                                                                 // Coloca zero volts nas I/O ou portas em low!
    P2DIR = 0xff;                                                               // Coloca todas as I/O como saída!
}
 
void data(unsigned char c)                                                      // Função que envia dados, o que se quer ver no LCD! RS = 1;
{
    P2OUT &= ~(BIT0);
    P2OUT |= 1; 
    __delay_cycles(40);                                                         //40 us delay
    databits = (databits & 0x0f) | (c & 0xf0);
    LCD_STROBE;
    databits = (databits & 0x0f) | (c << 4) ;
    LCD_STROBE;
}
 
void comando_instr(unsigned char c)                                             // Função que envia as instruções para configuração do LCD! RS = 0;
{
    P2OUT &= ~(BIT0);
    P2OUT |= 0; 
    __delay_cycles(40);                                                         //40 us delay
    databits = (databits & 0x0f) | (c & 0xf0);
    LCD_STROBE;
    databits = (databits & 0x0f) | (c << 4) ;
    LCD_STROBE;
}
 
void pseudo_8bit_cmd(unsigned char c)
{
    P2OUT &= ~(BIT0);
    P2OUT |= 0;          
    __delay_cycles(15000);                                                      //15 ms delay
    databits = (c & 0xf0);
    LCD_STROBE;
}
void clear(void)
{
    comando_instr(0x01);
    __delay_cycles(3000);                                                       //3 ms delay
}
 
void lcd_init()
{
    pseudo_8bit_cmd(0x30);                                                      //this command is like 8 bit mode command
    pseudo_8bit_cmd(0x30);                                                      //lcd expect 8bit mode commands at first
    pseudo_8bit_cmd(0x30);                                                      //for more details, check any 16x2 lcd spec
    pseudo_8bit_cmd(0x20);
    comando_instr(0x28);                                                        //4 bit mode command started, set two line
    comando_instr(0x0c);                                                        // Make cursorinvisible
    clear();                                                                    // Clear screen
    comando_instr(0x6);                                                         // Set entry Mode(auto increment of cursor)
}
 
void string(char *p)
{
    while(*p) data(*p++);
}
