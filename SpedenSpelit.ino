#include "display.h"
#include "buttons.h"
#include "leds.h"
#include "SpedenSpelit.h"
#include "lcd_logic.h"

#include <avr/interrupt.h>


volatile State state = IDLE;

// Button that was pressed
int painike = 0;
// PIN that gives us a random number
int randomNumberGeneratorPin = A0;
//
int randomValue;
int randomNumbers[100];
int userNumbers[100];
int indexRandomNumbers = 0; // this can also be used as player score, because it has same value
// These keep track on interrupt speed for the leds
int count1 = 0;
int count2 = 0;
/* ********** GAMELOGIC CHEAT SYSTEM **********
 Speed cheat
 if turned on gamespeed is faster, but still works right according to project assingment.
 Example:
 false: 1 * 0.9^2 = 0.81
 true: 1 - (1 * 0.8) = 0.8

 Speed cheat is enabled when button 0 is pressed in second round
*/
bool gameSpeedCheat = false;
// Display cheat can distract the opposing player since he/she can't be sure of current personal score.
// Display cheat is enabled when button 0 is pressed in fourth round
bool displayCheat = false;
// switchButtonsAround cheat. Example: button 0 -> button 3
bool switchButtonsAround = false;

//**************
uint8_t tempNumber;


void setup()
{
    Serial.begin(9600);

    initializeTimer();

    initializeLeds(9, 10, 11, 12);

    initButtonsAndButtonInterrupts();

    // Display
    // Digital pins 8,9,10
    initDisplay();

    //indicates that game has not started
    setAllLeds();

    // sei(); allows interrupts
    sei();

    lcd_init(0x25);
    lcd_displayScoreboard(); 
}

void loop()
{
    if ((displayCheat == true) && (indexRandomNumbers > 9)){ // This can distract opposing player. He can not know his/hers current score;
        displayNumber(indexRandomNumbers - 5 - tempNumber);
    }
    else displayNumber(indexRandomNumbers);

    switch (state){
    case IDLE:
        break;

    case PRESS:
        Serial.print("Loop button: ");
        Serial.println(painike);
        checkGame(painike);
        break;

    case STOP:
        displayCheat = false;
        setAllLeds(); // indicates that the game has ended
        Serial.println("WRONG");
        TCCR1B &= ~((1 << CS12) | (1 << CS10)); // stops timer1
        TCNT1 = 0; // resets timer1 counter

        // next state
        state = LCD_GAMEOVER;

        EIFR |= (1 << INTF0);
        Serial.println(String("EIFFRE: ") + EIFR);
        EIMSK |= (1 << INT0); // enables start interrupts
        break;

    case START:
        Serial.println("STARTING");
        // Default value is 0, so this changes values in the list to 8. Value changes from 8 to 0-3 when led turns on
        initializeGame();
        TCCR1B |= (1 << CS12) | (1 << CS10);
        state = GAMERUNNING;
        break;

    case GAMERUNNING:
        break;

    case LCD_ASKNAME:
        lcd_displayAskname();
        
        switch (painike) {
        case 0:
        {
            auto next = lcd_moveCursor(Direction::LEFT);
            if (next != NONE) state = next;
            break;
        }
        case 1:
            lcd_moveCursor(Direction::UP);
            break;
        case 2:
            lcd_moveCursor(Direction::DOWN);
            break;
        case 3:
        {
            auto next = lcd_moveCursor(Direction::RIGHT);
            if (next != NONE) state = next;
            break;
        }
        }
        break;

    case LCD_GAMEOVER:
        lcd_newScore(indexRandomNumbers); // set score here
        state = lcd_displayGameover();
        // delay
        // delay(2000);
        break;
    
    case LCD_SCORES:
        lcd_displayScoreboard();
        state = IDLE;
        // delay
        // delay(2000);
        break;

    default:
        break;
    }


  /*
    This loop function constantly checks if
	player has pushed a button and there is
	need for checking the game status
  */
}

ISR(INT0_vect) { // start button interrupt
    Serial.println("GAME BEGINS");
    state = START;
    EIMSK &= ~(1 << INT0);
}


