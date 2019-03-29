//
// Created by Junhao Liao on 2019-03-29.
//

#ifndef REFURBSOUP_OBJECTS_H
#define REFURBSOUP_OBJECTS_H

#include <stdbool.h>

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

typedef struct chick {
    int x;
    int y;
    int faceType; //0 for up, 1 for down, 2 for left, 3 for right
    int imageWidth;
    int imageHeight;
} chick;


// image array for displaying different chicken facing
int *chickImageSelection[4] = {image_UP_22x34, image_DOWN_22x34, image_LEFT_27x34, image_RIGHT_27x34};

// car array which can be randomly selected from when the car re/generates
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
#endif //REFURBSOUP_OBJECTS_H
