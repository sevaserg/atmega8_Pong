#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define F_CPU 1000000

#define chosenport PORTD

#define PIN_RST	PD0
#define PIN_CE	PD1
#define PIN_DC	PD2
#define PIN_DIN	PD3
#define PIN_CLK	PD4
#define PIN_BL  PD5

const unsigned char digits [] = {
	0x81, 0x5E, 0x6E, 0x76, 0x81, 0xFF, 0x7B, 0x7D, 0x00, 0x7F, 0x7F, 0xFF, 0x3D, 0x5E, 0x6E, 0x76,
	0x79, 0xFF, 0xBD, 0x7E, 0x76, 0x76, 0x89, 0xFF, 0xC7, 0xDB, 0xDD, 0x00, 0xDF, 0xFF, 0xB0, 0x76,
	0x76, 0x76, 0x8E, 0xFF, 0x81, 0x76, 0x76, 0x76, 0x8D, 0xFF, 0xFE, 0xFE, 0x06, 0xFA, 0xFC, 0xFF,
	0x89, 0x76, 0x76, 0x76, 0x89, 0xFF, 0xB1, 0x6E, 0x6E, 0x6E, 0x81, 0xFF,
};

void _5110_send_byte(char data, char data_com)
{
	chosenport &= ~(1 << PIN_CE);
	if (data_com == 0)
	chosenport &= ~(1 << PIN_DC);
	else chosenport |= (1 << PIN_DC);
	for (char i = 0; i < 8; i++)
	{
		chosenport &= ~(1 << PIN_CLK);
		if ((0x80 >> i) & data)
		chosenport |= (1 << PIN_DIN);
		else chosenport &= ~(1 << PIN_DIN);
		chosenport |= (1 << PIN_CLK);
	}
	chosenport |= (1 << PIN_CE);
}

void _5110_reset()
{
	chosenport |= (1 << PIN_RST);
	chosenport |= (1 << PIN_CE);
	_delay_ms(1);
	chosenport &= ~(1 << PIN_RST);
	_delay_ms(1);
	chosenport |= (1 << PIN_RST);
}

void _5110_draw_byte(char data, char x, char y)
{
	_5110_send_byte((0x80 + x), 0);
	_5110_send_byte((0x40 + y), 0);
	_5110_send_byte(data, 1);
}

void _5110_init()
{
	chosenport |= (1 << PIN_BL);
	_5110_reset();
	_5110_send_byte(0x21, 0);
	_5110_send_byte(0x13, 0);
	_5110_send_byte(0x06, 0);
	_5110_send_byte(0xB8, 0);
	_5110_send_byte(0x20, 0);
	_5110_send_byte(0x08, 0);
	_5110_send_byte(0x0C, 0);
	for (int i = 0; i < 84; i++)
	for (int j = 0; j < 6; j++)
	_5110_draw_byte(0x00, i, j);
}

char score[] = {0, 0};
char playercoord[] = {28, 28};
char ball[] = {28, 42};
char velocity = 1;
char direction[] = {-1, 1};
char plrdir[] = {1,-1};
int steps = 0;

 void timer_init(void)
 {
	 TCNT0 = 200;
	 TCCR0 = 0x05;
	 TIMSK |= (1 << TOIE0);
 }

void redraw_rocket(char player)
{
			for (int j = 1; j < 6; j++)
			{
				if (playercoord[player] / 8 == j)
				{
					char position[] = {0xFF, 0xFF};
					for (int i = 0; i < playercoord[player] % 8 + 1; i++)
					position[0] = (position[0] << 1) & ~(0x01);
					for (int i = 0; i < 8 - playercoord[player] % 8; i++)
					position[1] = (position[1] >> 1) & ~(0x80);
					_5110_draw_byte(position[0],player*82,playercoord[player] / 8);
					_5110_draw_byte(position[1],player*82,(playercoord[player] / 8) + 1);
					_5110_draw_byte(position[0],player*82+1,playercoord[player] / 8);
					_5110_draw_byte(position[1],player*82+1,(playercoord[player] / 8) + 1);
				}
				else if (playercoord[player] / 8 + 1 != j)
				{
					_5110_draw_byte(0x00, player*82,j);
					_5110_draw_byte(0x00, player*82+1,j);
				}
			}
}

