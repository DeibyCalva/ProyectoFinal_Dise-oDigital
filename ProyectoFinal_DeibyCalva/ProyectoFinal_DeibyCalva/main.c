/*
 * ProyectoFinal_DeibyCalva.c
 *
 * Created: 27/2/2020 17:48:59
 * Author : Deiby Calva
 */ 

#define F_CPU 16000000UL // Frecuencia de CPU en Hz.
#include <avr/io.h>    // contiene la definicion de registros y sus respectivos bits, nombres logicos de cada nombre PIN (OCR0A, OCIE0A)
#include <stdlib.h>    // contiene funciones basicas es necesaria para poder utilizar la funcion itoa(), que transforma un valor entero en una cadena de caracteres
#include <stdbool.h>   // permite usar bool como un tipo de datos booleano. true evalúa como 1 y false evalúa como 0
#include <util/delay.h>// Funciones convenientes para bucles de retardo de espera ocupada
#include <avr/eeprom.h>// Esta biblioteca le permite leer y escribir esos bytes
#include "lcd.c"        //libreria o mas bien es un codigo desarrollado que contiene algunas de las funciones mas cotidianas para el uso del LCD
#include "mat_kbrd.c"  // se carga la libreria del teclado
#include "uart.c"      //Trasmisor receptor asincrono universal controla los puertos y dispositivos en serie



