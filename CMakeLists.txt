cmake_minimum_required(VERSION 3.13)
project(refurbSoup C)


SET(GCCGNU99 "-std=gnu99")

add_executable(refurbSoup main.c objects.h interrupt.h)
add_definitions(${GCCGNU99})