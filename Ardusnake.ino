/* 
 * Ardusnake
 * Copyright (C) 2015 
 * Maicon Hieronymus <polygon6@web.de>
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
 * ------------------------------------------------------------------------------
 *  
 * @file        Ardusnake.ino
 * @date        14.10.2015
 * @version     0.1
 * @author      Maicon Hieronymus <polygon6@web.de>
 *
 * @brief This program is a game similar to Snake on old mobile phones. 
 * It is coded with Arduboy in mind. 
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
boolean pause = false; // True if game is paused.
boolean ingame = false; // True if game started.
boolean enteredName = false; // True if name has been entered for highscore.
char name[3]; // The name which can be entered for highscore.
boolean intercept; // Does the new mouse intercept with the snake?
// byte random = 42; // A number which will be overflowen in order to create a 
                  // pseudorandom new number.
int i; // A counter variable used in for-loops.
char text[16];
                 
// Intro with wooosh - Arduboy
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

// Titlescreen with AAaarrrr - Snake by me.
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
}

// Display Highscore with the scores. Duh!
boolean showHighscore()
{
    
}
                 
// Draw score.
void drawScore()
{
    // Draw a line and the score to the left.
    sprintf(text, "SCORE:%u", score);
    arduboy.setCursor(101, 40);
    arduboy.print(text);
    Arduboy.drawLine(100, 0, 100, 64, 1);
}

// create and draw random mouse.
void createMouse()
{
    // Create new coordinates which are not within the snake.
    intercept = true;
    while(intercept)
    {
        mousePosition[0] = random(0, PLAYGROUNDWIDTH);
        mousePosition[1] = random(0, PLAYGROUNDHEIGTH);
        // Multiply random with an arbitrary number and use modulo to be within
        // the playground.
//         mousePosition[0] = (random*random)%PLAYGROUNDWIDTH;
//         mousePosition[1] = (random*random)%PLAYGROUNDHEIGTH; 
        intercept = false;
        for(i=0; i<snakePosition.length; i++)
        {
            if(snakePosition[i][0] == mousePosition[0]
                && snakePosition[i][1] == mousePosition[1])
            {
                intercept = true;
            }
        }
    }
    mousePoints = eaten*5 + 10;
    // Draw the mouse
    arduboy.drawPixel(mousePosition[0], mousePosition[1], 1);
}

// This inits the snake with a head.
void createSnake()
{
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

// Draw snake.
void drawSnake()
{
    i=0;
    while(snakePosition[i][0] != 255)
    {
        arduboy.drawPixel(snakePosition[i][0], snakePosition[i][1], 1);
        i++;
    }
}

void moveSnake()
{
    // Change position for each segment of the snake except the head. 
    // This is an awful implementation!
    i=1;
    while(snakePosition[i][0] != 255)
    {
        snakePosition[i][0] = snakePosition[i-1][0];
        snakePosition[i][1] = snakePosition[i-1][1];
    }
    // If Button is pressed, use the direction or else just go straight forward.
    switch(arduboy.getInput())
    {
        case UP:
            direction = UP;
            break;
            
        case DOWN:
            direction = DOWN;
            break;
            
        case LEFT:
            direction = LEFT;
            break;
            
        case RIGHT:
            direction = RIGHT;
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

// Returns true, if mouse is eaten, adds the score and calls the method to
// create a new mouse.
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

// Pause.
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

void drawPause()
{
    arduboy.drawBitmap(0, 0, pause, 128, 64, 1); 
    arduboy.display();
}

// Did the snake eat herself (or himself or itself or whatever)?
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

// GameOver
void gameOver()
{
    drawGameOver();
    enterHighscore();
}

// Draw Game Over.
void drawGameOver()
{
    arduboy.clearDisplay();
    arduboy.drawBitmap(0, 0, gameover, 128, 64, 1);
    arduboy.display();
    delay(5000);
}
// Enter Highscore.
void enterHighscore()
{
    // If Highscore is high enough, enter name.
    if()
    {
        enterName();
    } else 
    {
        //delay
    }
    
}

// Enter name.
void enterName()
{
    
}

// Setup code runs once.
void setup
{
    arduboy.start();
    arduboy.setFrameRate(60);
    arduboy.print("Hello World!");
    arduboy.display();
    intro();
    arduboy.initRandomSeed();
}

// Loop is repeated.
void loop
{
    // pause render until it's time for the next frame
    if (!(arduboy.nextFrame())) return;
    
    // Show titlescreen and highscorelist until fire is pressed.
    while (!arduboy.getInput & FIRE_BUTTON)
    {
        showTitle();
        showHighscore();
    }
    
    
    
}