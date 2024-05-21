#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#include "lcd.h"
#include "lcd.cpp"

#define F_CPU 8000000

#define true 1
#define false 0
#define dimX 84
#define  dimY 48

LCD lcd;

uint8_t CLOCK_STOP = 0;
uint8_t INNER_TIMER_GAME = 0;
uint8_t INNER_TIMER_SCORE = 0;
uint8_t CURRENT_MIN = 0;
uint8_t CURRENT_SEC = 0;

void wait(){
	volatile uint16_t i,j;
	for (i = 0; i< 1000; i++){
		for (j = 0; j<1000; j++);;
	}
}

void print_start(){
	lcd.clear();
	lcd.render();
	
	lcd.setCursor( 18,8);
	lcd.writeString("Press any",1); //letters : 9
	lcd.setCursor( 25, 23);
	lcd.writeString("button",1);
	lcd.setCursor( 12, 38);
	lcd.writeString("to start :D",1); //letters : 11
	lcd.render();
}
void print_over(){
	lcd.clear();
	lcd.render();
	
	lcd.setCursor( 20, 0);
	lcd.writeString("Game over ",1);
	lcd.setCursor( 10, 10);
	lcd.writeString("score   XXXX ",1);
	lcd.setCursor( 10, 18);
	lcd.writeString("time   XX:XX ",1);
	lcd.setCursor( 18, 30);
	lcd.writeString("Press any",1);
	lcd.setCursor( 0,40);
	lcd.writeString("button to exit",1);
	lcd.render();
	
}


void main_clock(){
	// MAIN TIMER, SET FOR t = 0.016 [s], f = 61 [Hz]
	sei();						// ENABLE INTERRUPTS
	TCCR0A = (1 << WGM01);		// SET CTC BIT - CLEAR ON COMPARE, TIMER CONTROL REGISTER A
	OCR0A = 255;				// VALUE TO COMPARE WITH
	TIMSK0 = (1 << OCIE0A);		// SET MASK TO ENABLE INTERRUPTS FOR OCIE0A
						
	TCCR0B = (1 << CS02) | (1 << CS00);
}

// FRAME, SCOREBOARD, TIME 
void toggle_scr(){				// TURN ON SCREEN 
	lcd.begin();
	lcd.setPower(1);
	lcd.writeString("STARTING!",1);
	lcd.render();
}

void toggle_frame(){			// PRINT SNAKE FRAME
	uint8_t i;
	
	//	<-----0 : dimX -1------->		
	//	_____________________  
	//	|					|  
	//	|					|
	//	|					|
	//	|					|   8 : dimY-1
	//	|					|
	//	|					|
	//  |___________________|

	for (i = 0; i<=dimY-8; i++){
		lcd.setPixel(0, i+8, 1);			// set top     Y line 
		lcd.setPixel(dimX-1, i+8, 1);		// set bottom  Y line 
	} 
	for (i = 0; i< dimX;i++){
		lcd.setPixel(i, 8 , 1);			// set left  X line
		lcd.setPixel(i, dimY-1, 1);		// set right X line
	}
	lcd.render();
}

void toggle_scoreboard(){		// PRINT SCOREBOARD 
	
	/*
	012345....																	    DX-1
	__________________________________________________________________________________
	|--T-- --\0--  --X-- --X-- --:-- --X-- --X-- --\0-- --S-- --0-- --0-- --0-- --0-- |
	|--T-- --\0--  --X-- --X-- --:-- --X-- --X-- --\0-- --S-- --0-- --0-- --0-- --0-- |
	|--T-- --\0--  --X-- --X-- --:-- --X-- --X-- --\0-- --S-- --0-- --0-- --0-- --0-- |
	|--T-- --\0--  --X-- --X-- --:-- --X-- --X-- --\0-- --S-- --0-- --0-- --0-- --0-- |
	|--T-- --\0--  --X-- --X-- --:-- --X-- --X-- --\0-- --S-- --0-- --0-- --0-- --0-- |
	|________________________________________________________________________________|
							One letter size : 5px X 5px 
	*/

	lcd.setCursor(0, 0);
	lcd.writeString("T XX:XX   XXXX", 1);
	lcd.render();
}

void clear_scoreborard(){
	unsigned int i,j;
	for (i = dimX - 25; i< dimX-1; i++){
		for (j = 0 ; j<8;j++){
			lcd.setPixel(i,j,0);
		}
	}
	lcd.render();
}

