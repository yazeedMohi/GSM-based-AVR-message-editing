# GSM-based-AVR-message-editing

The project is simple message editor using: an AVR ATMega32, a LCD, and a 4x4 keypad.
The buttons on the keypad are treated like buttons of an old phone where each button can input several different characters.
If one if the number buttons is pressed more than once in a row, the character entered by the button is looped among a set of 
 predefined values.
The editor also has the ability of scrolling up and down using buttons in the keypad, after which the user can return to the 
 last cursor location simply by pressing any other key.
The user also has the ability of clearing the whole text or backspacing (deleting the last entered character).
The text entered by the user is stored in memory in an array named storage[][], so that the user can send the values of the 
 array as a text message (but the code and the circuit needs to be modified in order for the user to be actually able to send an SMS).
