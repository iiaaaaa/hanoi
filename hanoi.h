#pragma once
// 汉诺塔综合演示 —— 只放跨文件的函数声明/常量
// 注意：本作业要求可替换 hanoi_const_value.h，因此所有显示参数一律从该头文件读取
#include "hanoi_const_value.h"

// 菜单：显示并返回选择（0=退出，1~9 为菜单项）
int hanoi_menu();

// 菜单项执行函数（在 hanoi_multiple_solutions.cpp 中实现）
void menu_item1_basic();
void menu_item2_basic_steps();
void menu_item3_array_horizontal();
void menu_item4_array_horizontal_vertical();
void menu_item5_draw_rods();
void menu_item6_init_disks();
void menu_item7_first_move();
void menu_item8_full_demo();
void menu_item9_game();
