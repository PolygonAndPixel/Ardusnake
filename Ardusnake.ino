/* 
 * Ardusnake
 * Copyright (C) 2015 
 * Maicon Hieronymus <mhierony@students.uni-mainz.de>
 * All rights reserved.
 * 
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 * -----------------------------------------------------------------------------
 *  
 * @file        Ardusnake.ino
 * @date        14.10.2015
 * @version     0.2
 * @author      Maicon Hieronymus <mhierony@students.uni-mainz.de>
 *
 * @brief This program is a game similar to Snake on old mobile phones. 
 * It is coded with Arduboy in mind. Since I do not have an Arduboy yet,
 * I have not tested anything yet. 
 * Enter name and display highscore are taken from Sebastian Goscik as seen in
 * https://github.com/Arduboy/Arduboy/blob/master/examples/ArduBreakout/ArduBreakout.ino
 * and slightly modified.
 */
#include "Arduboy.h"
Arduboy arduboy;

#define FIRE_BUTTON 1
#define PAUSE_BUTTON 2
#define DOWN 4
#define RIGHT 8
#define UP 16
#define LEFT 32

const byte PLAYGROUNDWIDTH = 100;
const byte PLAYGROUNDHEIGTH = 64
byte direction; // The current direction of the snake. 4,8,16,32 <=> d,r,u,l
byte snakeLength = 1; // The current length of the snake.
// The X,Y-Position of each of the snake's segments. The head is always on 
// the lowest index.
byte snakePosition[PLAYGROUNDWIDTH*PLAYGROUNDHEIGTH][2];
byte mousePoints; // The points which can be gathered by the current mouse.                                              
byte[] mousePosition = new byte[2] // The X,Y-Position of a mouse.    
unsigned int score = 0; // The current score.
unsigned int eaten = 0; // The amount of eaten mices.
boolean initLevel = true;
boolean pause = false; // True if game is paused.
boolean ingame = false; // True if game started.
char name[3]; // The name which can be entered for highscore.
boolean intercept; // Does the new mouse intercept with the snake?
int i; // A counter variable used in for-loops.
char text[16];
                 
/**
 * @brief Intro with wooosh - Arduboy.
 */
void showIntro()
{
    for(i = 0; i < 16; i++)
    {
        arduboy.clearDisplay();
        // X, Y, name, width, height, color
        arduboy.drawBitmap(0, 0, arduboyLogo, 128, 64, 1); 
        arduboy.display();
    }

    arduboy.tunes.tone(987, 160);
    delay(160);
    arduboy.tunes.tone(1318, 400);
    delay(2000);
}

/**
 * @brief Titlescreen with AAaarrrr - Snake by me.
 * TODO: create nice music.
 */
void showTitle()
{
    for(i = 0; i < 16; i++)
    {
        arduboy.clearDisplay();
        // X, Y, name, width, height, color
        arduboy.drawBitmap(0, 0, title, 128, 64, 1); 
        arduboy.display();
    }

    arduboy.tunes.tone(987, 160);
    delay(160);
    arduboy.tunes.tone(1318, 400);
    delay(2000);
    
    for(i = 0; i < 16; i++)
    {
        arduboy.clearDisplay();
        // X, Y, name, width, height, color
        arduboy.drawBitmap(0, 0, author, 128, 64, 1); 
        arduboy.display();
    }
    
    arduboy.tunes.tone(987, 160);
    delay(160);
    arduboy.tunes.tone(1318, 400);
    delay(2000);
}

/**
 * @brief Display Highscore with the scores. 
 * This method is from Sebastian Goscik as seen in
 * https://github.com/Arduboy/Arduboy/blob/master/examples/ArduBreakout/ArduBreakout.ino
 * and slightly modified.
 * Function by nootropic design to display highscores.
 */
void showHighscore()
{
    byte y = 10;
    byte x = 24;
    // Each block of EEPROM has 10 high scores, and each high score entry
    // is 5 bytes long:  3 bytes for initials and two bytes for score.
    int address = 2*10*5;
    byte hi, lo;
    arduboy.clearDisplay();
    arduboy.drawBitmap(0, 0, highscore, 128, 24, 1);
    arduboy.display();

    for(i=0; i<10; i++)
    {
        sprintf(text, "%2d", i+1);
        arduboy.setCursor(x,y+(i*8));
        arduboy.print(text);
        arduboy.display();
        hi = EEPROM.read(address + (5*i));
        lo = EEPROM.read(address + (5*i) + 1);

        if ((hi == 0xFF) && (lo == 0xFF))
        {
            score = 0;
        }
        else
        {
            score = (hi << 8) | lo;
        }

        initials[0] = (char)EEPROM.read(address + (5*i) + 2);
        initials[1] = (char)EEPROM.read(address + (5*i) + 3);
        initials[2] = (char)EEPROM.read(address + (5*i) + 4);

        if (score > 0)
        {
            sprintf(text, "%c%c%c %u", initials[0], initials[1], initials[2], score);
            arduboy.setCursor(x + 24, y + (i*8));
            arduboy.print(text);
            arduboy.display();
        }
    }
    arduboy.display();
}
                 
