#include <stdio.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "images.h"
#include "objects.h"


// global variables for I/O devices addresses
volatile int *LEDR_ptr = (int *) 0xFF200000;
volatile int *SW_ptr = (int *) 0xFF200040;
volatile int *KEY_EDGE_ptr = (int *) 0xFF20005C;
volatile char *character_buffer = (char *) 0xC9000000;// VGA character buffer
volatile int *pixel_ctrl_ptr = (int *) 0xFF203020; // pixel controller
volatile int pixel_buffer_start;


// subroutine for plotting text on the screen
void VGA_text(int x, int y, char *text_ptr);

// subroutine for plotting a pixel on the screen
void plot_pixel(int x, int y, short int line_color) {
    *(short *) (pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

// subroutine for plotting a line on the screen (currently not used in this program)
void plot_line(int x0, int y0, int x1, int y1, short int line_color);

// subroutine for plotting an image given a specific location and image array
void plot_image(int initialX, int initialY, int imageArray[], unsigned width, unsigned height);

// subroutine for clearing the whole screen by writing black to every pixel
void clear_screen();

// boolean function for switching the front&back VGA buffer,
// and return to the caller when the plotting is finished
bool wait_for_vsync();


// subroutine for plotting the chicken on the screen
// will select the correct image according to the chicken's current facing
void plot_chicken(chick *myChick);

// subroutine for plotting a road's image onto the screen
void plot_road(ROAD *myRoad) {
    plot_image(0, myRoad->initialY, image_road_320x120, 320, 120);
}

// subroutine for plotting a car's image onto the screen
void plot_car_on_road(ROAD *myRoad) {
    plot_image(myRoad->carOnRoad.x, myRoad->carOnRoad.y, myRoad->carOnRoad.carImage,
               (unsigned int) myRoad->carOnRoad.imageWidth,
               (unsigned int) myRoad->carOnRoad.imageHeight);
}

// function for updating the chicken's location&facing, according to the user's input from KEYs
void chickMove(int key, chick *myChick);

// subroutine for modifying some car's location, as the car should be moving on the road
void carMove(ROAD *myRoad);

// boolean function for checking whether the chicken is hit by the car
//  it also updates which road the chicken is standing on
bool carHitTest(ROAD *myRoad, chick *myChick);


// global varible for determining the game state
bool gameOn = false;
bool gameOver = false;

int main() {

    /* set front pixel buffer to start of FPGA On-chip memory */
    *(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the
    // back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer
    /* set back pixel buffer to start of SDRAM memory */
    *(pixel_ctrl_ptr + 1) = 0xC0000000;


    unsigned SW_value;
    unsigned score;
    unsigned score_hundred = 0;
    unsigned score_ten = 0;
    unsigned score_one = 0;
    unsigned oneSecCount;
    chick newChick;
    ROAD road_3, road_2, road_1, road0, road1, road2, road3, road4, road5, road6, road7;
    newGame:
    {
        *LEDR_ptr = 0;
        oneSecCount = 0;
        gameOver = false;
        gameOn = true;
        score = 0;
        newChick = (chick) {.x = 160, .y= 186, .faceType=0, .imageWidth=22, .imageHeight=34};

        road_3 = (ROAD) {.initialY = -3 * 30, .stepOn= false, .carOnRoad = carsSelection[rand() % 8]};
        road_3.carOnRoad.y += -3 * 30;

        road_2 = (ROAD) {.initialY = -2 * 30, .stepOn= false, .carOnRoad = carsSelection[rand() % 8]};
        road_2.carOnRoad.y += -2 * 30;

        road_1 = (ROAD) {.initialY = -1 * 30, .stepOn= false, .carOnRoad = carsSelection[rand() % 8]};
        road_1.carOnRoad.y += -1 * 30;

        road0 = (ROAD) {.initialY = 0, .stepOn= false, .carOnRoad = carsSelection[rand() % 8]};
        road1 = (ROAD) {.initialY = 1 * 30, .stepOn= false, .carOnRoad = carsSelection[rand() % 8]};
        road1.carOnRoad.y += 1 * 30;
        road2 = (ROAD) {.initialY = 2 * 30, .stepOn= false, .carOnRoad = carsSelection[rand() % 8]};
        road2.carOnRoad.y += 2 * 30;
        road3 = (ROAD) {.initialY = 3 * 30, .stepOn= false, .carOnRoad = carsSelection[rand() % 8]};
        road3.carOnRoad.y += 3 * 30;
        road4 = (ROAD) {.initialY = 4 * 30, .stepOn= false, .carOnRoad = carsSelection[rand() % 8]};
        road4.carOnRoad.y += 4 * 30;
        road5 = (ROAD) {.initialY = 5 * 30, .stepOn= false, .carOnRoad = carsSelection[rand() % 8]};
        road5.carOnRoad.y += 5 * 30;
        road6 = (ROAD) {.initialY = 6 * 30, .stepOn= false, .carOnRoad = carsSelection[rand() % 8]};
        road6.carOnRoad.y += 6 * 30;
        road7 = (ROAD) {.initialY = 7 * 30, .stepOn= false, .carOnRoad = carsSelection[rand() % 8]};
        road7.carOnRoad.y += 7 * 30;
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
    }
    *KEY_EDGE_ptr = 0xF;
    while (true) {
        oneSecCount++;
        if (oneSecCount == 10) {
            oneSecCount = 0;
            score++;
            score_hundred = score / 100;
            score_ten = (score - score_hundred * 100) / 10;
            score_one = score - score_hundred * 100 - score_ten * 10;
            *LEDR_ptr = score;
        }
        clear_screen();
        SW_value = (unsigned int) *SW_ptr; // read SW
        gameOn = (bool) (SW_value >> 9);

        int KEY_release = *KEY_EDGE_ptr;
        *KEY_EDGE_ptr = 0xF;

        if (!gameOn) {
            //plot background
            goto nextFrame;
        }
        if (gameOver) {
            goto gameOverRoutine;
        }

        // update the chicken location and facing if it is moved
        chickMove(KEY_release, &newChick);

        // move the cars
        carMove(&road_3);
        carMove(&road_2);
        carMove(&road_1);
        carMove(&road0);
        carMove(&road1);
        carMove(&road2);
        carMove(&road3);
        carMove(&road4);
        carMove(&road5);
        carMove(&road6);
        carMove(&road7);

        // plot the roads
        plot_road(&road_3);
        plot_road(&road_2);
        plot_road(&road_1);
        plot_road(&road0);
        plot_road(&road1);
        plot_road(&road2);
        plot_road(&road3);
        plot_road(&road4);
        plot_road(&road5);
        plot_road(&road6);
        plot_road(&road7);

        // plot the roads and chicken in order so that the chicken won't appear standing
        // on any cars when it is actually behind the car
        // This achieves 3D interface very well
        plot_car_on_road(&road_3);
        if (road_3.stepOn)
            plot_chicken(&newChick);

        plot_car_on_road(&road_2);
        if (road_2.stepOn)
            plot_chicken(&newChick);

        plot_car_on_road(&road_1);
        if (road_1.stepOn)
            plot_chicken(&newChick);

        plot_car_on_road(&road0);
        if (road0.stepOn)
            plot_chicken(&newChick);

        plot_car_on_road(&road1);
        if (road1.stepOn)
            plot_chicken(&newChick);

        plot_car_on_road(&road2);
        if (road2.stepOn)
            plot_chicken(&newChick);

        plot_car_on_road(&road3);
        if (road3.stepOn)
            plot_chicken(&newChick);

        plot_car_on_road(&road4);
        if (road4.stepOn)
            plot_chicken(&newChick);

        plot_car_on_road(&road5);
        if (road5.stepOn)
            plot_chicken(&newChick);

        plot_car_on_road(&road6);
        if (road6.stepOn)
            plot_chicken(&newChick);

        plot_car_on_road(&road7);
        if (road7.stepOn)
            plot_chicken(&newChick);


        // update the gameOver status
        gameOver |= carHitTest(&road_3, &newChick);
        gameOver |= carHitTest(&road_2, &newChick);
        gameOver |= carHitTest(&road_1, &newChick);
        gameOver |= carHitTest(&road0, &newChick);
        gameOver |= carHitTest(&road1, &newChick);
        gameOver |= carHitTest(&road2, &newChick);
        gameOver |= carHitTest(&road3, &newChick);
        gameOver |= carHitTest(&road4, &newChick);
        gameOver |= carHitTest(&road5, &newChick);
        gameOver |= carHitTest(&road6, &newChick);
        gameOver |= carHitTest(&road7, &newChick);

        gameOverRoutine:
        {
            if (gameOver) {

                plot_image(0, 0, image_gameOverPage_320x240, 320, 240);

                int KEY_release = *KEY_EDGE_ptr;
                *KEY_EDGE_ptr = 0xF;

                if (KEY_release == 0b0001) { // press KEY0 to restart
                    gameOver = false;
                    gameOn = true;
                    goto newGame;
                }
                wait_for_vsync(); // swap front and back buffers on VGA vertical sync
                pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
                goto gameOverRoutine;
            }

        }


        nextFrame:
        {
            wait_for_vsync(); // swap front and back buffers on VGA vertical sync
            pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
        }
        if (!gameOn) {
            goto newGame;
        }

        // plot score
        char myScoreString[40];
        if (score_hundred != 0) {
            myScoreString[0] = score_hundred + '0';
        } else {
            myScoreString[0] = ' ';
        }
        if (score_hundred == 0 && score_ten == 0) {
            myScoreString[1] = ' ';
        } else {
            myScoreString[1] = score_ten + '0';
        }
        myScoreString[2] = score_one + '0';
        myScoreString[3] = '\0';
        VGA_text(300, 0, myScoreString);
    }
    return 0;
}

// subroutine for plotting text on the screen
void VGA_text(int x, int y, char *text_ptr) {
    /* assume that the text string fits on one line */
    int offset = (y << 7) + x;

    while (*(text_ptr)) // while it hasn't reach the null-terminating char in the string
    {
        // write to the character buffer
        *(character_buffer + offset) = *(text_ptr);
        ++text_ptr;
        ++offset;
    }
}


// function for swapping two intergers
void swap(int *left, int *right) {
    int temp = *left;
    *left = *right;
    *right = temp;
}

// subroutine for plotting a line on the screen (currently not used in this program)
void plot_line(int x0, int y0, int x1, int y1, short int line_color) {
    bool is_steep = abs(y1 - y0) > abs(x1 - x0);
    if (is_steep) {
        swap(&x0, &y0);
        swap(&x1, &y1);
    }
    if (x0 > x1) {
        swap(&x0, &x1);
        swap(&y0, &y1);
    }
    int delta_x = x1 - x0;
    int delta_y = abs(y1 - y0);
    int error = -(delta_x / 2);
    int y = y0;
    int y_step;
    if (y0 < y1) {
        y_step = 1;
    } else {
        y_step = -1;
    }

    for (int x = x0; x <= x1; x++) {
        if (is_steep) {
            plot_pixel(y, x, line_color);
        } else {
            plot_pixel(x, y, line_color);
        }
        error += delta_y;
        if (error >= 0) {
            y += y_step;
            error -= delta_x;
        }
    }
}


// subroutine for plotting an image given a specific location and image array
void plot_image(int initialX, int initialY, int imageArray[], unsigned width, unsigned height) {

    int i = 0; // index for pixel colours in the image array

    for (unsigned y = 0; y < height; y++) {
        for (unsigned x = 0; x < width; x++) {
            int plotX = initialX + x;
            int plotY = initialY + y;

            // check for magenta, which is selected as a substitute of the alpha(transparent) colour
            // when the pixel is out of bound, ignore it
            if (imageArray[i] != 0b1111100000011111 && plotX >= 0 && plotY >= 0 && plotX < 320 && plotY < 240)
                plot_pixel(plotX, plotY, imageArray[i]);

            i++; // switch to the next pixel colour
        }
    }
}

// subroutine for clearing the whole screen by writing black to every pixel
void clear_screen() {
    for (int y = 0; y < 240; y++) {
        for (int x = 0; x < 320; x++) {
            plot_pixel(x, y, 0);
        }
    }
}

// boolean function for switching the front&back VGA buffer,
// and return to the caller when the plotting is finished
bool wait_for_vsync() {
    // register for storing the plotting status
    register int status;

    // write to switch the front&back VGA buffer
    *pixel_ctrl_ptr = 1;

    // keep getting the plotting status until the plotting is finished
    // which is denoted by status "1"
    status = *(pixel_ctrl_ptr + 3);
    while ((status & 0x01) != 0) {
        status = *(pixel_ctrl_ptr + 3);
    }

    return true;
}


// subroutine for plotting the chicken on the screen
// will select the correct image according to the chicken's current facing
void plot_chicken(chick *myChick) {

    // properties of the chicken facing image
    unsigned height = 0;
    unsigned width = 0;

    // get the face type from the chicken
    switch (myChick->faceType) {
        case 0: // facing UP
            height = 34;
            width = 22;
            break;
        case 1: // facing DOWN
            height = 34;
            width = 22;
            break;
        case 2: // facing LEFT
            height = 34;
            width = 27;
            break;
        case 3: // facing RIGHT
            height = 34;
            width = 27;
            break;
        default:; // which does not exist, just for suppressing warning
    }

    // plot the image according to the corrections above
    plot_image(myChick->x, myChick->y, chickImageSelection[myChick->faceType], width, height);
}

// function for updating the chicken's location&facing, according to the user's input from KEYs
void chickMove(int key, chick *myChick) {
    switch (key) {
        case 0b1000: {// KEY3 pressed, UP
            if ((myChick->x + 7) < 293 && (myChick->y - 29) > -4) {
                myChick->x += 7;
                myChick->y -= 29;
                myChick->faceType = 0;
            }
            break;
        }
        case 0b0100: {// KEY2 pressed, DOWN
            if ((myChick->x - 7) > 0 && (myChick->y + 29) < 206) {
                myChick->x -= 7;
                myChick->y += 29;
                myChick->faceType = 1;
            }
            break;
        }
        case 0b0010: {// KEY1 pressed, LEFT
            if ((myChick->x - 29) > 0 && (myChick->y - 7) > -4) {
                myChick->x -= 29;
                myChick->y -= 7;
                myChick->faceType = 2;
            }
            break;
        }
        case 0b0001: {// KEY0 pressed, RIGHT
            if ((myChick->x + 29) < 293 && (myChick->y + 7) < 206) {
                myChick->x += 29;
                myChick->y += 7;
                myChick->faceType = 3;
            }
            break;
        }
        default:;
    }
}

// subroutine for modifying some car's location, as the car should be moving on the road
void carMove(ROAD *myRoad) {

    // update the location according to the car's type
    switch (myRoad->carOnRoad.carType) {
        case 0 ... 4: { // those cars are moving from left to right
            if (myRoad->carOnRoad.x + myRoad->carOnRoad.speed * 4 < 320) {
                myRoad->carOnRoad.x += myRoad->carOnRoad.speed * 4;
                myRoad->carOnRoad.y += myRoad->carOnRoad.speed;
            } else {
                myRoad->carOnRoad = carsSelection[rand() % 8];
                myRoad->carOnRoad.y += myRoad->initialY;
            }
            break;
        }
        case 5 ... 7: { // those cars are moving from right to left
            if (myRoad->carOnRoad.x - myRoad->carOnRoad.speed * 4 < -80) {
                myRoad->carOnRoad.x -= myRoad->carOnRoad.speed * 4;
                myRoad->carOnRoad.y -= myRoad->carOnRoad.speed;
            } else {
                myRoad->carOnRoad = carsSelection[rand() % 8];
                myRoad->carOnRoad.y += myRoad->initialY;
            }
            break;
        }
        default:;
    }
}


// boolean function for checking whether the chicken is hit by the car
//  it also updates which road the chicken is standing on
bool carHitTest(ROAD *myRoad, chick *myChick) {

    // the hitting bound should be adjusted according to the car types
    switch (myRoad->carOnRoad.carType) {
        case 0: {
            myRoad->carOnRoad.collisionLeft = myRoad->carOnRoad.x + 8;
            myRoad->carOnRoad.collisionRight = myRoad->carOnRoad.x + 77;
            break;
        }
        case 1: {
            myRoad->carOnRoad.collisionLeft = myRoad->carOnRoad.x + 5;
            myRoad->carOnRoad.collisionRight = myRoad->carOnRoad.x + 60;
            break;
        }
        case 2: {
            myRoad->carOnRoad.collisionLeft = myRoad->carOnRoad.x + 6;
            myRoad->carOnRoad.collisionRight = myRoad->carOnRoad.x + 69;
            break;
        }
        case 3: {
            myRoad->carOnRoad.collisionLeft = myRoad->carOnRoad.x + 8;
            myRoad->carOnRoad.collisionRight = myRoad->carOnRoad.x + 113;
            break;
        }
        case 4: {
            myRoad->carOnRoad.collisionLeft = myRoad->carOnRoad.x + 5;
            myRoad->carOnRoad.collisionRight = myRoad->carOnRoad.x + 67;
            break;
        }
        case 5: {
            myRoad->carOnRoad.collisionLeft = myRoad->carOnRoad.x + 5;
            myRoad->carOnRoad.collisionRight = myRoad->carOnRoad.x + 68;
            break;
        }
        case 6: {
            myRoad->carOnRoad.collisionLeft = myRoad->carOnRoad.x + 6;
            myRoad->carOnRoad.collisionRight = myRoad->carOnRoad.x + 68;
            break;
        }
        case 7: {
            myRoad->carOnRoad.collisionLeft = myRoad->carOnRoad.x + 9;
            myRoad->carOnRoad.collisionRight = myRoad->carOnRoad.x + 116;
            break;
        }
        default:;
    }

    // get the Y difference to determine whether the chicken is on this road
    int diffY = (myRoad->initialY + myChick->x / 4) - myChick->y;

    // check whether the chicken's x coordinate overlaps with the range defined above
    bool carOverLap = ((myChick->x + 11) > myRoad->carOnRoad.collisionLeft) &&
                      ((myChick->x + 11) < myRoad->carOnRoad.collisionRight);

    // assume the chicken is not on the road at first, and it is not hit by any cars as well
    myRoad->stepOn = false;
    bool hitByCar = false;

    // if the chicken is on the road
    if (diffY < 30 && diffY > 0) {
        myRoad->stepOn = true; // update the on Road flag
        if (carOverLap) { // if the x coordinate overlaps with the car
            hitByCar = true; // update the hit flag
        }
    }

    return hitByCar;
}