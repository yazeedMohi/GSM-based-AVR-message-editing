//Name: Yazeed Mohi-Eldeen
//Index: 145096

/*
Project: GSM based AVR message editing
The project is simple message editor using: an AVR ATMega32, a LCD, and a 4*4 keypad.
The buttons on the keypad are treated like buttons of an old phone where each button can input several different characters.
If one if the number buttons is pressed more than once in a row, the character entered by the button is looped among a set of 
 predefined values.
The editor also has the ability of scrolling up and down using buttons in the keypad, after which the user can return to the 
 last cursor location simply by pressing any other key.
The user also has the ability of clearing the whole text or backspacing (deleting the last entered character).
The text entered by the user is stored in memory in an array named storage[][], so that the user can send the values of the 
 array as a text message (but the code and the circuit needs to be modified in order for the user to be actually able to send an SMS).
*/

//including the required header files
#include <asf.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

//The clock speed is 8MHz in this case, else it can be edited from here.
#define F_CPU 8000000UL

//Defining which MC I/O pins are connected to which LCD pins
#define LCD_PRT PORTA
#define LCD_DDR DDRA
#define LCD_PIN PINA
#define LCD_RS 0
#define LCD_RW 1
#define LCD_EN 2

///Defining which MC I/O pins are connected to which Keyboard pins
#define KEY_PRT PORTC
#define KEY_DDR DDRC
#define KEY_PIN PINC

//Defining the characteristics of the LCD
#define Max_Message_Size 70
#define Row_Size 16
#define Number_Of_Rows 2
#define Max_Number_Of_Rows (64/16+1)

//Global variables
unsigned char colloc, rowloc;	//the row/column location of the pressed key in the keypad.
unsigned char B1 = 0;			//timer finished or not.
unsigned char B2 = 0;
unsigned char j = 1;			//current pointer position (1->70).
unsigned char line = 1;			//current line of the text (1->5).


//Shaped attached to each button in the actual keypad.
unsigned char keypad[4][4] = { {'N', '0', '=', '+'},
							   {'1', '2', '3', '-'},
							   {'4', '5', '6', 'X'},
							   {'7', '8', '9', '/'}};
								   
//The values stored for each of the keypad keys (the looping values), where / = no value.
unsigned char allKeys[10][9] ={			  {'0',' ','(',')','-','+','=','[',']'},
	{'1','.',',','?','!','@','$','%','&'},{'2','a','b','c','A','B','C','/','/'},{'3','d','e','f','D','E','F','/','/'},
	{'4','g','h','i','G','H','I','/','/'},{'5','j','k','l','J','K','L','/','/'},{'6','m','n','o','M','N','O','/','/'},
	{'7','p','q','r','s','P','Q','R','S'},{'8','t','u','v','T','U','V','/','/'},{'9','w','x','y','z','W','X','Y','Z'}};
		
		
//Allocating memory space for storing the text entered								
unsigned char storage[Row_Size][Max_Number_Of_Rows];

//A function for a delay in the range of microseconds
void delay_us(int d)
{
	for(int i=1; i<=d; i++){
		_delay_us(1);					//written like this because _delay_us() accepts only constant values.
	}
	
}

//A function for a delay in the range of nanoseconds
void delay_ms(int d)
{
	for(int i=1; i<=d; i++){
		_delay_ms(1);
	}
}

//A function for sending a command to the LCD. (written according to LCD timing diagrams)
void lcdCommand( unsigned char cmnd)
{

	LCD_PRT = (LCD_PRT & 0x0F) | (cmnd & 0xF0);
	LCD_PRT &= ~ (1<<LCD_RS);
	LCD_PRT &= ~ (1<<LCD_RW);
	LCD_PRT |= (1<<LCD_EN);
	
	delay_us(1);
	
	LCD_PRT &= ~ (1<<LCD_EN);
	
	delay_us(20);
	
	LCD_PRT = (LCD_PRT & 0x0F) | (cmnd << 4);
	LCD_PRT |= (1<<LCD_EN);
	
	delay_us(1);
	
	LCD_PRT &= ~ (1<<LCD_EN);
	
	delay_us(100);

}

//A function for sending data to the LCD
void lcdData( unsigned char data)
{
	LCD_PRT = (LCD_PRT & 0x0F) | (data & 0xF0);
	LCD_PRT |= (1<<LCD_RS);
	LCD_PRT &= ~ (1<<LCD_RW);
	LCD_PRT |= (1<<LCD_EN);
	
	delay_us(1);
	
	LCD_PRT &= ~ (1<<LCD_EN);
	
	delay_us(20);
	
	LCD_PRT = (LCD_PRT & 0x0F) | (data << 4);
	LCD_PRT |= (1<<LCD_EN);
	
	delay_us(1);
	
	LCD_PRT &= ~ (1<<LCD_EN);
}

