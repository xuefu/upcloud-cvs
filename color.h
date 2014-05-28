/*
 * ===================================================================================
 *
 *      Filename: color.h
 *
 *   Description: 自定义标准输出字体顔色
 *
 *       Version: 1.0
 *       Created: 2014/05/28 20:02:18
 *      Revision: none
 *      Compiler: gcc
 *  
 *        Author: xuefu
 *  Organization: none
 *
 * ===================================================================================
 */

#ifndef _COLOR_H
#define _COLOR_H

#include <stdio.h>

/* set red color */
void fg_red_color();

/* set blue color */
void fg_blue_color();

/* set green color */
void fg_green_color();

/* set purple color */
void fg_purple_color();

/* reset origin color */
void reset_color();
    
#endif