/**
 * @brief Draw the score on the right of the payground with a line on the left
 * to bound the playground.
 */
void drawScore()
{
    // Draw a line and the score to the left.
    sprintf(text, "SCORE:%u", score);
    arduboy.setCursor(101, 40);
    arduboy.print(text);
    Arduboy.drawLine(100, 0, 100, 64, 1);
}

/**
 * @brief Create and draw a mouse with random position.
 */
void createMouse()
{
    // Create new coordinates which are not within the snake.
    intercept = true;
    while(intercept)
    {
        mousePosition[0] = random(0, PLAYGROUNDWIDTH);
        mousePosition[1] = random(0, PLAYGROUNDHEIGTH);
        intercept = false;
        for(i=0; i<snakeLength; i++)
        {
            if(snakePosition[i][0] == mousePosition[0]
                && snakePosition[i][1] == mousePosition[1])
            {
                intercept = true;
            }
        }
    }
    mousePoints = eaten*5 + 10;
    drawMouse();
}

/**
 * @brief Draw a mouse which consists only of a single pixel.
 */
void drawMouse()
{
    arduboy.drawPixel(mousePosition[0], mousePosition[1], 1);
}

/**
 * @brief Initiate a snake with a head only and draw the snake on the middle of
 * the playground with a direction to the right.
 */
void createSnake()
{
    snakeLength = 1;
    for(i=1; i<snakePosition.length; i++)
    {
        // Non-existent bodyparts are set to 255 since there is no coordinate
        // with 255.
        snakePosition[i][0] = 255;
        snakePosition[i][1] = 255;
    }
    // Start in the middle and move to the right.
    snakePosition[0][0] = 49;
    snakePosition[0][1] = 31;
    direction = RIGHT;
    drawSnake();
}

/**
 * @brief Draw a snake with only one pixel per segment.
 */
void drawSnake()
{
    for(i=0; i<snakeLength; i++)
    {
        arduboy.drawPixel(snakePosition[i][0], snakePosition[i][1], 1);
    }
}

/**
 * @brief Move the snake towards the current direction or the direction of
 * the pressed button and draw the new position.
 */
void moveSnake()
{
    // Change position for each segment of the snake except the head. 
    // This is an awful implementation!
    i=1;
    while(i<snakeLength)
    {
        snakePosition[i][0] = snakePosition[i-1][0];
        snakePosition[i][1] = snakePosition[i-1][1];
        i++;
    }
    // If Button is pressed, use the direction or else just go straight forward.
    // Using two directions at once does not work!
    switch(arduboy.getInput())
    {
        case UP:
            if(direction != DOWN) direction = UP;
            break;
            
        case DOWN:
            if(direction != UP) direction = DOWN;
            break;
            
        case LEFT:
            if(direction != RIGHT) direction = LEFT;
            break;
            
        case RIGHT:
            if(direction != LEFT) direction = RIGHT;
            break;
            
        default:
            // Do not change direction
    }
    switch(direction)
    {
        case DOWN:
            // Bouncing on a wall?
            if(snakePosition[0][0] != 0) 
            {
                snakePosition[0][0]--;
            } else 
            {
                gameOver();
            }
            break;
            
        case RIGHT:
                // Bouncing on a wall?
            if(snakePosition[0][1] != PLAYGROUNDWIDTH-1) 
            {
                snakePosition[0][0]++;
            } else 
            {
                gameOver();
            }
            break;
            
        case UP:
            // Bouncing on a wall?
            if(snakePosition[0][0] != PLAYGROUNDHEIGTH-1) 
            {
                snakePosition[0][0]++;
            } else 
            {
                gameOver();
            }
            break;
            
        case DOWN:
            // Bouncing on a wall?
            if(snakePosition[0][1] != 0) 
            {
                snakePosition[0][0]--;
            } else 
            {
                gameOver();
            }
            break;
            
        default:
    }
    if(eatingYourself) gameOver();
    drawSnake();
}

/**
 * @brief Checks if the mouse is eaten and increases the score and calls
 * createMouse() if it is true.
 * @return Returns true if mouse is eaten.
 */
boolean didSnakeEat()
{
    if(snakePosition[0][0] == mousePosition[0]
        && snakePosition[0][1] == mousePosition[1])
    {
        score += mousePoints;
        eaten++;
        createMouse();
        return true;
    }
    return false;
}

/**
 * @brief Pauses the game until PAUSE_BUTTON is pressed again and redraws
 * the playground.
 */