//A function for initializing the LCD before using it.
void lcd_init()
{
	LCD_DDR = 0xFF;
	LCD_PRT &=~(1<<LCD_EN);
	
	delay_us(2000);
	
	lcdCommand(0x33);	//<
	lcdCommand(0x32);	//for 4-bit LCD mode (saving up the MC I/O pins).
	lcdCommand(0x28);	//>
	lcdCommand(0x0E);	//display ON, cursor ON
	lcdCommand(0x01);	//clear LCD
	
	delay_us(2000);
	
	lcdCommand(0x06);	//shift cursor right (to make it start at the beginning of the first row).
}

//A function for going to a certain location within the LCD display.
void lcd_gotoxy (unsigned char x, unsigned char y)
{
	unsigned char firstCharAdr[] = {0x80, 0xC0, 0x94, 0xD4};	//must be according to the LCD type.
		
	lcdCommand(firstCharAdr[y-1] + x - 1);
}

//A function for initializing the keypad. (setting its port as half input half output).
void keypad_init()
{
	KEY_DDR = 0xF0;
	KEY_PRT = 0xFF;
}	

//A function that waits for the user to press a button in the keypad and returns the value of the corresponding character.
unsigned char getKey(void)
{
	while(1)
	{
		do 
		{
			KEY_PRT &= 0x0F;
			colloc = (KEY_PIN & 0x0F);
		} while (colloc != 0x0F);
		do
		{
			do{
				delay_ms(20);
				
				colloc = (KEY_PIN & 0x0F);
			}while(colloc == 0x0F);

			delay_ms(20);
			
			colloc = (KEY_PIN & 0x0F);
		}while (colloc == 0x0F);
		while(1)
		{
			KEY_PRT = 0xEF;
			colloc = (KEY_PIN & 0x0F);
			if(colloc != 0x0F)
			{
				rowloc = 0;
				break;
			}

			KEY_PRT = 0xDF;
			colloc = (KEY_PIN & 0x0F);
			
			if(colloc != 0x0F)
			{
				rowloc = 1;
				break;
			}

			KEY_PRT = 0xBF;
			colloc = (KEY_PIN & 0x0F);

			if(colloc != 0x0F)
			{
				rowloc = 2;
				break;
			}

			KEY_PRT = 0x7F;
			colloc = (KEY_PIN & 0x0F);
			rowloc = 3;
			break;

		}
		if(colloc == 0x0E)
			return (keypad[rowloc][0]);
		else if(colloc == 0x0D)
			return (keypad[rowloc][1]);
		else if(colloc == 0x0B)
			return (keypad[rowloc][2]);
		else
			return (keypad[rowloc][3]);
	}
		
}

//A function for restarting the timer (Timer1).
void restartTimer (void)
{
	B1 = 0;

	TCNT1 = 0xD17B;			//about 300ms.
	
	TCCR1A = 0x00;
	TCCR1B = 0x03;			//normal mode, pre-scaler = 64

}

/*
A function for storing a character in the allocated storage area (storage[][]), the character value is stored in 
a location in the array that is the same as its location in the text (by using the value of j).
*/
void saveChar(unsigned char c)
{
		 if(j>64) storage[j-64-1][5]=c;
	else if(j>48) storage[j-48-1][4]=c;
	else if(j>32) storage[j-32-1][3]=c;
	else if(j>16) storage[j-16-1][2]=c;
	else		  storage[j-1][1]   =c;
}

/*
A function for scrolling down while the user is entering the text, it puts the current row in the upper row of 
the display and puts the cursor at the start of the second row on the display.
*/
//This function cannot be used for an LCD other than the 2*16 type.
void scrollDown(unsigned char line2)
{
	unsigned char i;
	for(i=0;i<=15;i++){
		lcd_gotoxy(i+1,1);
		lcdData(storage[i][line2-1]);
		lcd_gotoxy(i+1,2);
		lcdData(0);
	} lcd_gotoxy(1,2);
}

/*
A function for scrolling down after pressing the scroll down button, its difference from the above is that it
puts the current row in the bottom row of the display, and the row before it on the upper row of the display.
*/
void scrollDown2(unsigned char line2)
{
	unsigned char i;
	for(i=0;i<=15;i++){
		lcd_gotoxy(i+1,1);
		lcdData(storage[i][line2-1]);
		lcd_gotoxy(i+1,2);
		lcdData(storage[i][line2]);
	} lcd_gotoxy(16,2);
}
/*
A function for scrolling up the display, can be used either if the user pressed the scroll up button or if 
he/she pressed the backspace while in the beginning of a row, it puts the current row in the bottom row of 
the display and the previous one on the top row of the display.
*/
 //This function cannot be used for an LCD other than the 2*16 type.
void scrollUp(unsigned char line2)
{
	unsigned char i;
	for(i=0;i<=15;i++){
		lcd_gotoxy(i+1,1);
		lcdData(storage[i][line2-2]);
		lcd_gotoxy(i+1,2);
		lcdData(storage[i][line2-1]);
	} lcd_gotoxy(16,2);
}

