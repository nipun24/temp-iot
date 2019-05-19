#include<avr/io.h>
#define BAUD_RATE 9600UL
#define F_CPU 16000000UL
#define ubrr_value ((F_CPU/(16UL*BAUD_RATE))-1)
#include<util/delay.h>
#include<mamalib/ADC.h>

void UART_init()
{
	UBRRL=ubrr_value;
	UBRRH=(ubrr_value>>8);
	UCSRB=(1<<RXEN)|(1<<TXEN);
	UCSRC=(1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0); //UMSEL=0 IS BY DEFAULT ..NO NEED TO DECLARE THAT
}

void UART_send(unsigned char data)
{
  while(!(UCSRA&(1<<UDRE)));
  UDR=data;
}

unsigned char UART_rec()
{
  while(!(UCSRA&(1<<RXC)));
  return UDR;
}

void GSM_string(unsigned char *str)
{ unsigned char i=0;
  while(str[i]!='\0')
  {
    UART_send(str[i]);
	i++;
  }
}

void UART_number1(unsigned long long int p)
{
    if(p>9) 
	{
	UART_number1(p/10);
	}
	UART_send(p%10+48);
}

void sms(int i)
{
	GSM_string("AT+CMGF=1\r\n");
	_delay_ms(1000);
	GSM_string("AT+CMGS=\"8802767106\"\r\n");
	_delay_ms(1000);
	GSM_string(" ALERT!!!...");
	GSM_string("\r\n");
	GSM_string("Current Temperature at your home is: ");
	UART_number1(i);
	GSM_string("*Celcius");
	_delay_ms(300);
	UART_send(0x1A);
	_delay_ms(300);
}

void delay(unsigned char *time)
{ 
  unsigned char *p;
  unsigned char rec[10]={'\0'};
  while(1)
  {
	  for(int i=0;rec[i]!='\r';i++)
		  { 
		    rec[i]=UART_rec();
				if(rec[i]=='\r')
					break;
		  }
		p=strstr(rec,time);
		if(p)
		{
		  PORTB=0x03;
		  break;
		}
		else
		{
		  PORTB=0x01;
		}
	}
}

void main()
{   	
	DDRA=0xf0;
	DDRB=0xff;
	UART_init();
	ADC_init();
	unsigned int x=0;
	while(1)
  {
	  PORTA=0X00;
		x=ADC_value(1);
		x=x*4;
		x=x/10;
		GSM_string("AT\r\n");
	  unsigned char str="OK";
		delay(str);
		GSM_string("AT+CIPSEND\r\n");
		_delay_ms(5000); //wait for 5sec
		GSM_string("GET https://api.thingspeak.com/update?api_key=CIWJBG1GB3O9Q53C&field1=0");
		UART_number1(x);
		GSM_string("\r\n");
		_delay_ms(300);
		UART_send(0x1A);
		_delay_ms(5000);

		GSM_string("AT+CIPSHUT\r\n");
		_delay_ms(5000);
		_delay_ms(5000);
		while(x>40)
		{
			PORTA=0x80;
			_delay_ms(500);
			sms(x);
			GSM_string("AT+CIPMUX=0\r\n");
			_delay_ms(5000);
			GSM_string("AT+CIPCSGP=1,\"www\"\r\n");
			_delay_ms(5000);
			GSM_string("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80\r\n");
		  _delay_ms(5000);
			GSM_string("AT+CIPSEND\r\n");
			_delay_ms(5000);//wait for 5sec
		  GSM_string("GET https://api.thingspeak.com/update?api_key=CIWJBG1GB3O9Q53C&field1=0");
			UART_number1(x);
		  GSM_string("\r\n");
		  _delay_ms(300);
		  UART_send(0x1A);
		  _delay_ms(5000);
		  GSM_string("AT+CIPSHUT\r\n");
		  _delay_ms(5000);
		  _delay_ms(5000);
			x=ADC_value(1);
			x=x*4;
			x=x/10;
		}
	}
}
