#include <stdio.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "images.h"

// global variables
volatile int * LEDR_ptr = (int *)0xFF200000;
volatile int *SW_ptr = (int *) 0xFF200040;
volatile int *KEY_EDGE_ptr = (int *) 0xFF20005C;//hahalol
volatile int pixel_buffer_start;

int *chickImageSelection[4] = {image_UP_22x34, image_DOWN_22x34, image_LEFT_27x34, image_RIGHT_27x34};


void VGA_text(int x, int y, char * text_ptr)
{
    int offset;
    volatile char * character_buffer = (char *) 0xC9000000  ;	// VGA character buffer

    /* assume that the text string fits on one line */
    offset = (y << 7) + x;
    while ( *(text_ptr) )
    {
        // write to the character buffer
        *(character_buffer + offset) = *(text_ptr);
        ++text_ptr;
        ++offset;
    }
}
void plot_pixel(int x, int y, short int line_color) {
    *(short *) (pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

void swap(int *left, int *right) {
    int temp = *left;
    *left = *right;
    *right = temp;
}

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

bool wait_for_vsync() {
    volatile int *pixel_ctrl_ptr = (int *) 0xFF203020; // pixel controller
    register int status;

    *pixel_ctrl_ptr = 1;
    status = *(pixel_ctrl_ptr + 3);
    while ((status & 0x01) != 0) {
        status = *(pixel_ctrl_ptr + 3);
    }
    //*pixel_ctrl_ptr =1;
    return true;
}

void clear_screen() {
    for (int y = 0; y < 240; y++) {
        for (int x = 0; x < 320; x++) {
            plot_pixel(x, y, 0);
        }
    }
}

typedef struct CAR {
    int x;
    int y;
    int speed;
    int imageWidth;
    int imageHeight;
    int carType;
    int *carImage;
    int collisionLeft;
    int collisionRight;
} CAR;


typedef struct road {
    int initialY;
    bool stepOn;
    CAR carOnRoad;
} ROAD;


CAR carsSelection[8] = {
        {.x=-80, .y=-31, .speed=1, .imageWidth=83, .imageHeight=57, .carType=0, .carImage=image_carGreenLTR_83x57, .collisionLeft=
        -80 + 8, .collisionRight=-80 + 77},
        {.x=-80, .y=-36, .speed=2, .imageWidth=64, .imageHeight=56, .carType=1, .carImage=image_carBlueLTR_64x56, .collisionLeft=
        -80 + 5, .collisionRight=-80 + 60},
        {.x=-80, .y=-35, .speed=3, .imageWidth=74, .imageHeight=55, .carType=2, .carImage=image_carYellowLTR_74x55, .collisionLeft=
        -80 + 6, .collisionRight=-80 + 69},
        {.x=-80, .y=-51, .speed=4, .imageWidth=117, .imageHeight=83, .carType=3, .carImage=image_truckRedLTR_117x83, .collisionLeft=
        -80 + 8, .collisionRight=-80 + 113},
        {.x=-80, .y=-35, .speed=4, .imageWidth=74, .imageHeight=53, .carType=4, .carImage=image_carVioletLTR_74x53, .collisionLeft=
        -80 + 5, .collisionRight=-80 + 67},
        {.x=320, .y=73, .speed=3, .imageWidth=74, .imageHeight=51, .carType=5, .carImage=image_carYellowRTL_74x51, .collisionLeft=
        320 + 5, .collisionRight=68},
        {.x=320, .y=71, .speed=2, .imageWidth=74, .imageHeight=52, .carType=6, .carImage=image_carRedRTL_74x52, .collisionLeft=
        320 + 6, .collisionRight=68},
        {.x=320, .y=58, .speed=1, .imageWidth=122, .imageHeight=74, .carType=7, .carImage=image_truckBlueRTL_122x74, .collisionLeft=
        320 + 9, .collisionRight=116},
};

typedef struct chick {
    int x;
    int y;
    int faceType; //0 for up, 1 for down, 2 for left, 3 for right
    int imageWidth;
    int imageHeight;
} chick;

void plot_image(int initialX, int initialY, int imageArray[], unsigned width, unsigned height) {
    int i = 0;
    for (unsigned y = 0; y < height; y++) {
        for (unsigned x = 0; x < width; x++) {
            int plotX = initialX + x;
            int plotY = initialY + y;
            if (imageArray[i] != 0b1111100000011111 && plotX >= 0 && plotY >= 0 && plotX < 320 && plotY < 240)
                plot_pixel(plotX, plotY, imageArray[i]);
            i++;
        }
    }
}

void plot_chicken(int initialX, int initialY, int facetype,){

    int *imageArray = NULL;
    int height = 0;
    int width = 0;

    if(facetype == 0){
        imageArray = chickImageSelection[1];
        height = 34;
        width = 22;
    }else if(facetype == 1){
        imageArray = chickImageSelection[2];
        height = 34;
        width = 22;
    }else if(facetype == 2){
        imageArray = chickImageSelection[3];
        height = 34;
        width = 27;
    }else if(facetype == 3){
        imageArray = chickImageSelection[4];
        height = 34;
        width = 27;
    }

    plot_image(initialX,initialY,imageArray,width,height);
}





void carMove(ROAD *myRoad) {
    switch (myRoad->carOnRoad.carType) {
        case 0 ... 4: {
            if (myRoad->carOnRoad.x + myRoad->carOnRoad.speed * 4 < 320) {
                myRoad->carOnRoad.x += myRoad->carOnRoad.speed * 4;
                myRoad->carOnRoad.y += myRoad->carOnRoad.speed;
            } else {
                myRoad->carOnRoad = carsSelection[rand() % 8];
                myRoad->carOnRoad.y += myRoad->initialY;
            }
            break;
        }
        case 5 ... 7: {
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

bool carHitTest(ROAD *myRoad, chick *myChick) {
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
    int diffY = (myRoad->initialY + myChick->x / 4) - myChick->y;
    bool carOverLap = ((myChick->x + 11) > myRoad->carOnRoad.collisionLeft) &&
                      ((myChick->x + 11) < myRoad->carOnRoad.collisionRight);
    myRoad->stepOn = false;
    bool hitByCar = false;
    if (diffY < 30 && diffY > 0) {
        myRoad->stepOn = true;
        if (carOverLap) {
            hitByCar = true;
        }
    }
    return hitByCar;
}

void plot_road(ROAD *myRoad) {
    plot_image(0, myRoad->initialY, image_road_320x120, 320, 120);
}

void plot_car_on_road(ROAD *myRoad) {
    plot_image(myRoad->carOnRoad.x, myRoad->carOnRoad.y, myRoad->carOnRoad.carImage,
               (unsigned int) myRoad->carOnRoad.imageWidth,
               (unsigned int) myRoad->carOnRoad.imageHeight);
}

void chickMove(int key, chick *myChick) {
    switch (key) {
        case 0b1000: {//up
            if ((myChick->x + 7) < 293 && (myChick->y - 29) > -4) {
                myChick->x += 7;
                myChick->y -= 29;
                myChick->faceType = 0;
            }
            break;
        }
        case 0b0100: {//down
            if ((myChick->x - 7) > 0 && (myChick->y + 29) < 206) {
                myChick->x -= 7;
                myChick->y += 29;
                myChick->faceType = 1;
            }
            break;
        }
        case 0b0010: {//left
            if ((myChick->x - 29) > 0 && (myChick->y - 7) > -4) {
                myChick->x -= 29;
                myChick->y -= 7;
                myChick->faceType = 2;
            }
            break;
        }
        case 0b0001: {//right
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

bool gameOn = false;
bool gameOver = false;

int main() {
    volatile int *pixel_ctrl_ptr = (int *) 0xFF203020;
    // declare other variables(not shown)
    // initialize location and direction of rectangles(not shown)

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
unsigned score_hundred=0;
    unsigned score_ten=0;
    unsigned score_one=0;
unsigned oneSecCount;
    chick newChick;
    ROAD road_3,road_2,road_1,road0, road1, road2, road3, road4, road5, road6, road7;
    newGame:
    {
        *LEDR_ptr = 0;
        oneSecCount=0;
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
        if(oneSecCount==10){
            oneSecCount = 0;
            score++;
            score_hundred = score/100;
            score_ten = (score - score_hundred*100)/10;
            score_one = score - score_hundred*100 - score_ten*10;
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
        if(!gameOver){
            goto gameOverRoutine;
        }
        chickMove(KEY_release, &newChick);


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


        plot_car_on_road(&road_3);
        if (road_3.stepOn)//plot chick
            plot_chicken(newChick.x, newChick.y, chickImageSelection[newChick.faceType]);

        plot_car_on_road(&road_2);
        if (road_2.stepOn)//plot chick
            plot_chicken(newChick.x, newChick.y, chickImageSelection[newChick.faceType]);

        plot_car_on_road(&road_1);
        if (road_1.stepOn)//plot chick
            plot_chicken(newChick.x, newChick.y, chickImageSelection[newChick.faceType]);

        plot_car_on_road(&road0);
        if (road0.stepOn)//plot chick
            plot_chicken(newChick.x, newChick.y, chickImageSelection[newChick.faceType]);

        plot_car_on_road(&road1);
        if (road1.stepOn)//plot chick
            plot_chicken(newChick.x, newChick.y, chickImageSelection[newChick.faceType]);

        plot_car_on_road(&road2);
        if (road2.stepOn)//plot chick
            plot_chicken(newChick.x, newChick.y, chickImageSelection[newChick.faceType]);

        plot_car_on_road(&road3);
        if (road3.stepOn)//plot chick
            plot_chicken(newChick.x, newChick.y, chickImageSelection[newChick.faceType]);

        plot_car_on_road(&road4);
        if (road4.stepOn)//plot chick
            plot_chicken(newChick.x, newChick.y, chickImageSelection[newChick.faceType]);

        plot_car_on_road(&road5);
        if (road5.stepOn)//plot chick
            plot_chicken(newChick.x, newChick.y, chickImageSelection[newChick.faceType]);


        plot_car_on_road(&road6);
        if (road6.stepOn)//plot chick
            plot_chicken(newChick.x, newChick.y, chickImageSelection[newChick.faceType]);

        plot_car_on_road(&road7);
        if (road7.stepOn)//plot chick
            plot_chicken(newChick.x, newChick.y, chickImageSelection[newChick.faceType]);

        //carMove(&carGreenLTR_83x57);




        //plor car
        //plot_image(carGreenLTR_83x57.x,carGreenLTR_83x57.y,carImage[carGreenLTR_83x57.carType],83,57);

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



        gameOverRoutine:{
            if(gameOver){
                plot_image(0,0,image_gameOverPage_320x240,320,240);
                int KEY_release = *KEY_EDGE_ptr;
                *KEY_EDGE_ptr = 0xF;
                if(KEY_release==0b0001){
                    gameOver=false;
                    gameOn=true;
                    goto newGame;
                }
            }
        }


        nextFrame:
        {
            wait_for_vsync(); // swap front and back buffers on VGA vertical sync
            pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
        }
        // plot score
        char myScoreString[40];
        if(score_hundred!=0){
            myScoreString[0] = score_hundred+'0';
        } else{
            myScoreString[0] = ' ';
        }
        if(score_hundred==0&&score_ten==0){
            myScoreString[1] = ' ';
        } else{
            myScoreString[1] = score_ten+'0';
        }
        myScoreString[2]=score_one+'0';
        myScoreString[3]='\0';
        VGA_text(300,0,myScoreString);
    }
    return 0;
}