void change_scoreboard(uint16_t score){
	char str[5];
	snprintf(str, sizeof(str), "%u", score);
	
	clear_scoreborard();		
	
	lcd.setCursor(dimX - 25,0);
	
	 if(score <=9 ){
			lcd.writeString( "000", 1);
			lcd.writeString( str, 1);
	}if(score <=99 && score > 9){
			lcd.writeString( "00", 1);
			lcd.writeString( str, 1);
	}if(score <=999 && score > 99){
			lcd.writeString( "0", 1);
			lcd.writeString( str, 1);
	}if(score > 999){
			lcd.writeString( "", 1);
			lcd.writeString( str, 1);
	}
	lcd.render();

}

void clear_time(){
	unsigned int i,j;
	lcd.setCursor(10, 0);

	for (i = 10; i< 60; i++){
		for (j = 0 ; j<8 ;j++){
			lcd.setPixel(i,j,0);
		}
	}
	
	lcd.render();
}

void set_time(){
	
	lcd.setCursor(10, 0);
	lcd.writeString("00:00", 1);
	lcd.render();
	CURRENT_SEC = 0;
	CURRENT_MIN = 0;
}

void increment_time(){
	
	CURRENT_SEC ++;
	if ( CURRENT_SEC == 60 ) {CURRENT_MIN++; CURRENT_SEC= 0;}
	
	char str_min[3];
	snprintf(str_min, sizeof(str_min), "%u",CURRENT_MIN); // changes uint8_t to char array
	
	char str_sec[3];
	snprintf(str_sec, sizeof(str_sec), "%u",CURRENT_SEC); // changes uint8_t to char array
	
	clear_time();
	lcd.setCursor(10, 0);
	
	if (CURRENT_MIN <= 9) {
		lcd.writeString("0",1);
		lcd.writeString(str_min, 1);
	}else{
		lcd.writeString(str_min, 1);
	}
	lcd.writeString(":", 1);
	
	if (CURRENT_SEC <= 9) {
		lcd.writeString("0",1);
		lcd.writeString(str_sec, 1);
	}else{
		lcd.writeString(str_sec, 1);
	}

	lcd.render();
	
}

void enable_lcd(){
	
 	toggle_scr();
	lcd.clear();
	toggle_frame();
	toggle_scoreboard();
	clear_time();
	set_time();
	lcd.render();
	
}

ISR(TIMER0_COMPA_vect){
		
	
	
	// TO INCREMENT SCOREBOARD TIME - 30.30 INNER_TIMER NEEDED - EVERY 1 SECOND
	// MAIN GAME SPEED TIME - 	15 INNER_TIMER NEEDED - EVERY 0.5 SECOND
	//
	if (CLOCK_STOP != 1){
		INNER_TIMER_SCORE++;
		INNER_TIMER_GAME++;
	
		if(INNER_TIMER_GAME == 20){
			INNER_TIMER_GAME = 0;
			PORTB = ~PORTB;
	
		}
		if(INNER_TIMER_SCORE == 63){
			INNER_TIMER_SCORE = 0;
			increment_time();
			change_scoreboard(INNER_TIMER_GAME);
		}
	}
}


void stop_clock(bool var){ // if do == 1, CLOCK_STOP = 1, else CLOCK STOP = 0
	if (var == 1){
		CLOCK_STOP = 1;
	} 
	if (var == 0){
		CLOCK_STOP = 0;
	}
}

int main(void)
{
	
	
	DDRB = 0x77;
	PORTB = 0b111111;
	
	uint16_t score = 1519;
	uint16_t game_start = 0;
	uint16_t game_over = 0;
	toggle_scr();

	while(1) {
		print_start();
		wait(); // wait for button press, if so, change game_start = 1
		wait();
		wait();
		wait();
		wait();
		wait();
		
		game_start = 1;
		
		if( game_start != 0){
			lcd.clear();
			
			enable_lcd();
			main_clock();
			
			while(1) {	
				// main game loop
				CLOCK_STOP = 0;
			
				wait();
				wait();
				wait();
				wait();
				wait();			// if snake hit itself, game_over = 1
				game_over = 1;
				if (game_over == 1){
					lcd.clear();
					lcd.render();
					
					CLOCK_STOP = 1;
					
					print_over();	// print game over screen
					
					game_start = 0;
					game_over = 0;
					while(1){
						// wait for button press, to start over
						wait();
						wait();
						wait();
						wait();
						wait();		// wait for press of a button to exit to the main screen, if so, game_start = 1
						game_start = 1;
						
						if(game_start != 0) {break;};
					}
					break;
				}
			}
		}
		
	};
}



