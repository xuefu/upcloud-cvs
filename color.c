#include "color.h"

void fg_red_color()
{
    printf("\033[0m\033[31m");
}

void fg_blue_color()
{
    printf("\033[0m\033[34m");
}

void fg_green_color()
{
    printf("\033[0m\033[32m");
}

void fg_purple_color()
{
  printf("\033[0m\033[35m");
}

void reset_color()
{
    printf("\033[0m");
}
