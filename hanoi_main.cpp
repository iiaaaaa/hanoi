/* 班级 学号 姓名 */

#include <iostream>
#include <conio.h>   //本源程序允许使用，因为要_getch()
#include "hanoi.h"

using namespace std;

/***************************************************************************
  函数名称：main
  功    能：程序入口，循环显示菜单并调用对应功能
***************************************************************************/
int main()
{
    /* 将这段复制到main的最前面 */
    cout << "请确认当前cmd窗口的大小为40行*120列以上，字体为新宋体/16，按C继续，Q退出" << endl;
    while (1) {
        char ch = _getch();
        if (ch == 'C' || ch == 'c')
            break;
        if (ch == 'Q' || ch == 'q')
            return 0;
    }

    while (true) {
        int op = hanoi_menu();
        if (op == 0) break;

        switch (op) {
        case 1: menu_item1_basic(); break;
        case 2: menu_item2_basic_steps(); break;
        case 3: menu_item3_array_horizontal(); break;
        case 4: menu_item4_array_horizontal_vertical(); break;
        case 5: menu_item5_draw_rods(); break;
        case 6: menu_item6_init_disks(); break;
        case 7: menu_item7_first_move(); break;
        case 8: menu_item8_full_demo(); break;
        case 9: menu_item9_game(); break;
        default: break;
        }
    }
    return 0;
}