int main(void)
{
	/* Replace with your application code */
	

	void Wait() // Realiza una espera para que el servomotor pueda girar
	{
		uint8_t i;
		for(i=0;i<10;i++)
		{
			//Espera un tiempo
			_delay_loop_2(0);
			_delay_loop_2(0);
			_delay_loop_2(0);
		}

	}
	
	DDRF=0x07;    // Puertos de salida
	PORTF=0x00;   // Inician en Cero
	
	#define eeprom_true 0
	#define eeprom_data 1
	FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);
	char claveMaestra[5]= "1*8#"; // Clave maestra que sirve para cambiar la contraseña hija 
	bool Bmaestra = false;        // Sirve para saber si se ha ingresado la clave maestra
	eeprom_write_word(1, claveMaestra[0] - '0') ; // se guarda la clave maestra en la eeprom
	eeprom_write_word(2, claveMaestra[1] - '0') ;
	eeprom_write_word(3, claveMaestra[2] - '0') ;
	eeprom_write_word(4, claveMaestra[3] - '0') ;
	char cl[4]; // SE recupera de la eeprom la clave Maestra
	cl[0] = eeprom_read_word(1) + '0';
	cl[1] = eeprom_read_word(2) + '0';
	cl[2] = eeprom_read_word(3) + '0';
	cl[3] = eeprom_read_word(4) + '0';
	
	char claveNueva[4]; // se recupera de la eeprom la clave que ingreso el usuario 
	claveNueva[0] = eeprom_read_word(6) ;
	claveNueva[1] = eeprom_read_word(7) ;
	claveNueva[2] = eeprom_read_word(8) ;
	claveNueva[3] = eeprom_read_word(9) ;
	char key; // variable de caracter
	int i=0;  // inicia en cero
	lcd_init(LCD_DISP_ON); // se inicia o activa la LCD 
	kbrd_init();		   // Se inicia el teclado
	lcd_home();			   // Esa ubicación al enviar el texto posterior a la pantalla.
	lcd_clrscr();			//Limpiar la pantalla del LCD
	lcd_puts("INGRESE CLAVE MA: ");//Imprime en la LCD el mensaje
	_delay_ms(50);         // 50 ms de retraso
	char cont[5]="00000"; // variable de contador de caracteres en un arreglo
	int x=0;
	
	while (1)
	{
		
		key = kbrd_read(); // LEE EL TECLADO
		if (key != 0)
		{
			PORTF=0x01;   // Se activa el puerto del buzzer para que suene al momento de digitar cada tecla
			_delay_ms(50);// 50 ms de retraso
			PORTF=0x00;   // se desactiva el puerto del buzzer
			lcd_gotoxy(i, 1); //Mueve el cursor a la posicion x=i y=0
			cont[i]=  key;   // i se almacena en la variable contador 
			lcd_putc(cont[i]);// imprime el caracter i en la posicion 0 y esto va ir 
			i++;             // incrementando hasta que llegue las teclas del matricial
			if (!Bmaestra)
			{
				if (i==4 && Bmaestra == false)
				{
					if (cont[0] == cl[0] &&cont[1] == cl[1] && cont[2] == cl[2] && cont[3] == cl[3])// compara la contraseña ingresada por el usuario
					// con la contraseña maestra
					{
						lcd_clrscr();       //Limpiar la pantalla del LCD
						lcd_puts("CORRECTO");//Imprime en la LCD el mensaje
						PORTF=0x04;          // Se activa el puerto del buzzer y LED en PORTF2
						_delay_ms(1000);     //1000 ms de retraso
						PORTF=0x00;			 //se desactiva el puerto del buzzer y del LED
						Bmaestra = true;    
						i=0;
						x=0;
						lcd_clrscr();		//Limpiar la pantalla del LCD
						lcd_puts("NUEVA CLAVE: ");//Imprime en la LCD el mensaje
						_delay_ms(2000);    //2000 ms de retraso
					}
					else
					{    //Compara la contraseña ingresada ingresada con la contraseña normal
						if (cont[0] == claveNueva[0] &&cont[1] == claveNueva[1] && cont[2] == claveNueva[2] && cont[3] == claveNueva[3])
						{
							lcd_clrscr();    //Limpiar la pantalla del LCD
							lcd_puts("ABIERTO");//Imprime en la LCD el mensaje
							PORTF=0x04;      // Se activa el puerto del buzzer y LED en PORTF2
							_delay_ms(1000); //1000 ms de retraso
							PORTF=0x00;      //se desactiva el puerto del buzzer y del LED
							
							//PWM  Modulación por Ancho de Pulso es una forma muy eficaz de establecer cantidades intermedias
							// de potencia eléctrica entre la máxima potencia y apagado
							
							////declarar una frecuencia de 50Hz para el servo motor 
							TCCR1A|=(1<<COM1A1)|(1<<COM1B1)|(1<<WGM11);        //NON Inverted PWM
							TCCR1B|=(1<<WGM13)|(1<<WGM12)|(1<<CS11)|(1<<CS10); //PRESCALER=64 MODE 14(FAST PWM)
							
							// TOP, configurado para 50Hz (4.9ms)
							ICR1=4999;  //fPWM=50Hz (Period = 4.9ms Standard). //50Hz velocidad del servo

							DDRB=(1<<PB5);   //PWM Puerto de salida
							
							OCR1A=120;   //0 degree     Registro de comparación de salida1A (OCR1A) 
							Wait();
							ICR1=0;
							_delay_ms(5000);
							ICR1=4999;
							OCR1A=425;  //90 degree
							Wait();
							
							ICR1=0;
							
							
							i=0;
							lcd_clrscr();//Limpiar la pantalla del LCD
							lcd_puts("INGRESE CLAVE: ");//Imprime en la LCD el mensaje
						}
						else
						{
							i=0;
							lcd_clrscr();//Limpiar la pantalla del LCD
							lcd_puts("INCORRECTO");//Imprime en la LCD el mensaje
							PORTF=0x03;  // Se activa el puerto del buzzer y LED en PORTF0 PORTF0 Encendiendo el LED  rojo y sonido de alerta
							_delay_ms(2000);//2000 ms de retraso
							PORTF=0x00;     //se desactiva el puerto del buzzer y del LED
							lcd_clrscr();  //Limpiar la pantalla del LCD
							lcd_puts("INGRESE CLAVE: "); //Imprime en la LCD el mensaje
							
						}
					}
					
				}
				
			}
			
		}
		if (Bmaestra )
		{
			if (i==4)
			{    // se guarda en la eeprom la nueva contraseña
				eeprom_write_word(6, cont[0] ) ;
				eeprom_write_word(7, cont[1] ) ;
				eeprom_write_word(8, cont[2] ) ;
				eeprom_write_word(9, cont[3] ) ;
				// la va recuperar de la eeprom la contraseña recientemente cambiada
				claveNueva[0] = eeprom_read_word(6) ;
				claveNueva[1] = eeprom_read_word(7) ;
				claveNueva[2] = eeprom_read_word(8) ;
				claveNueva[3] = eeprom_read_word(9) ;
				
				lcd_clrscr();                //Limpiar la pantalla del LCD
				lcd_puts("SE HA CAMBIADO");  //Imprime en la LCD el mensaje
				PORTF=0x04;                  // Se activa el puerto del buzzer y LED en PORTF2
				_delay_ms(3000);             // 3000 ms de retraso
				PORTF=0x00;                  //se desactiva el puerto del buzzer y del LED
				
				
				
				lcd_clrscr();				//Limpiar la pantalla del LCD
				lcd_puts("INGRESE CLAVE: ");//Imprime en la LCD el mensaje
				_delay_ms(50);              //2000 ms de retraso
				i=0;
				x=0;
				Bmaestra =false;
			}
			
		}

	}
}
