/*
 * PhoneWave.c
 *
 * Controls Display, Light and Plate
 *
 * Created: 07.05.2014 18:33:50
 *  Author: Christian
 */ 


#include <avr/io.h>
#define  F_CPU 3686400UL
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>

uint8_t segcodeB[11];
bool segcodeD[11];
uint8_t temp = 0;
uint8_t darstellzahl[4] = {0,0,0,0};
uint16_t Zeitwert = 0;
uint8_t displaycounter = 0;
bool triggermove = false;
bool reset = false;
bool countdown = false;
bool noinput = false;
uint8_t tzaehler = 0;
int main(void)
{
	//welche Zahl welche leuchtenden Segmente
	//			= 0b00gfedcb
	segcodeB[0] = 0b00011111;
	segcodeB[1] = 0b00000011;
	segcodeB[2] = 0b00101101;
	segcodeB[3] = 0b00100111;
	segcodeB[4] = 0b00110011;
	segcodeB[5] = 0b00110110;
	segcodeB[6] = 0b00111110;
	segcodeB[7] = 0b00000011;
	segcodeB[8] = 0b00111111;
	segcodeB[9] = 0b00110111;
	segcodeB[10] = 0b00011111;
			//  = a
	segcodeD[0] = 1;
	segcodeD[1] = 0;
	segcodeD[2] = 1;
	segcodeD[3] = 1;
	segcodeD[4] = 0;
	segcodeD[5] = 1;
	segcodeD[6] = 1;
	segcodeD[7] = 1;
	segcodeD[8] = 1;
	segcodeD[9] = 1;
	segcodeD[10] = 1;
	
	DDRD =(1<<PD3)|(1<<PD4)|(1<<PD5)|(1<<PD6)|(1<<PD7);
	DDRB =(1<<PB0)|(1<<PB1)|(1<<PB2)|(1<<PB3)|(1<<PB4)|(1<<PB5);
	DDRC =(1<<PC4);
	TCCR0 = 0b00000101; //Teiler 1024
	TCNT0 = 256-90; //Interrupt jede 25ms;
	PORTD|=(1<<PD2);
	PORTC |=(1<<PC0)|(1<<PC1)|(1<<PC2)|(1<<PC3);
	MCUCR=0b00000011;
	GICR=0b01000000;
	sei();

    while(1)
    {
		if (triggermove) //move numbers from right to left if new input came
		{
				darstellzahl[0]=darstellzahl[1];
				darstellzahl[1]=darstellzahl[2];
				darstellzahl[2]=darstellzahl[3];
				darstellzahl[3]=temp;
				triggermove=false;
		}
		if (countdown)
		{
			
			darstellzahl[0] = Zeitwert/1000;
			darstellzahl[1] = Zeitwert/100-(darstellzahl[0]*10);
			darstellzahl[2] = Zeitwert/10-((darstellzahl[1]*10)+(darstellzahl[0]*100));
			darstellzahl[3] = Zeitwert-((darstellzahl[2]*10)+(darstellzahl[1]*100)+(darstellzahl[0]*1000));
			countdown=false;
		}
		if (reset)
		{
			
			darstellzahl[0]=0;
			darstellzahl[1]=0;
			darstellzahl[2]=0;
			darstellzahl[3]=0;
			PORTC = 0b00001111;
			reset = false;
		}
		if (temp<11) //output calculated numbers. destination is multiplexed 7-segments LED Display
		{
		PORTD=(1<<PD2)|(1<<PD3)|(0<<PD4)|(0<<PD5)|(0<<PD6)|(segcodeD[darstellzahl[0]]<<PD7);
		PORTB=segcodeB[darstellzahl[0]];
		_delay_ms(2);
		PORTD=(1<<PD2)|(0<<PD3)|(1<<PD4)|(0<<PD5)|(0<<PD6)|(segcodeD[darstellzahl[1]]<<PD7);
		PORTB=segcodeB[darstellzahl[1]];
		_delay_ms(2);
		PORTD=(1<<PD2)|(0<<PD3)|(0<<PD4)|(1<<PD5)|(0<<PD6)|(segcodeD[darstellzahl[2]]<<PD7);
		PORTB=segcodeB[darstellzahl[2]];
		_delay_ms(2);
		PORTD=(1<<PD2)|(0<<PD3)|(0<<PD4)|(0<<PD5)|(1<<PD6)|(segcodeD[darstellzahl[3]]<<PD7);
		PORTB=segcodeB[darstellzahl[3]];
		_delay_ms(2);
		}
    }

}
ISR(INT0_vect)
{
	temp = PINC & 0x0F;
	if (noinput==false)
	{
	if (temp<11)
	{
	triggermove = true;
	}
	else
	{
		triggermove=false;
	}
	}
	if (temp>10)//when input is not a number
	{
		if (temp==11) //Input = *
		{
			reset = true;
			TIMSK=(0<<TOIE0);
			TCNT0 = 256-90;
			noinput = false;
		}
		if (temp == 12)// when # registered
		{		
			noinput=true;
				for (int n = 0; n < 4; n++)
				{
					//Because DTMF 0 is outputted as 10 by MT8870, correct array
					if (darstellzahl[n]==10)
					{
						darstellzahl[n] = 0;
					}
				}
		Zeitwert = darstellzahl[0]*1000+darstellzahl[1]*100+darstellzahl[2]*10+darstellzahl[3];	
		if (Zeitwert>0)
		{
			TIMSK|=(1<<TOIE0);
			PORTC |=(1<<PC4);
		}
		else
		{
			noinput=false;
		}
		}
		temp=10;
		triggermove=false;
	}
//	_delay_ms(10);
}
ISR(TIMER0_OVF_vect)
{
	tzaehler++;
	if (tzaehler == 40) //when 1 sec passed
	{
		Zeitwert = Zeitwert-1;
		if (Zeitwert<=0)
		{
			TIMSK=(0<<TOIE0);
			noinput=false;
			PORTC = 0b00001111;
		}
		countdown = true;
		tzaehler = 0;
	}
	TCNT0 = 256-90;
}