//A function used for moving the cursor according to the value of j, and scrolls down if needed.
void moveCursor(void)
{
	
	if((j==33 || j==49 || j==65) && (B2==0)) {line++;scrollDown(line);}
	else if(j>70) {lcd_gotoxy(70-64,2);line=5;}
	else if(j>64 && j<=70) {lcd_gotoxy(j-64,2);line=5;}
	else if(j>48) {lcd_gotoxy(j-48,2);line=4;}
	else if(j>32) {lcd_gotoxy(j-32,2);line=3;}
	else if(j>16) {lcd_gotoxy(j-16,2);line=2;}
	else {lcd_gotoxy(j,1);line=1;}
}

//A function for performing the backspace functionality, it deletes the most recently entered character and might scroll up if needed.
void backSpace(void)
{
		B2 = 1;
		
		saveChar(0);
		if(j==33 || j==49 || j==65) {scrollUp(line);}
		if(j>1){j--;}
		moveCursor();
		lcdData(0);
		moveCursor();
		
		B2 = 0;	
}

void userScrollDown(unsigned char l);

//A function for handling the press of the scroll button by the user, it determines whether or not the user is allowed to scroll up (are there any lines above?), and it returns to the current cursor position in the case the user pressed any key other than the scroll up/down keys.
void userScrollUp(unsigned char l)
{
	if(l>2){
		unsigned char ch3;
		scrollUp(l);
		ch3 = getKey();
		if(ch3 == '-' && l>2) userScrollUp(l-1);
		else if(ch3 == '+' && l>2) userScrollDown(l);
	} if(line>2){scrollUp(line+1);moveCursor();}
}

//A function for handling the press of the scroll down by the user, it determines whether or not the user is allowed to scroll down (are there any lines below?), and it returns to the current cursor position in the case the user pressed any key other than the scroll up/down keys.
void userScrollDown(unsigned char l)
{
	if(l>2 && l<5 && l<line){
		unsigned char ch3;
		scrollDown2(l);
		ch3 = getKey();
		if(ch3 == '+' && l>2 && l<5) userScrollDown(l+1);
		else if(ch3 == '-' && l>2 && l<5) userScrollUp(l-1);
	} if(line>2){scrollUp(line+1);moveCursor();}
}

//A function for clearing the LCD display and restarting the cursor and the line pointer positions.
void clearScreen(void){
	
	lcdCommand(0x1);
	delay_us(2000);
	
	for(unsigned char i; i<=16; i++){
		for(unsigned char k; k<=5; k++){
			storage[i][k]=0;
		}
	}
	
	j=1;
	line=1;
	moveCursor();
}

//A function for handling the pressing of one of the multivalued keys, it determines what value will be the final result of the several presses.
void loopKey(unsigned char ch)
{
	unsigned char k = 1;
	unsigned char x = ch - '0';
	moveCursor();
	lcdData(allKeys[x][1]);
	saveChar(allKeys[x][1]);
	unsigned char ch2;
	while(1){
		restartTimer();
		ch2 = getKey();
		if(	B1 ==0 && ch2 == ch){
			k++;
			if(k>=9) k=0;
			else if(allKeys[x][k] == '/')k=0;
			moveCursor();
			lcdData(allKeys[x][k]);
			saveChar(allKeys[x][k]);
		}
		else if( B1 == 1 && ch2 == ch){
			j++;
			loopKey(ch2);break;
			}
		else 
			{
				j++;
				if(ch2 >= '0' && ch2 <= '9') {
					loopKey(ch2);
				}
				else if(ch2 == 'N'){
					moveCursor();
					lcdData('*');
					saveChar('*');
					j++;
				}
				else if(ch2 == '='){
					moveCursor();
					lcdData('#');
					saveChar('#');
					j++;
				}
				else if(ch2 == '/'){
					backSpace();
				}
				else if(ch2 == '+'){
					userScrollDown(line);
					break;
				}
				else if(ch2 == '-'){
					userScrollUp(line);
					break;
				}
				else if(ch2 == 'X'){
					clearScreen();
					break;
				}
			break;
		}
	}
}

//The main function.
int main (void)
{
	board_init();
	
	lcd_init();
	
	keypad_init();
	
	moveCursor();
	
	unsigned char c;
	unsigned char c2;
	unsigned char x = 0;

	TIMSK = (1<<TOV1);
	sei();
	
	while(1)
	{
		c = getKey();
		
		if(c >= '0' && c <= '9') {
			loopKey(c);
			continue;
		}
		else if(c=='/'){
			backSpace();
			continue;
		}
		else if(c == 'X'){
			clearScreen();
			continue;
		}
		else if(c == 'N'){
			c2 = '*';
		}
		else if(c == '='){
			c2 = '#';
		}
		else if(c == '+'){
			userScrollDown(line);
			continue;
		}
		else if(c == '-'){
			userScrollUp(line);
			continue;
		}
		else {
			c2 = '&';
		}
		moveCursor();
		lcdData(c2);
		saveChar(c2);
		j++;
	}
	return 0;
}

/*
//The interrupt handling routine for if the timer finishes the given period (specified by the TCNT0 registers and is 
//executed when they overflow to zero), it stops the timer and tells the program that it has finished by setting the value of the global variable B1.
*/
ISR (TIMER1_OVF_vect)
{
	TCCR1A = 0x00;
	TCCR1B = 0x00;
	
	B1=1;
}