void pause()
{
    pause = true;
    drawPause();
    while(pause)
    {
        // Once PauseButton is pressed, resume.
        if(arduboy.getInput() & PAUSE_BUTTON) pause = false;
    }
    // Redraw everything from the playground necessary?
    arduboy.clearDisplay();
    drawScore();
    drawSnake();
    arduboy.drawPixel(mousePosition[0], mousePosition[1], 1);
}

/**
 * @brief Draw a Bitmap 'pause' on the screen.
 */
void drawPause()
{
    arduboy.drawBitmap(0, 0, pause, 128, 64, 1); 
    arduboy.display();
}

/**
 * @brief Checks if the snake collides with herself.
 * @return True if there is a collision with the snake.
 */
boolean eatingYourself()
{
    i=1;
    while(snakePosition[i][0] != 255)
    {
        if(snakePosition[0][0] == snakePosition[i][0]
            && snakePosition[0][1] == snakePosition[i][1])
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief Calls the functions to show 'Game Over', enter the highscore and start
 * a new level.
 */
void gameOver()
{
    drawGameOver();
    if(score>0) enterHighscore();
    arduboy.clearDisplay();
    initLevel = true;
}

/**
 * @brief Clears the screen and draws a bitmap 'gameover' on the screen.
 */
void drawGameOver()
{
    arduboy.clearDisplay();
    arduboy.drawBitmap(0, 0, gameover, 128, 64, 1);
    arduboy.display();
    delay(5000);
}

/**
 * @brief This method lets the user enter his or hers name along with the 
 * highscore.
 * This method is from Sebastian Goscik as seen in
 * https://github.com/Arduboy/Arduboy/blob/master/examples/ArduBreakout/ArduBreakout.ino
 * and slightly modified.
 */
void enterHighscore()
{
    // Each block of EEPROM has 10 high scores, and each high score entry
    // is 5 bytes long:  3 bytes for initials and two bytes for score.
    int address = file * 10 * 5;
    byte hi, lo;
    char tmpInitials[3];
    unsigned int tmpScore = 0;

    // High score processing
    for(byte i = 0; i < 10; i++)
    {
        hi = EEPROM.read(address + (5*i));
        lo = EEPROM.read(address + (5*i) + 1);
        if ((hi == 0xFF) && (lo == 0xFF))
        {
            // The values are uninitialized, so treat this entry
            // as a score of 0.
            tmpScore = 0;
        } else
        {
            tmpScore = (hi << 8) | lo;
        }
        if (score > tmpScore)
        {
            enterInitials();
            for(byte j=i;j<10;j++)
            {
                hi = EEPROM.read(address + (5*j));
                lo = EEPROM.read(address + (5*j) + 1);

                if ((hi == 0xFF) && (lo == 0xFF))
                {
                    tmpScore = 0;
                }
                else
                {
                    tmpScore = (hi << 8) | lo;
                }

                tmpInitials[0] = (char)EEPROM.read(address + (5*j) + 2);
                tmpInitials[1] = (char)EEPROM.read(address + (5*j) + 3);
                tmpInitials[2] = (char)EEPROM.read(address + (5*j) + 4);

                // write score and initials to current slot
                EEPROM.write(address + (5*j), ((score >> 8) & 0xFF));
                EEPROM.write(address + (5*j) + 1, (score & 0xFF));
                EEPROM.write(address + (5*j) + 2, initials[0]);
                EEPROM.write(address + (5*j) + 3, initials[1]);
                EEPROM.write(address + (5*j) + 4, initials[2]);

                // tmpScore and tmpInitials now hold what we want to
                //write in the next slot.
                score = tmpScore;
                initials[0] = tmpInitials[0];
                initials[1] = tmpInitials[1];
                initials[2] = tmpInitials[2];
            }

            score = 0;
            initials[0] = ' ';
            initials[1] = ' ';
            initials[2] = ' ';

            return;
        }
    }
}

/**
 * @brief Prints 'Hello World!', shows the intro and inits the seed for random
 * numbers.
 */
void setup
{
    arduboy.start();
    arduboy.setFrameRate(60);
    arduboy.print("Hello World!");
    arduboy.display();
    intro();
    arduboy.initRandomSeed();
}

/**
 * @brief This function calls all the other functions during the game.
 */
void loop
{
    // pause render until it's time for the next frame
    if (!(arduboy.nextFrame())) return;
    
    // Show titlescreen and highscorelist until fire is pressed.
    while ((!arduboy.getInput & FIRE_BUTTON) && !ingame)
    {
        showTitle();
        showHighscore();
    }
    ingame = true;
    if(initLevel)
    {
        score = 0;
        eaten = 0;
        createMouse();
        createSnake();
        initLevel=false;
    }
    drawSnake();
    drawMouse();
    drawScore();
    
    // Pause if PAUSE is pressed.
    if(arduboy.getInput() & PAUSE_BUTTON) pause();
    moveSnake();
    arduboy.display();
}