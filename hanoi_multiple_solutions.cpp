/* 班级 学号 姓名 */
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdio>
#include <Windows.h>
#include <conio.h>
#include "cmd_console_tools.h"
#include "cmd_hdc_tools.h"
#include "hanoi_const_value.h" 
using namespace std;

// =================================================================
// 全局变量引用 (配合 hanoi_main.cpp)
[cite_start]// 依据作业要求: 允许使用全局变量记录盘子状态和延时 [cite: 115-118]
// =================================================================
extern int yuanzhu[3][10];
extern int top[3];
extern int time1;       // 步数记录
static int t = 0;       // 延时 (static 限制在本文件)
extern const int win_width;
extern const int win_high;
extern const int win_bgcolor;
extern const int win_fgcolor;

// =================================================================
// 内部工具函数: 坐标计算
// 说明: 所有坐标均通过 HDC_Base_Width 等常量动态计算
// =================================================================

// 计算底座的总宽度 (像素) = (最大盘宽 + 2 * 基础宽)
static int get_pan_width() { return (2 * MAX_LAYER + 3) * HDC_Base_Width; }

// 计算某柱子(0-2)底座左侧 X 坐标
static int get_rod_x(int r) {
    return HDC_Start_X + r * (get_pan_width() + HDC_Underpan_Distance);
}

// 计算某柱子中心 X 坐标
static int get_center_x(int r) { return get_rod_x(r) + get_pan_width() / 2; }

// 计算某盘子(size=n)的宽度
static int get_disk_w(int n) { return (2 * n + 1) * HDC_Base_Width; }

// 计算某盘子在某柱子上的左侧 X 坐标
static int get_disk_x(int r, int n) { return get_center_x(r) - get_disk_w(n) / 2; }

// 计算第 level 层 (0为最底层) 的 Y 坐标
static int get_disk_y(int level) { return HDC_Start_Y - (level + 1) * HDC_Base_High; }

// 智能擦除: 只在必要时修补柱子, 防止闪烁
static void smart_erase(int x, int y, int w, int h, int r, int repair) {
    hdc_rectangle(x, y, w, h, HDC_COLOR[0]); // 擦除盘子
    if (repair) { // 修补柱子
        int px = get_center_x(r) - HDC_Base_Width / 2;
        // 只有当柱子在被擦除区域内时才重画
        if (px >= x && (px + HDC_Base_Width) <= (x + w))
            hdc_rectangle(px, y, HDC_Base_Width, h, HDC_COLOR[MAX_LAYER + 1]);
    }
}

// =================================================================
// 图形与动画函数 (对应 main 中的调用)
// =================================================================

// 对应 main 中的 yuanpan(): 画三根柱子
void yuanpan() {
    int pw = get_pan_width();
    int ph = HDC_Base_High;
    int col = HDC_COLOR[MAX_LAYER + 1];

    for (int r = 0; r < 3; r++) {
        int rx = get_rod_x(r);
        // 画底座
        hdc_rectangle(rx, HDC_Start_Y, pw, ph, col);
        // 画立柱
        int px = rx + pw / 2 - HDC_Base_Width / 2;
        // 柱子高度
        int py = HDC_Top_Y + 2 * HDC_Base_High;
        int ph_pole = HDC_Start_Y - py;
        hdc_rectangle(px, py, HDC_Base_Width, ph_pole, col);
        if (HDC_Init_Delay) Sleep(HDC_Init_Delay);
    }
}

// 对应 main 中的 chushi(): 画初始盘子
void chushi(int n) {
    int src = 0; // 默认在A柱(0)
    if (top[0] == 0 && top[1] > 0) src = 1; // 简单的自动检测起始柱
    if (top[0] == 0 && top[1] == 0) src = 2;

    for (int i = 0; i < n; i++) {
        // 从底层往上画: yuanzhu[src][0] 是最底层的盘子
        int disk = yuanzhu[src][i];
        int x = get_disk_x(src, disk);
        int y = get_disk_y(i);
        hdc_rectangle(x, y, get_disk_w(disk), HDC_Base_High, HDC_COLOR[disk]);
        if (HDC_Init_Delay) Sleep(HDC_Init_Delay);
    }
}

