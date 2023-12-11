#include "display.h"
#include "buttons.h"
#include "leds.h"
#include "SpedenSpelit.h"

// Switch states for loop
enum State {
    IDLE,
    PRESS,
    STOP,
    START,
    GAMERUNNING
};
volatile State state = IDLE;

// Button that was pressed
int painike;
// PIN that gives us a random number
int randomNumberGeneratorPin = A0;
//
int randomNumbers[100];
int userNumbers[100];
int indexRandomNumbers = 0; // this can also be used as player score, because it has same value
// Led number that is going to go to the randomNumbers -array
uint8_t ledNumber;
// These keep track on interrupt speed for the leds
int count1 = 0; //has to be zero here in order for gameSwitchButtonsAroundCheat to work
int count2;
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
bool gameDisplayCheat = false;
/*
 switchButtonsAround cheat. Example: button 0 -> button 3. Normally is enabled, but can be disabled by
 pressing button 3 for 1 second.

 NOTE: Buttons are read from left to right
*/
bool gameSwitchButtonsAroundCheat = true;
unsigned long pressTime;
unsigned long releaseTime;
bool buttonPress = false;

void setup()
{
    Serial.begin(9600);

    initializeTimer();

    initializeLeds();

    initButtonsAndButtonInterrupts();

    // Display
    // Digital pins 8,9,10
    initDisplay();

    //indicates that game has not started
    setAllLeds();

    // sei(); allows interrupts
    sei();
}

void loop()
{
    /* DEBUG
    static State prevState = IDLE;

    if (state != prevState) {
        Serial.println(String("State: ") + state);
        prevState = state;
    }
    */

    // This display cheat can distract opposing player. He/She can not know his/hers current score (except when the game ends);
    if ((gameDisplayCheat == true) && (indexRandomNumbers > 9)){
        displayNumber(indexRandomNumbers - 5 - ledNumber);
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
        gameDisplayCheat = false; // needs to be disabled in order to show the final score
        gameSwitchButtonsAroundCheat = true; // needs to be reanabled here. Can't be in initializeGame() since, this has to be turned on before the game begins
        setAllLeds(); // indicates that the game has ended
        Serial.println("WRONG");
        TCCR1B &= ~((1 << CS12) | (1 << CS10)); // stops timer1
        TCNT1 = 0; // resets timer1 counter
        state = IDLE;
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
                if (gameSwitchButtonsAroundCheat == true){
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
    int randomValue = analogRead(randomNumberGeneratorPin);
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
        ledNumber = random(minValue, maxValue);
        //this makes sure that same numbers don't appear in row in to the list
        randomNumbers[count1] = ledNumber;
    }while(ledNumber == randomNumbers[count1-1]);

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
	//s
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
    // button 3 pressed for 1 second when game hasn't begun. nbrOfButtonPush == 0, because the gameSwitchButtonsAroundCheat is currently on
    if ((count1 == 0) && nbrOfButtonPush == 0){
        if (digitalRead(lastpin) == HIGH && (buttonPress == false)){ //reads from pin 6
            pressTime = millis();
            buttonPress = true;
        }
        else if (digitalRead(lastpin) && (buttonPress == true)){
            releaseTime = millis();
            buttonPress = false;
            unsigned long pressDuration = releaseTime - pressTime;
            if (pressDuration > 1000){
                Serial.println("SWITCH BUTTONS CHEAT OFF!");
                gameSwitchButtonsAroundCheat = false;
            }
        }
        state = IDLE;
    }
    // button 0, second button press after game has begun
    else if ((count1 == 2) && (nbrOfButtonPush == 0)) {
        Serial.println("SPEED CHEAT ON!");
        gameSpeedCheat = true;
        indexRandomNumbers++;
        state = GAMERUNNING;
    }
    // button 0, fourth button press after game has begun
    else if ((count1 == 4) && (nbrOfButtonPush == 0)) {
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
    count1 = 0;
    count2 = 0;
    indexRandomNumbers = 0;
    for (int i = 0; i <= 99;i++){randomNumbers[i] = 8;}
    OCR1A = TICKS_PER_SECONDS;
    //cheats
    gameSpeedCheat = false;
    gameDisplayCheat = false;

	// see requirements for the function from SpedenSpelit.h
}

