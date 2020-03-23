# GSM-based-AVR-message-editing

The project is simple message editor using: an AVR ATMega32, a LCD, and a 4x4 keypad.
- The buttons on the keypad are treated like buttons of an old phone where each button can input several different characters.
- If one of the number buttons is pressed more than once in a row, the character entered by the button is looped among a set of 
 predefined values.
- The editor also has the ability of scrolling up and down using buttons in the keypad, after which the user can return to the 
 last cursor location simply by pressing any other key.
- The user also has the ability of clearing the whole text or backspacing (deleting the last entered character).
- The text entered by the user is stored in memory in an array named storage[][], so that the user can send the values of the 
 array as a text message (but the code and the circuit needs to be modified in order for the user to be actually able to send an SMS).
 
 ## Requirements:
- Atmel Studio 7
- Protous 8

## Usage:
- Open the project in Atmel studio, the code is in main.c.
- Edit the code if needed, then build the project to a .hex file (or you can use the one provided)
- Open up the Protous file: "AVR Message editor.pdsprj"
- Double click on the AVR Microcontroller.
- Click on the folder icon next to "Program file".
- Select the .hex file from the atmel project (e.g. Atmel Studio\7.0\GccBoardProject1\GccBoardProject1\Debug\GccBoardProject1.hex)
- Run the simulation in Protous and press the different keys on the keyboard, see the results on the LCD.
- The same file can be written into an actual AVR chip, see online tutorials on how to do that.

Please don't hesitate to contact me if you have any questions/ comments regarding the project.