// 内部移动动画实现 (替代原有的 move/direction 逻辑, 更流畅)
static void anim_impl(int disk, int r1, int r2, int src_lvl, int dst_lvl) {
    int cx = get_disk_x(r1, disk);
    int cy = get_disk_y(src_lvl);
    int tx = get_disk_x(r2, disk);
    int ty = get_disk_y(dst_lvl);
    int w = get_disk_w(disk);
    int h = HDC_Base_High;
    int delay = (t > 0) ? t : 0;

    // 1. 上移 (修补源柱)
    while (cy > HDC_Top_Y) {
        smart_erase(cx, cy, w, h, r1, 1);
        cy -= HDC_Step_Y;
        if (cy < HDC_Top_Y) cy = HDC_Top_Y;
        hdc_rectangle(cx, cy, w, h, HDC_COLOR[disk]);
        if (delay) Sleep(delay);
    }
    // 2. 平移 (不修补)
    int dir = (cx < tx) ? 1 : -1;
    while (cx != tx) {
        smart_erase(cx, cy, w, h, 0, 0);
        cx += dir * HDC_Step_X;
        if ((dir == 1 && cx > tx) || (dir == -1 && cx < tx)) cx = tx;
        hdc_rectangle(cx, cy, w, h, HDC_COLOR[disk]);
        if (delay) Sleep(delay);
    }
    // 3. 下移 (修补目标柱)
    while (cy < ty) {
        smart_erase(cx, cy, w, h, r2, 1);
        cy += HDC_Step_Y;
        if (cy > ty) cy = ty;
        hdc_rectangle(cx, cy, w, h, HDC_COLOR[disk]);
        if (delay) Sleep(delay);
    }
    if (t == 0) while (_getch() != '\r'); // 单步模式等待回车
}

// 对应 main 中的 MOVE
void MOVE(char start, char end, int n) {
    int r1 = start - 'A';
    int r2 = end - 'A';
    // 注意: 调用前 top 已经更新了, 所以 src_lvl 是原高度(现在top), dst_lvl 是新高度(现在top-1)
    // 需反推一下物理位置:
    // 此时 yuanzhu[r2][top[r2]-1] 已经是盘子 n 了
    int src_lvl = top[r1]; // 因为已经 pop 了, 原位置就是当前 top
    int dst_lvl = top[r2] - 1; // 已经 push 了, 位置是 top-1

    anim_impl(n, r1, r2, src_lvl, dst_lvl);
}

// =================================================================
// 文本输出与逻辑函数
// =================================================================

// 辅助打印: 横向
void dayin(int n, char src, char dst, int flag) {
    // 纯文本模式
    if (flag == 1) { cout << n << "# " << src << "---->" << dst << endl; return; }
    if (flag == 2) { return; } // 步数模式在递归外打印

    // 伪图形文本坐标设置
    int bx = MenuItem4_Start_X;
    int by = MenuItem4_Start_Y;
    if (flag >= 8) { bx = MenuItem8_Start_X; by = MenuItem8_Start_Y; }

    // 打印状态行
    cct_gotoxy(bx, by);
    if (time1 == 0) cout << "第" << setw(4) << time1 << " 步: 初始状态      ";
    else            cout << "第" << setw(4) << time1 << " 步: (" << n << "#: " << src << "-->" << dst << ") ";

    // 打印横向数组
    for (int r = 0; r < 3; r++) {
        cct_gotoxy(bx, by + 1 + r);
        cout << " " << (char)('A' + r) << ":";
        for (int i = 0; i < top[r]; i++) cout << setw(2) << yuanzhu[r][i];
        cout << "            "; // 清除残余
    }

    // 打印纵向图形 (仅菜单4, 8, 9)
    if (flag == 4 || flag >= 8) {
        int ax = bx + Underpan_A_X_OFFSET;
        int ay = by + Underpan_A_Y_OFFSET;
        // 打印底标
        cct_gotoxy(ax, ay); cout << "A";
        cct_gotoxy(ax + Underpan_Distance, ay); cout << "B";
        cct_gotoxy(ax + Underpan_Distance * 2, ay); cout << "C";
        // 打印盘子
        for (int h = 0; h < MAX_LAYER; h++) {
            int py = ay - 1 - h;
            for (int r = 0; r < 3; r++) {
                cct_gotoxy(ax + r * Underpan_Distance, py);
                if (h < top[r]) cout << setw(2) << yuanzhu[r][h];
                else cout << " |";
            }
        }
    }
}

// 对应 main 中的逻辑: 从源柱取, 放入目标柱
void pull(char m, char n, int flag, int pan) {
    if (time1 == 0 && flag >= 3) dayin(pan, m, n, flag); // 打印初始状态

    // 逻辑移动
    int r1 = m - 'A', r2 = n - 'A';
    int disk = yuanzhu[r1][--top[r1]];
    yuanzhu[r2][top[r2]++] = disk;
    time1++; // 步数+1

    // 打印移动后状态
    if (flag >= 3) dayin(pan, m, n, flag);

    // 执行图形动画
    if (flag >= 7) MOVE(m, n, disk); // 菜单7,8,9需要动画
}

