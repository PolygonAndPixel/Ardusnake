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
 * @date        09.11.2015
 * @version     0.9
 * @author      Maicon Hieronymus <mhierony@students.uni-mainz.de>
 *
 * @brief This program is a game similar to Snake on old mobile phones. 
 * It is coded with Arduboy in mind. I took some inspirations from
 * https://github.com/dragula96/snakeling
 */
#include "Arduboy.h"
#include "ardusnake_bitmaps.h"

#define FIRE_BUTTON 1
#define PAUSE_BUTTON 2
#define DOWN 64
#define RIGHT 4
#define UP 16
#define LEFT 32

Arduboy arduboy;

const byte PLAYGROUNDWIDTH = 99;
const byte PLAYGROUNDHEIGTH = 62;
byte direction; // The current direction of the snake. 4,8,16,32 <=> d,r,u,l
byte snakeLength = 1; // The current length of the snake.
// The X,Y-Position of each of the snake's segments. The head is always on 
// the lowest index.
byte snakePosition[125][2]; 
byte mousePoints; // The points which can be gathered by the current mouse.                                              
byte mousePosition[2]; // The X,Y-Position of a mouse.    
unsigned int score; // The current score.
unsigned int eaten; // The amount of eaten mices.
boolean initLevel = true;
boolean gamePaused = false; // True if game is paused.
boolean ingame = false; // True if game started.
char name[3]; // The name which can be entered for highscore.
boolean intercept; // Does the new mouse intercept with the snake?
int i; // A counter variable used in for-loops.
char text[16];
byte speed = 10; // The higher the framerate, the harder it gets.
byte speeddelay;
byte input;
                 
/**
 * @brief Intro with wooosh - Arduboy.
 */
void showIntro()
{
    arduboy.clearDisplay();
    // X, Y, name, width, height, color
    arduboy.drawBitmap(0, 0, arduboyLogo, 128, 64, 1); 
    arduboy.display();

    //arduboy.tunes.tone(987, 160);
    delay(160);
    //arduboy.tunes.tone(1318, 400);
    delay(2000);
}

/**
 * @brief Titlescreen with AAaarrrr - Snake by me.
 * TODO: create nice music.
 */
void showTitle()
{
    arduboy.clearDisplay();
    // X, Y, name, width, height, color
    arduboy.drawBitmap(0, 0, title, 128, 64, 1); 
    arduboy.display();

    //arduboy.tunes.tone(987, 160);
    delay(160);
    //arduboy.tunes.tone(1318, 400);
    delay(2000);
}
                 
/**
 * @brief Draw the score on the right of the payground with a line on the left
 * to bound the playground.
 */
void drawScore()
{
    // Draw a line and the score to the left.
    sprintf(text, "SCORE");
    arduboy.setCursor(101, 20);
    arduboy.print(text);
    arduboy.setCursor(101, 40);
    arduboy.print(score); 
    arduboy.drawLine(100, 0, 100, 63, 1);
    arduboy.drawLine(0, 0, 100, 0, 1);
    arduboy.drawLine(0, 0, 0, 63, 1);
    arduboy.drawLine(0, 63, 100, 63, 1); // Not drawn
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
        mousePosition[1] = random(1, PLAYGROUNDWIDTH);
        mousePosition[0] = random(1, PLAYGROUNDHEIGTH);
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
    mousePoints = 10-speed + 10;
    drawMouse();
}

/**
 * @brief Draw a mouse which consists only of a single pixel.
 */
void drawMouse()
{
    arduboy.drawPixel(mousePosition[1], mousePosition[0], 1);
}

/**
 * @brief Calls the functions to show 'Game Over', enter the highscore and start
 * a new level.
 */
void gameOver()
{
    drawGameOver();
    arduboy.clearDisplay();
    initLevel = true;
}

/**
 * @brief Initiate a snake with a head only and draw the snake on the middle of
 * the playground with a direction to the right.
 */
void createSnake()
{
    snakeLength = 1;
    // PLAYGROUNDWIDTH*PLAYGROUNDHEIGTH is the max length of the snake.
    for(i=1; i<125; i++)
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
        arduboy.drawPixel(snakePosition[i][1], snakePosition[i][0], 1);
    }
}

/**
 * @brief Move the snake towards the current direction or the direction of
 * the pressed button and draw the new position.
 */
