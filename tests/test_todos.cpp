// ============================================================
//  自动评测 v2 · 检查 4 个 TODO（无窗口运行）
//  跑法：make test
//  比 v1 强在：
//    ① 用「键盘 mock」真正测 FireBullets（能抓到少 break / 方向错）
//    ② 更狠的用例：错位下标(i≠j)、幽灵敌人、对角同时移动、四条边界
//    ③ 断言失败时打印「期望 vs 实际」，一眼看出差在哪
//    ④ 按 4 个 TODO 分组统计
//  原理：先 include raylib.h，再用宏把 IsKeyPressed 换成可编程的假货，
//        然后 include src/main.cpp（UNIT_TEST 跳过它自己的 main）。
// ============================================================
#include "raylib.h"
#include <cstdio>
#include <cmath>

// —— 键盘 mock：把 g_fakeKey 设成 KEY_UP/... 即“假装按下”那个键 ——
static int g_fakeKey = -1;
static bool FakeIsKeyPressed(int k) { return k == g_fakeKey; }
#define IsKeyPressed(k) FakeIsKeyPressed(k)   // 之后 main.cpp 里的调用都走假货

#define UNIT_TEST
#include "../src/main.cpp"

// 控制台初始化：UTF-8 显示中文 + 打开 ANSI 颜色（不 include windows.h 以避开与 raylib 冲突）
#ifdef _WIN32
extern "C" __declspec(dllimport) int   __stdcall SetConsoleOutputCP(unsigned int cp);
extern "C" __declspec(dllimport) void* __stdcall GetStdHandle(unsigned long n);
extern "C" __declspec(dllimport) int   __stdcall GetConsoleMode(void* h, unsigned long* mode);
extern "C" __declspec(dllimport) int   __stdcall SetConsoleMode(void* h, unsigned long mode);
static void initConsole() {
    SetConsoleOutputCP(65001);                              // 65001 = UTF-8
    void* h = GetStdHandle((unsigned long)-11);             // STD_OUTPUT_HANDLE
    unsigned long m = 0;
    if (GetConsoleMode(h, &m)) SetConsoleMode(h, m | 0x0004); // 打开虚拟终端(ANSI 颜色)
}
#else
static void initConsole() {}   // Linux/mintty 终端天生支持，无需处理
#endif

// —— 颜色（Linux / mintty / 新版 Windows 终端都认这套 ANSI 码）——
#define C_GRN "\033[32m"
#define C_RED "\033[31m"
#define C_DIM "\033[2m"
#define C_RST "\033[0m"

// —— 断言与统计 ——
static int g_pass = 0, g_fail = 0, g_secPass = 0, g_secFail = 0;
static void P(bool ok) { if (ok) { g_pass++; g_secPass++; } else { g_fail++; g_secFail++; } }

static void ck_(const char* name, bool ok, int line) {
    if (ok) printf("  " C_GRN "[通过]" C_RST "   %s\n", name);
    else    printf("  " C_RED "[未通过]" C_RST " %s  " C_DIM "(test_todos.cpp:%d)" C_RST "\n", name, line);
    P(ok);
}
static void ckf_(const char* name, float expect, float actual, int line) {   // 浮点带容差
    bool ok = fabsf(expect - actual) < 1e-4f;
    if (ok) printf("  " C_GRN "[通过]" C_RST "   %s\n", name);
    else    printf("  " C_RED "[未通过]" C_RST " %s （期望 %.2f，实际 %.2f） " C_DIM "(test_todos.cpp:%d)" C_RST "\n", name, expect, actual, line);
    P(ok);
}
static void cki_(const char* name, int expect, int actual, int line) {
    bool ok = (expect == actual);
    if (ok) printf("  " C_GRN "[通过]" C_RST "   %s\n", name);
    else    printf("  " C_RED "[未通过]" C_RST " %s （期望 %d，实际 %d） " C_DIM "(test_todos.cpp:%d)" C_RST "\n", name, expect, actual, line);
    P(ok);
}
// 宏包一层：自动带上调用处的源码行号（__LINE__），像专业框架那样能定位到失败的断言
#define ck(name, ok)      ck_(name, ok, __LINE__)
#define ckf(name, e, a)   ckf_(name, e, a, __LINE__)
#define cki(name, e, a)   cki_(name, e, a, __LINE__)

static void beginSec(const char* t) { g_secPass = g_secFail = 0; printf("\n" C_DIM "%s" C_RST "\n", t); }
static void endSec() {
    const char* c = (g_secFail == 0) ? C_GRN : C_RED;
    printf("   %s—— 本组通过 %d / %d ——" C_RST "\n", c, g_secPass, g_secPass + g_secFail);
}