// 对应 main 中的 hanoi 递归 (严格控制行数)
void hanoi(int n, char src, char tmp, char dst, int flag) {
    if (n == 1) {
        pull(src, dst, flag, n);
    }
    else {
        hanoi(n - 1, src, dst, tmp, flag);
        pull(src, dst, flag, n);
        hanoi(n - 1, tmp, src, dst, flag);
    }
}

// =================================================================
// 输入与初始化 (对应 main 中的调用)
// =================================================================

void chushihua(char first, char last, int n, int a) {
    time1 = 0;
    // 重置数组
    for (int i = 0; i < 3; i++) {
        top[i] = 0;
        for (int j = 0; j < MAX_LAYER; j++) yuanzhu[i][j] = 0;
    }
    // 初始化起始柱
    int r = first - 'A';
    for (int i = n; i >= 1; i--) yuanzhu[r][top[r]++] = i;

    // 针对特定菜单的预处理
    if (a == 4) {
        cct_cls();
        dayin(0, 0, 0, 4); // 打印初始
        if (t) Sleep(t); else while (_getch() != '\r');
    }
    else if (a >= 8) {
        // 菜单8,9需要预先初始化图形界面
        cct_cls();
        hdc_init(win_bgcolor, win_fgcolor, win_width, win_high);
        hdc_cls();
        yuanpan(); // 画柱子
        chushi(n); // 画盘子
        if (t) Sleep(t); else if (a == 8) while (_getch() != '\r');
    }
}

void input(int* n, char* first, char* mm, char* last, int a) {
    if (a == 5) return; // 菜单5不需要输入

    // 输入层数
    while (1) {
        cout << "请输入层数(1-" << MAX_LAYER << "): ";
        cin >> *n;
        if (cin.good() && *n >= 1 && *n <= MAX_LAYER) break;
        cin.clear(); cin.ignore(1024, '\n');
    }
    // 输入柱子
    while (1) {
        cout << "请输入起始柱和目标柱 (如 A C): ";
        cin >> *first >> *last;
        // 转大写
        if (*first >= 'a') *first -= 32;
        if (*last >= 'a') *last -= 32;
        if (*first >= 'A' && *first <= 'C' && *last >= 'A' && *last <= 'C' && *first != *last) break;
        cout << "输入错误, 请重输。" << endl;
    }
    // 计算中间柱
    if (*first != 'A' && *last != 'A') *mm = 'A';
    else if (*first != 'B' && *last != 'B') *mm = 'B';
    else *mm = 'C';

    // 输入延时
    if (a == 4 || a >= 7) {
        while (1) {
            cout << "请输入延时(0-200, 0为单步): ";
            cin >> t;
            if (cin.good() && t >= 0 && t <= 1000) break;
            cin.clear(); cin.ignore(1024, '\n');
        }
    }
}

void game(char last, int n) {
    // 游戏逻辑: 循环输入指令
    char s_char, d_char;
    while (1) {
        cct_gotoxy(MenuItem8_Start_X, MenuItem8_Start_Y + 3);
        cout << "请输入移动(如AC), Q退出:              "; // 清空旧字
        cct_gotoxy(MenuItem8_Start_X + 22, MenuItem8_Start_Y + 3);

        s_char = _getche(); // 使用 getche 无论如何都会回显
        if (s_char == 'Q' || s_char == 'q') break;
        d_char = _getche();
        if (t == 0) while (_getch() != '\r'); // 吸收回车

        if (s_char >= 'a') s_char -= 32;
        if (d_char >= 'a') d_char -= 32;

        // 合法性检查
        int r1 = s_char - 'A', r2 = d_char - 'A';
        if (r1 < 0 || r1 > 2 || r2 < 0 || r2 > 2 || r1 == r2) continue;

        if (top[r1] == 0) {
            cct_gotoxy(MenuItem8_Start_X, MenuItem8_Start_Y + 4); cout << "源柱为空!   ";
            Sleep(1000); cct_gotoxy(MenuItem8_Start_X, MenuItem8_Start_Y + 4); cout << "            ";
            continue;
        }
        int disk_s = yuanzhu[r1][top[r1] - 1];
        int disk_d = (top[r2] > 0) ? yuanzhu[r2][top[r2] - 1] : 999;

        if (disk_s > disk_d) {
            cct_gotoxy(MenuItem8_Start_X, MenuItem8_Start_Y + 4); cout << "大盘压小盘! ";
            Sleep(1000); cct_gotoxy(MenuItem8_Start_X, MenuItem8_Start_Y + 4); cout << "            ";
            continue;
        }

        // 执行移动 (复用 pull 里的逻辑)
        pull(s_char, d_char, 9, 0);

        // 胜利判断
        if (top[last - 'A'] == n) {
            cct_gotoxy(MenuItem8_Start_X, MenuItem8_Start_Y + 4);
            cout << "游戏结束! 总步数: " << time1;
            _getch();
            break;
        }
    }
}