void redraw_ball()
{
	for (int j = 1; j < 6; j++)
	{
		if (ball[1] / 8 == j)
		{
			char position[] = {0x07, 0x07};
			for (int i = 0; i < ball[1] % 8 + 1; i++)
				position[0] = (position[0] << 1) & ~(0x01);
			for (int i = 0; i < 8 - ball[1] % 8; i++)
				position[1] = (position[1] >> 1) & ~(0x80);
			for (int i = 0; i < 3; i++)
			{
				_5110_draw_byte(position[0],ball[0] + i,ball[1] / 8);
				_5110_draw_byte(position[1],ball[0] + i,(ball[1] / 8) + 1);
				//_5110_draw_byte(position[1],ball[0] + i,(ball[1] / 8) + 2);
			}
			if (ball[0] > 2)
			{
				_5110_draw_byte(0x00,ball[0] - 1,(ball[1] / 8) + 1);
				_5110_draw_byte(0x00,ball[0] - 1,(ball[1] / 8));
				if (ball[1] > 15) _5110_draw_byte(0x00,ball[0] - 1,(ball[1] / 8) - 1);
			}
			if (ball[0] < 79)
			{
				_5110_draw_byte(0x00, ball[0] + 3, (ball[1] / 8) + 1);
				_5110_draw_byte(0x00, ball[0] + 3, (ball[1] / 8));
				if (ball[1] > 15) _5110_draw_byte(0x00,ball[0] + 3,(ball[1] / 8) - 1);
			}
		}
	}
}

void redraw_field()
{
	redraw_ball();
	redraw_rocket(0);
	redraw_rocket(1);
}

void redraw_all()
{
	for (int i = 0; i < 5; i++)
	{
		_5110_draw_byte(digits[i+6*score[0]],i+1,0);
		_5110_draw_byte(digits[i+6*score[1]],i+78,0);
	}
	redraw_field();
}

void reset()
{
	playercoord[0] = 28;
	playercoord[1] = 28;
	ball[0] = 28;
	ball[1] = 42;
	velocity = 1;
	for (int j = 1; j < 6; j++)
	{
		for (int i = 0; i < 83; i++)
			_5110_draw_byte(0x00, i, j);
	}
	redraw_all();
}

void goal (char player)
{
	score[player]++;
	if (score[player] == 10)
	{
		score[0] = 0;
		score[1] = 0;
	}
	reset();
}

void PWM_PB1(char percent)
{
	for (int j = 0; j < 10; j++)
	{
		PORTB |= 1<<PB1;
		for (int i = 0; i < percent; i++)
			_delay_us(10);
		PORTB &= ~(1<<PB1);
		for (int i = 0; i < 100-percent; i++)
			_delay_us(10);
	}
}

void moveball()
{
	ball[0] += direction[0];
	ball[1] += direction[1];
	if (ball[0] == 2 || ball[0] == 80) direction[0] = -direction[0];
	if (ball[1] == 8 || ball[1] == 45)
	{
		direction[1] = -direction[1];
		if (velocity < 80) velocity+=10;
		PORTB = 0x02;
		_delay_ms(10);
		PORTB = 0x00;
	}
	if (ball[0] == 2 && (ball[1] < playercoord[0] - 1 || ball[1] > 7 + playercoord[0])) goal(1);
	else if (ball[0] == 79 && (ball[1] < playercoord[1] - 1|| ball[1] > 7 + playercoord[1])) goal(0);
	else if (ball[0] == 80 || ball[0] == 2)
	{
		if (ball[0] == 80)
			PWM_PB1(10);
		else PWM_PB1(50);
	}
}

void playermove()
{
	if (playercoord[0] + 1 <= 39 && !(PINC & 1)) playercoord[0]++;
	if (playercoord[0] - 1 >= 7 && !(PINC & 2)) playercoord[0]--;
}

void aimove()
{
	if (ball[0] > 30 && direction[0] == 1)
	{
		if (ball[1] > playercoord[1] + 4 && playercoord[1] + 1 <= 39) playercoord[1]++;
		if (ball[1] < playercoord[1] + 4 && playercoord[1] - 1 >= 7) playercoord[1]--;
	}
}

ISR(TIMER0_OVF_vect)
{
	
	playermove();
	aimove();
	TCNT0 = 200;
}

int main(void)
{
	DDRC = 0x00;
	DDRB = 0xFF;
	PORTC = 0xFF;
	PORTB = 0x00;
	DDRD = 0xFF;
	int k = 0, step = 0;
	timer_init();
	_5110_init();
	for(int i = 0; i < 84; i++)
		_5110_draw_byte(0xFF,i,0);
	sei();
    while(1)
    {
		for(int i = 0; i < 70-velocity; i++) _delay_ms(1);
		moveball();
		redraw_all();
    }
}