// 每个测试前把全局状态清成干净局面
static void resetAll() {
    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
    for (int i = 0; i < MAX_ENEMIES; i++) enemies[i].active = false;
    player = { { 480, 270, 32, 32 }, 4.0f, 100 };
    score = 0; spawnTimer = 0; gameOver = false; g_fakeKey = -1;
}
static int countBullets() { int n = 0; for (int i = 0; i < MAX_BULLETS; i++) if (bullets[i].active) n++; return n; }
static int firstBullet()  { for (int i = 0; i < MAX_BULLETS; i++) if (bullets[i].active) return i; return -1; }

// ==================== ① UpdateBullets ====================
static void test_UpdateBullets() {
    beginSec("① UpdateBullets —— 子弹飞行 + 出界回收");

    resetAll();
    bullets[0] = { { 100, 100, 8, 8 }, 5, -3, true };
    UpdateBullets();
    ckf("按 vx 走 (x:100→105)", 105, bullets[0].rect.x);
    ckf("按 vy 走 (y:100→97)",  97,  bullets[0].rect.y);
    ck ("没在用的子弹不动", bullets[1].active == false);

    resetAll();
    bullets[0] = { { 480, 270, 8, 8 }, 0, 0, true };
    UpdateBullets();
    ck("屏幕正中的子弹不该被回收", bullets[0].active == true);

    resetAll(); bullets[0] = { { (float)(SCREEN_W - 1), 270, 8, 8 }, 5, 0, true };
    UpdateBullets(); ck("飞出右边界→回收", bullets[0].active == false);
    resetAll(); bullets[0] = { { 1, 270, 8, 8 }, -5, 0, true };
    UpdateBullets(); ck("飞出左边界→回收", bullets[0].active == false);
    resetAll(); bullets[0] = { { 480, 1, 8, 8 }, 0, -5, true };
    UpdateBullets(); ck("飞出上边界→回收", bullets[0].active == false);
    resetAll(); bullets[0] = { { 480, (float)(SCREEN_H - 1), 8, 8 }, 0, 5, true };
    UpdateBullets(); ck("飞出下边界→回收", bullets[0].active == false);
    endSec();
}

// ==================== ② FireBullets（含键盘 mock）====================
static void test_FireBullets() {
    beginSec("② FireBullets —— 按方向键发射（已能真正测）");

    resetAll(); FireBullets();
    cki("没按键→0 颗子弹", 0, countBullets());

    resetAll(); g_fakeKey = KEY_UP; FireBullets();
    cki("按↑只发 1 颗（少 break 会喷一大串）", 1, countBullets());

    resetAll(); g_fakeKey = KEY_UP; FireBullets();
    int i = firstBullet();
    if (i < 0) ck("按↑应产生一颗子弹", false);
    else {
        ck("↑：vy<0 且 vx==0（笔直向上）", bullets[i].vy < 0 && bullets[i].vx == 0);
        ck("子弹有大小（宽高>0，不是隐形点）", bullets[i].rect.width > 0 && bullets[i].rect.height > 0);
        ck("子弹出现在玩家附近", fabsf(bullets[i].rect.x - player.rect.x) <= 40 &&
                                 fabsf(bullets[i].rect.y - player.rect.y) <= 40);
    }

    resetAll(); g_fakeKey = KEY_DOWN;  FireBullets(); i = firstBullet(); ck("↓：vy>0", i >= 0 && bullets[i].vy > 0);
    resetAll(); g_fakeKey = KEY_LEFT;  FireBullets(); i = firstBullet(); ck("←：vx<0", i >= 0 && bullets[i].vx < 0);
    resetAll(); g_fakeKey = KEY_RIGHT; FireBullets(); i = firstBullet(); ck("→：vx>0", i >= 0 && bullets[i].vx > 0);

    resetAll();
    bullets[0].active = true; bullets[0].rect = { 10, 10, 8, 8 };   // 0 号已被占用
    g_fakeKey = KEY_UP; FireBullets();
    cki("已有 1 颗时再发→共 2 颗（会找空位）", 2, countBullets());
    ck ("原有子弹没被覆盖", bullets[0].rect.x == 10 && bullets[0].rect.y == 10);
    endSec();
}

