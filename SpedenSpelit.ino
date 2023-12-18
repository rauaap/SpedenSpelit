#include "display.h"
#include "buttons.h"
#include "leds.h"
#include "SpedenSpelit.h"
#include "lcd_logic.h"


volatile State state = State::IDLE;

// Button that was pressed
int painike = 0;
// PIN that gives us a random number
int randomNumberGeneratorPin = A0;
//
int randomValue;
int randomNumbers[100];
int userNumbers[100];
int indexRandomNumbers = 0; // this can also be used as player score, because it has same value
// Led number that is going to go to the randomNumbers -array
uint8_t ledNumber;
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
bool gameDisplayCheat = false;
/*
 gameSwitchButtonsAroundCheat cheat. Example: button 0 -> button 3. Normally is enabled, but can be disabled by
 pressing button 3 for 1 second.

 NOTE: Buttons are read from left to right
*/
bool gameSwitchButtonsAroundCheat = true;
unsigned long pressTime;
unsigned long releaseTime;
bool cheatButtonPress = false;

void setup()
{
    Serial.begin(9600);
    Serial.println("setup");

    initializeTimer();
    initializeLeds();
    initButtonsAndButtonInterrupts();
    initDisplay();

    // indicates that game has not started
    setAllLeds();

    // sei(); allows interrupts
    sei();

    lcd_init(0x25);
    lcd_displayScoreboard();
}

State prevState = State::IDLE;

/*
    This loop function constantly checks if
    player has pushed a button and there is
    need for checking the game status
*/
void loop()
{
    if (state != prevState)
    {
        Serial.print("STATE CHANGED: ");
        Serial.println(state);
        prevState = state;
    }

    // This display cheat can distract opposing player. He/She can not know his/hers current score (except when the game ends);
    if ((gameDisplayCheat == true) && (indexRandomNumbers > 9)){
        displayNumber(indexRandomNumbers - 5 - ledNumber);
    }
    else displayNumber(indexRandomNumbers);

    switch (state) {
    case State::IDLE:
    case State::GAMERUNNING:
        break;

    case State::PRESS:
    {
        Serial.print("Loop button: ");
        Serial.println(painike);
        checkGame(painike);
        break;
    }

    case State::STOP:
    {
        gameDisplayCheat = false;
        setAllLeds(); // indicates that the game has ended
        Serial.println("WRONG");
        TCCR1B &= ~((1 << CS12) | (1 << CS10)); // State::stops timer1
        TCNT1 = 0; // resets timer1 counter

        // next state
        state = State::LCD_GAMEOVER;

        EIFR |= (1 << INTF0);
        Serial.println(String("EIFFRE: ") + EIFR);
        EIMSK |= (1 << INT0); // enables start interrupts
        break;
    }

    case State::START:
    {
        Serial.println("STARTING");
        // Default value is 0, so this changes values in the list to 8. Value changes from 8 to 0-3 when led turns on
        initializeGame();
        TCCR1B |= (1 << CS12) | (1 << CS10);
        state = State::GAMERUNNING;
        break;
    }

    case State::LCD_ASKNAME:
    {
        if (painike == 0)
        {
            bool res = lcd_moveCursor(Direction::LEFT);
            if (res)
            {
                Serial.println("SKIPPED SCORE SAVE");
                state = State::LCD_SCOREBOARD;
            }
            else lcd_displayAskname();
        }
        else if (painike == 1)
        {
            lcd_moveCursor(Direction::DOWN);
            lcd_displayAskname();
        }
        else if (painike == 2)
        {
            lcd_moveCursor(Direction::UP);
            lcd_displayAskname();
        }
        else if (painike == 3)
        {
            bool res = lcd_moveCursor(Direction::RIGHT);
            if (res)
            {
                Serial.println("SAVED SCORE");
                state = State::LCD_SCOREBOARD;
            }
            else lcd_displayAskname();
        }
        painike = -1;
        break;
    }

    case State::LCD_SCOREBOARD:
    {
        Serial.println("case 1");
        painike = -1;
        Serial.println("case 2");
        lcd_displayScoreboard();
        Serial.println("case 3");
        Serial.println("SHOW SCORES");
        delay(1000);
        state = State::IDLE;
        break;
    }

    case State::LCD_GAMEOVER:
    {
        painike = -1;
        lcd_newScore(indexRandomNumbers); // set score here
        bool res = lcd_displayGameover();
        delay(1000);
        if (res)
        {
            state = State::LCD_ASKNAME;
            lcd_displayAskname();
            painike = -1;
        }
        else
        {
            state = State::LCD_SCOREBOARD;
        }
        break;
    }

    default:
        break;
    }
}


