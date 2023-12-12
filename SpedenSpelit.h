#ifndef SPEDENSPELIT_H
#define SPEDENSPELIT_H
#include <arduino.h>

constexpr uint16_t TICKS_PER_SECONDS = 15625;

// Switch states for loop
enum State {
    NONE = 0,
    IDLE,
    PRESS,
    STOP,
    START,
    GAMERUNNING,
    LCD_SCORES,
    LCD_GAMEOVER,
    LCD_ASKNAME
};

//#define TICKS_PER_SECONDS 15625;

/*
  initializeTimer() subroutine intializes Arduino Timer1 module to
  give interrupts at rate 1Hz
*/
void initializeTimer(void);

/*
  initializeGame() subroutine is used to initialize all variables
  needed to store random numbers and player button push data.
  This function is called from startTheGame() function.
  
*/
void initializeGame(void);

/*
  checkGame() subroutine is used to check the status
  of the Game after each player button press.
  
  If the latest player button press is wrong, the game stops
  and if the latest press was right, game display is incremented
  by 1.
  
  Parameters
  byte lastButtonPress of the player 0 or 1 or 2 or 3
  
*/
void checkGame(byte);


/*
  startTheGame() subroutine calls InitializeGame()
  function and enables Timer1 interrupts to start
  the Game.
*/




#endif