void moveSnake()
{
    speeddelay--;
    input = arduboy.getInput();
    if(speeddelay < 1)
    {
        speeddelay = speed;
        // Change position for each segment of the snake except the head. 
        // This is an awful implementation!
        i=snakeLength-1;
        while(i>0)
        {
            snakePosition[i][0] = snakePosition[i-1][0];
            snakePosition[i][1] = snakePosition[i-1][1];
            i--;
        }
        // If Button is pressed, use the direction or else just go straight forward.
        // Using two directions at once does not work!
        switch(input)
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
                break;
        }
        switch(direction)
        {
            case DOWN:
                // Bouncing on a wall?
                if(snakePosition[0][0] != PLAYGROUNDHEIGTH-1) 
                {
                    snakePosition[0][0]++;
                } else 
                {
                    gameOver();
                }
                break;
                
            case RIGHT:
                    // Bouncing on a wall?
                if(snakePosition[0][1] != PLAYGROUNDWIDTH-1) 
                {
                    snakePosition[0][1]++;
                } else 
                {
                    gameOver();
                }
                break;
                
            case UP:
                // Bouncing on a wall?
                if(snakePosition[0][0] != 0) 
                {
                    snakePosition[0][0]--;
                } else 
                {
                    gameOver();
                }
                break;
                
            case LEFT:
                // Bouncing on a wall?
                if(snakePosition[0][1] != 0) 
                {
                    snakePosition[0][1]--;
                } else 
                {
                    gameOver();
                }
                break;
                
            default:
                break;
        }
        if(eatingYourself())
        {
            gameOver();
        }
    }
}

/**
 * @brief Checks if the mouse is eaten and increases the score and calls
 * createMouse() if it is true.
 */
void didSnakeEat()
{
    if(snakePosition[0][0] == mousePosition[0]
        && snakePosition[0][1] == mousePosition[1])
    {
        score += mousePoints;
        eaten++;
        snakeLength++;
        createMouse();
        if(snakeLength==125)
        {
            if(speed > 1)
            {
                speed--;
            }
            snakeLength = 1;
        }
    }
}

/**
 * @brief Pauses the game until PAUSE_BUTTON is pressed again and redraws
 * the playground.
 */
void pauseGame()
{
    gamePaused = true;
    drawpauseGame();
    while(gamePaused)
    {
        // Once PauseButton is pressed, resume.
        if(arduboy.pressed(A_BUTTON)) gamePaused = false;
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
void drawpauseGame()
{
    arduboy.drawBitmap(28, 17, pause, 72, 20, 1); 
    arduboy.display();
}

/**
 * @brief Checks if the snake collides with herself.
 * @return True if there is a collision with the snake.
 */
boolean eatingYourself()
{
    i=1;
    while(snakePosition[i][0] != 255 && i != snakeLength)
    {
        if(snakePosition[0][0] == snakePosition[i][0]
            && snakePosition[0][1] == snakePosition[i][1])
        {
            return true;
        }
        i++;
    }
    return false;
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
    ingame = false;
}

/**
 * @brief Let the player decide the difficulty.
 */
void showMenu()
{
    boolean selected = false;
    byte pointer[2];
    byte selection = 0;
    pointer[1] = 0;
    pointer[0] = 23;
    byte rectWidth = 32;
    arduboy.clearDisplay();
    arduboy.setCursor(0, 25);
    arduboy.print(" Easy  Medium  Hard");
    arduboy.setCursor(0, 5);
    arduboy.print("Select difficulty");
    delay(500);
    
    while(!selected)
    {
        arduboy.clearDisplay();
        arduboy.setCursor(0, 25);
        arduboy.print(" Easy  Medium  Hard");
        arduboy.setCursor(0, 5);
        arduboy.print("Select difficulty");
        switch(selection)
        {
            case 0:
                rectWidth = 32;
                pointer[1] = 1;
                break;

            case 1:
                rectWidth = 47;
                pointer[1] = 36;
                break;

            case 2:
                rectWidth = 31;
                pointer[1] = 85;
                break;

            default:
                break;
        }
        
        arduboy.drawRect(pointer[1], pointer[0], rectWidth, 15, 1);
        arduboy.display();
        
        input = arduboy.getInput();
        switch(input)
        {
            case LEFT:
                if(selection > 0) selection--;
                break;
                
            case RIGHT:
                if(selection < 2) selection++;
                break;
                
            case FIRE_BUTTON:
            case PAUSE_BUTTON:
                selected = true;
                ingame = true;
                switch(selection)
                {
                    case 0:
                        speed = 10;
                        break;
                        
                    case 1:
                        speed = 5;
                        break;
                        
                    case 2:
                        speed = 1;
                        break;
                        
                    default:
                        speed = 5;
                        break;
                }
                break;
                
            default:
                break;
        }  
        delay(200);
    }
}

/**
 * @brief Prints 'Hello World!', shows the intro and inits the seed for random
 * numbers.
 */
void setup()
{
    arduboy.start();
    arduboy.setFrameRate(60);
    showIntro();
    arduboy.initRandomSeed();
    showTitle();
    while(!arduboy.getInput());
}

/**
 * @brief This function calls all the other functions during the game.
 */
void loop()
{
    // pause render until it's time for the next frame
    if (!(arduboy.nextFrame())) return;
    arduboy.clearDisplay();
    // Show titlescreen and highscorelist until fire is pressed.
    if(!ingame)
    {
        showMenu();
    } else 
    {
        if(initLevel)
        {
            speeddelay = speed;
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
        if(arduboy.pressed(B_BUTTON)) pauseGame();
        moveSnake();
        drawSnake();
        didSnakeEat();
        arduboy.display();
    }
}