ISR(PCINT2_vect) {
    static byte previousStates = 255;
    byte currentStates = PIND >> firstPin;
    for (byte i = 0; i <= lastPin - firstPin; ++i) {
        byte mask = 1 << i;
        if (((previousStates ^ currentStates) & mask) != 0) {
            if ((currentStates & mask) == 0) {
                if (switchButtonsAround == true){
                    painike = -(i) + 3; // switches buttons around
                }
                else painike = i;
                state = PRESS;
            }
        }
    }
    previousStates = currentStates;
}

    /*
     Here you implement logic for handling
     interrupts from 2,3,4,5 pins for Game push buttons
     and for pin 6 for start Game push button.
    */




ISR(TIMER1_COMPA_vect)
{
    //Random seed generator
    randomValue = analogRead(randomNumberGeneratorPin);
    randomSeed(randomValue);
    // Set random number to randomNumbers -array
    // This is for game speed cheat
    int minValue = 0;
    int maxValue = 4;
    if ((count1 == 1) || (count1 == 3)){
        minValue = 1;
        maxValue = 3;
    }
    else{
        maxValue = 4;
        minValue = 0;
    }

    do{
        //numbers between 0-3 (normally)
        tempNumber = random(minValue, maxValue);
        //this makes sure that same numbers don't appear in row in to the list
        randomNumbers[count1] = tempNumber;
    }while(tempNumber == randomNumbers[count1-1]);



    setLed(randomNumbers[count1]);

    // this keeps track of game speed. Game gets 10% (0.1) faster after every 10 right answers
    // Both speeds are allowed since they work with same logic as given in Project assignment
    if (count1 <= 99){
        Serial.print("led: ");
        Serial.println(randomNumbers[count1]);

        count1++;
        if ((gameSpeedCheat == false) && (count1 % 10 == 0)){
            count2++;
            int temp = (TICKS_PER_SECONDS - (TICKS_PER_SECONDS * (0.1 * count2)));
            OCR1A = temp ? temp : 1562; //faster max game speed
        }
        else if ((gameSpeedCheat == true) && (count1 % 10 == 0)){
            count2++;
            OCR1A *= 0.9; //slower max game speed
        }
        Serial.print("GAME SPEED: ");
        Serial.println(OCR1A);
    }
    else {
        setAllLeds(); // indicates that the game has ended
    }

  /*
    Here you generate a random number that is used to
    set certain led.
	
	Each timer interrupt increments "played" numbers,
	and after each 10 numbers you must increment interrupt
	frequency.
	
	Each generated random number must be stored for later
	comparision against player push button presses.
  */
  
}


void initializeTimer(void)
{   //1 hz
    //set all in TCCR1A register to 0
    TCCR1A = 0;
    //Compare match register // => (16 000 000 / 1024)
    OCR1A = TICKS_PER_SECONDS;
    //CTC mode on (Clear Timer on Compare Match) and 1024 prescaler
    //TCCR1B => timer1, which is 16bit timer
    TCCR1B = (1 << WGM12);
    //enable timer compare interrupt
    TIMSK1 = (1 << OCIE1A);


	// see requirements for the function from SpedenSpelit.h
}

void checkGame(byte nbrOfButtonPush)
{
    if ((count1 == 2) && (nbrOfButtonPush == 0)) { // second button press
        Serial.println("SPEED CHEAT ON!");
        gameSpeedCheat = true;
        indexRandomNumbers++;
        state = GAMERUNNING;
    }
    else if ((count1 == 4) && (nbrOfButtonPush == 0)) { // fourth button press
        Serial.println("DISPLAY CHEAT ON!");
        gameSpeedCheat = true;
        indexRandomNumbers++;
        state = GAMERUNNING;
    }
    else if ((nbrOfButtonPush == randomNumbers[indexRandomNumbers])){
        indexRandomNumbers++;
        state = GAMERUNNING;
    }
    else
        state = STOP;


	// see requirements for the function from SpedenSpelit.h
}



void initializeGame()
{
    clearAllLeds();
    count1 = 0;
    count2 = 0;
    indexRandomNumbers = 0;
    for (int i = 0; i <= 99;i++){randomNumbers[i] = 8;}
    OCR1A = TICKS_PER_SECONDS;
    //cheats
    gameSpeedCheat = false;
    displayCheat = false;
    switchButtonsAround = false;

	// see requirements for the function from SpedenSpelit.h
}
