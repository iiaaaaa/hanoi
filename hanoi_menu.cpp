/* 班级 学号 姓名 */

#include <iostream>
#include <limits>
#include "hanoi.h"
#include "cmd_console_tools.h"

using namespace std;

static void flush_line()
{
    cin.clear();
    cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
}

/***************************************************************************
  函数名称：hanoi_menu
  功    能：显示菜单并返回合法选项（0=退出，1~9）
  输入参数：无
  返 回 值：int
  说    明：本函数只负责“显示 + 读入 + 校验”，不做任何演示逻辑
***************************************************************************/
int hanoi_menu()
{
    while (true) {
        cct_cls();
        cct_gotoxy(0, 0);
        cout << "================= 汉诺塔综合演示（伪图形界面） =================" << endl;
        cout << " 1. 基本解" << endl;
        cout << " 2. 基本解（步数记录）" << endl;
        cout << " 3. 内部数组显示（横向）" << endl;
        cout << " 4. 内部数组显示（纵向 + 横向）" << endl;
        cout << " 5. 画出三根圆柱（伪图形）" << endl;
        cout << " 6. 初始化：在起始柱画出 n 个盘（伪图形）" << endl;
        cout << " 7. 在6的基础上完成第一个盘的移动（伪图形）" << endl;
        cout << " 8. 汉诺塔完整演示（伪图形 + 动画移动）" << endl;
        cout << " 9. 汉诺塔游戏（手动移动）" << endl;
        cout << " 0. 退出" << endl;
        cout << "===============================================================" << endl;
        cout << "请输入选项(0-9)：";

        int op;
        cin >> op;
        if (!cin.good()) { flush_line(); continue; }
        flush_line();
        if (op >= 0 && op <= 9) return op;
    }
}