// ==================== ③ HandleHits ====================
static void test_HandleHits() {
    beginSec("③ HandleHits —— 子弹撞敌人得分");

    resetAll();
    bullets[0] = { { 200, 200, 8, 8 }, 0, 0, true };
    enemies[0] = { { 201, 201, 30, 30 }, 1.5f, true };
    HandleHits();
    ck ("命中后子弹消失", bullets[0].active == false);
    ck ("命中后敌人消失", enemies[0].active == false);
    cki("命中 +10 分", 10, score);

    // 错位下标：3 号子弹撞 1 号敌人（把 enemies[j] 写成 enemies[i] 会挂在这）
    resetAll();
    bullets[3] = { { 300, 300, 8, 8 }, 0, 0, true };
    enemies[1] = { { 301, 301, 30, 30 }, 1.5f, true };
    enemies[3] = { { 50, 50, 30, 30 }, 1.5f, true };   // 3 号敌人在别处，不该被误杀
    HandleHits();
    ck ("被撞的 1 号敌人消失", enemies[1].active == false);
    ck ("无辜的 3 号敌人还活着（下标别用混）", enemies[3].active == true);
    cki("错位命中也只 +10", 10, score);

    // 幽灵敌人：已死(inactive)但 rect 还压在原地，不该得分
    resetAll();
    bullets[0] = { { 400, 400, 8, 8 }, 0, 0, true };
    enemies[0] = { { 401, 401, 30, 30 }, 1.5f, false };
    HandleHits();
    cki("撞到已死敌人不加分", 0, score);
    ck ("子弹不该被幽灵吃掉", bullets[0].active == true);

    // 不相撞
    resetAll();
    bullets[0] = { { 0, 0, 8, 8 }, 0, 0, true };
    enemies[0] = { { 500, 500, 30, 30 }, 1.5f, true };
    HandleHits();
    cki("不相撞→不加分", 0, score);
    ck ("不相撞→子弹敌人都还在", bullets[0].active && enemies[0].active);
    endSec();
}

// ==================== ④ UpdateEnemies ====================
static void test_UpdateEnemies() {
    beginSec("④ UpdateEnemies —— 敌人追人 + 撞人掉血");

    resetAll();
    player.rect = { 500, 300, 32, 32 };
    enemies[0] = { { 100, 300, 30, 30 }, 2.0f, true };
    UpdateEnemies();
    ckf("玩家在右→敌人 x 增大 (100→102)", 102, enemies[0].rect.x);

    resetAll();
    player.rect = { 300, 500, 32, 32 };
    enemies[0] = { { 300, 100, 30, 30 }, 2.0f, true };
    UpdateEnemies();
    ckf("玩家在下→敌人 y 增大 (100→102)", 102, enemies[0].rect.y);

    // 对角：x、y 应在同一帧都动（写成 else-if 会漏掉一个轴）
    resetAll();
    player.rect = { 500, 500, 32, 32 };
    enemies[0] = { { 100, 100, 30, 30 }, 2.0f, true };
    UpdateEnemies();
    ck("左上方敌人→x、y 同时靠近（防 else-if 只动一轴）",
       enemies[0].rect.x == 102 && enemies[0].rect.y == 102);

    resetAll();
    player.rect = { 300, 300, 32, 32 }; player.hp = 100;
    enemies[0] = { { 305, 305, 30, 30 }, 2.0f, true };
    UpdateEnemies();
    cki("撞到玩家掉 10 血", 90, player.hp);
    ck ("撞到玩家后该敌人消失", enemies[0].active == false);

    resetAll();
    player.rect = { 300, 300, 32, 32 }; player.hp = 10;
    enemies[0] = { { 305, 305, 30, 30 }, 2.0f, true };
    UpdateEnemies();
    ck("血量归零→gameOver=true", gameOver == true);

    resetAll();
    enemies[5].active = false; enemies[5].rect = { 0, 0, 30, 30 };
    UpdateEnemies();
    ck("没在用的敌人不该被移动", enemies[5].rect.x == 0 && enemies[5].rect.y == 0);
    endSec();
}

// ==================== 入口 ====================
int main() {
    initConsole();               // UTF-8 中文 + ANSI 颜色
    printf(C_DIM "==== 自动评测 v2：检查 4 个 TODO ====" C_RST "\n");
    test_UpdateBullets();
    test_FireBullets();
    test_HandleHits();
    test_UpdateEnemies();
    if (g_fail == 0)
        printf("\n" C_GRN "==== 总计：通过 %d 项，全部通过，太棒了！ ====" C_RST "\n", g_pass);
    else
        printf("\n==== 总计：通过 " C_GRN "%d" C_RST " 项，未通过 " C_RED "%d" C_RST " 项 ====\n"
               "看上面红色 " C_RED "[未通过]" C_RST " 行的 (期望/实际) 和源码行号。\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}