// start button interrupt
ISR(INT0_vect) {
    if (state != State::IDLE) return;

    Serial.println("GAME BEGINS");
    state = State::START;
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

                if (state == State::GAMERUNNING) state = State::PRESS;
            }
        }
    }
    previousStates = currentStates;
}




ISR(TIMER1_COMPA_vect)
{
    // This is for game speed cheat
    int minValue = 0;
    int maxValue = 4;
    if ((count1 == 1) || (count1 == 3)) {
        minValue = 1;
        maxValue = 3;
    }
    else {
        maxValue = 4;
        minValue = 0;
    }

    do {
        // numbers between 0-3 (normally)
        ledNumber = random(minValue, maxValue);
        // this makes sure that same numbers don't appear in row in to the list
        randomNumbers[count1] = ledNumber;
    } while(ledNumber == randomNumbers[count1-1]);

    setLed(randomNumbers[count1]);

    // this keeps track of game speed. Game gets 10% (0.1) faster after every 10 right answers
    // Both speeds are allowed since they work with same logic as given in Project assignment
    if (count1 <= 99) {
        Serial.print("led: ");
        Serial.println(randomNumbers[count1]);

        count1++;
        if ((gameSpeedCheat == false) && (count1 % 10 == 0)) {
            count2++;
            int temp = (TICKS_PER_SECONDS - (TICKS_PER_SECONDS * (0.1 * count2)));
            OCR1A = temp ? temp : 1562; //faster max game speed
        }
        else if ((gameSpeedCheat == true) && (count1 % 10 == 0)) {
            count2++;
            OCR1A *= 0.9; //slower max game speed
        }
        Serial.print("GAME SPEED: ");
        Serial.println(OCR1A);
    }
    else {
        setAllLeds(); // indicates that the game has ended
    }
}


void initializeTimer(void)
{
    // 1 hz
    // set all in TCCR1A register to 0
    TCCR1A = 0;
    // Compare match register // => (16 000 000 / 1024)
    OCR1A = TICKS_PER_SECONDS;
    // CTC mode on (Clear Timer on Compare Match) and 1024 prescaler
    // TCCR1B => timer1, which is 16bit timer
    TCCR1B = (1 << WGM12);
    // enable timer compare interrupt
    TIMSK1 = (1 << OCIE1A);
}


void checkGame(byte nbrOfButtonPush)
{
    // button 3 pressed for 1 second when game hasn't begun. nbrOfButtonPush == 0, because the gameSwitchButtonsAroundCheat is currently on
    if ((count1 == 0) && nbrOfButtonPush == 0){
        if ((digitalRead(lastpin) == HIGH) && (cheatButtonPress == false)){ //reads from pin 6
            pressTime = millis();
            cheatButtonPress = true;
        }
        else if ((digitalRead(lastpin) == LOW) && (cheatButtonPress == true)){
            releaseTime = millis();
            cheatButtonPress = false;
            unsigned long pressDuration = releaseTime - pressTime;
            if (pressDuration > 1000){
                Serial.println("SWITCH BUTTONS CHEAT OFF!");
                gameSwitchButtonsAroundCheat = false;
            }
        }
        state = State::IDLE;
    }
    state = State::GAMERUNNING;
    if ((count1 == 2) && (nbrOfButtonPush == 0)) { // second button press
        Serial.println("SPEED CHEAT ON!");
        gameSpeedCheat = true;
        indexRandomNumbers++;
        // state = State::GAMERUNNING;
    }
    else if ((count1 == 4) && (nbrOfButtonPush == 0)) { // fourth button press
        Serial.println("DISPLAY CHEAT ON!");
        gameSpeedCheat = true;
        indexRandomNumbers++;
        // state = State::GAMERUNNING;
    }
    else if ((nbrOfButtonPush == randomNumbers[indexRandomNumbers])) {
        indexRandomNumbers++;
        // state = State::GAMERUNNING;
    }
    else
        state = State::STOP;
}


void initializeGame()
{
    // Read static from an unconnected analog pin to seed 
    // the random number generator
    randomValue = analogRead(randomNumberGeneratorPin);
    randomSeed(randomValue);

    clearAllLeds();
    count1 = 0;
    count2 = 0;
    indexRandomNumbers = 0;
    for (int i = 0; i <= 99;i++) {randomNumbers[i] = 8;}
    OCR1A = TICKS_PER_SECONDS;
    // cheats
    gameSpeedCheat = false;
    gameDisplayCheat = false;
    gameSwitchButtonsAroundCheat = false;
}
