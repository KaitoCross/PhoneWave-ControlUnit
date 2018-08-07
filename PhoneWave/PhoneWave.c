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
uint8_t displayedDigit[4] = {0,0,0,0};
uint16_t TimeInSecs = 0;
uint8_t displaycounter = 0;
bool triggermove = false;
bool reset = false;
bool countdown = false;
bool noinput = false;
uint8_t timercounter = 0;
int main(void)
{
	//which digit lights up which segments of the display
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
		if (triggermove) //move digits from right to left if new input came
		{
				displayedDigit[0]=displayedDigit[1];
				displayedDigit[1]=displayedDigit[2];
				displayedDigit[2]=displayedDigit[3];
				displayedDigit[3]=temp;
				triggermove=false;
		}
		if (countdown)
		{
			
			displayedDigit[0] = TimeInSecs/1000;
			displayedDigit[1] = TimeInSecs/100-(displayedDigit[0]*10);
			displayedDigit[2] = TimeInSecs/10-((displayedDigit[1]*10)+(displayedDigit[0]*100));
			displayedDigit[3] = TimeInSecs-((displayedDigit[2]*10)+(displayedDigit[1]*100)+(displayedDigit[0]*1000));
			countdown=false;
		}
		if (reset)
		{
			
			displayedDigit[0]=0;
			displayedDigit[1]=0;
			displayedDigit[2]=0;
			displayedDigit[3]=0;
			PORTC = 0b00001111;
			reset = false;
		}
		if (temp<11) //output calculated digits. destination is multiplexed 7-segments LED Display
		{
		PORTD=(1<<PD2)|(1<<PD3)|(0<<PD4)|(0<<PD5)|(0<<PD6)|(segcodeD[displayedDigit[0]]<<PD7);
		PORTB=segcodeB[displayedDigit[0]];
		_delay_ms(2);
		PORTD=(1<<PD2)|(0<<PD3)|(1<<PD4)|(0<<PD5)|(0<<PD6)|(segcodeD[displayedDigit[1]]<<PD7);
		PORTB=segcodeB[displayedDigit[1]];
		_delay_ms(2);
		PORTD=(1<<PD2)|(0<<PD3)|(0<<PD4)|(1<<PD5)|(0<<PD6)|(segcodeD[displayedDigit[2]]<<PD7);
		PORTB=segcodeB[displayedDigit[2]];
		_delay_ms(2);
		PORTD=(1<<PD2)|(0<<PD3)|(0<<PD4)|(0<<PD5)|(1<<PD6)|(segcodeD[displayedDigit[3]]<<PD7);
		PORTB=segcodeB[displayedDigit[3]];
		_delay_ms(2);
		}
    }

}
ISR(INT0_vect) //Interrupt triggered by high sighnal on Port that has INT0,
// use to read the next new number given by the MT8870
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
					if (displayedDigit[n]==10)
					{
						displayedDigit[n] = 0;
					}
				}
		TimeInSecs = displayedDigit[0]*1000+displayedDigit[1]*100+displayedDigit[2]*10+displayedDigit[3];
		if (TimeInSecs>0)
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
	timercounter++;
	if (timercounter == 40) //when 1 sec passed
	{
		TimeInSecs = TimeInSecs-1;
		if (TimeInSecs<=0)
		{
			TIMSK=(0<<TOIE0);
			noinput=false;
			PORTC = 0b00001111;
		}
		countdown = true;
		timercounter = 0;
	}
	TCNT0 = 256-90;
}