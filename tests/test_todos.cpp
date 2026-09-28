// ============================================================
//  第 2 课 · 自动评测：检查配置表相关的 4 个可测 TODO
//  跑法：make test        （测你的 src/main.cpp）
//        make test-sol    （老师验证参考答案）
//  可自动测：② 配置表数值、③ 刷怪种类+满血、④ 追人速度按种类、⑥ 扣血+按种类给分
//  画面类的 ①(玩家/子弹贴图) ⑤(敌人贴图) 靠眼睛看——跑起来四种怪长相不同即对。
//  输出用英文（ASCII），任何终端都不乱码；说明与手册的 TODO 序号一一对应。
//  原理：include src/main.cpp（UNIT_TEST 跳过它的 main），直接调用里面的函数。
// ============================================================
#include "raylib.h"
#include <cstdio>
#include <cmath>

#ifndef SRC_MAIN
#define SRC_MAIN "../src/main.cpp"      // 默认测孩子的文件；make test-sol 会改指到参考答案
#endif
#define UNIT_TEST
#include SRC_MAIN

// 控制台：开 ANSI 颜色（Windows 需要，Linux 天生支持）。输出纯英文，无需改代码页。
#ifdef _WIN32
extern "C" __declspec(dllimport) void* __stdcall GetStdHandle(unsigned long n);
extern "C" __declspec(dllimport) int   __stdcall GetConsoleMode(void* h, unsigned long* mode);
extern "C" __declspec(dllimport) int   __stdcall SetConsoleMode(void* h, unsigned long mode);
static void initConsole() {
    void* h = GetStdHandle((unsigned long)-11);
    unsigned long m = 0;
    if (GetConsoleMode(h, &m)) SetConsoleMode(h, m | 0x0004);
}
#else
static void initConsole() {}
#endif

#define C_GRN "\033[32m"
#define C_RED "\033[31m"
#define C_DIM "\033[2m"
#define C_RST "\033[0m"

static int g_pass = 0, g_fail = 0, g_secPass = 0, g_secFail = 0;
static void P(bool ok) { if (ok) { g_pass++; g_secPass++; } else { g_fail++; g_secFail++; } }
static void ck_(const char* name, bool ok, int line) {
    if (ok) printf("  " C_GRN "[PASS]" C_RST " %s\n", name);
    else    printf("  " C_RED "[FAIL]" C_RST " %s  " C_DIM "(test_todos.cpp:%d)" C_RST "\n", name, line);
    P(ok);
}
static void cki_(const char* name, int expect, int actual, int line) {
    bool ok = (expect == actual);
    if (ok) printf("  " C_GRN "[PASS]" C_RST " %s\n", name);
    else    printf("  " C_RED "[FAIL]" C_RST " %s (expected %d, got %d) " C_DIM "(test_todos.cpp:%d)" C_RST "\n", name, expect, actual, line);
    P(ok);
}
#define ck(name, ok)      ck_(name, ok, __LINE__)
#define cki(name, e, a)   cki_(name, e, a, __LINE__)
static void beginSec(const char* t) { g_secPass = g_secFail = 0; printf("\n" C_DIM "%s" C_RST "\n", t); }
static void endSec() {
    const char* c = (g_secFail == 0) ? C_GRN : C_RED;
    printf("   %s-- group passed %d / %d --" C_RST "\n", c, g_secPass, g_secPass + g_secFail);
}
static void resetAll() {
    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
    for (int i = 0; i < MAX_ENEMIES; i++) enemies[i] = { { 0,0,0,0 }, EnemyKind::Grunt, 0, false };
    player = { { 600, 350, 40, 40 }, 5.0f, 100 };
    score = 0; spawnTimer = 0; gameOver = false;
}

// ==================== ② 配置表 ConfigOf ====================
static void test_Config() {
    beginSec("[2] Config table -- four enemy kinds' stats");
    EnemyConfig g = ConfigOf(EnemyKind::Grunt),  r = ConfigOf(EnemyKind::Runner);
    EnemyConfig h = ConfigOf(EnemyKind::Heavy),  e = ConfigOf(EnemyKind::Elite);
    ck("Runner faster than Grunt (speed)", r.speed > g.speed);
    ck("Heavy slower than Grunt (speed)", h.speed < g.speed);
    ck("Heavy hp > 1 (takes several hits)", h.maxHp > 1);
    ck("Elite has the highest hp", e.maxHp > h.maxHp && e.maxHp > r.maxHp && e.maxHp > g.maxHp);
    ck("Elite gives the highest score", e.score > h.score && e.score > r.score && e.score > g.score);
    ck("All four sprites differ (no wrong/missing sprite)",
       g.sprite != r.sprite && g.sprite != h.sprite && g.sprite != e.sprite &&
       r.sprite != h.sprite && r.sprite != e.sprite && h.sprite != e.sprite);
    endSec();
}

// ==================== ③ SpawnEnemies：种类 + 满血 ====================
static void test_Spawn() {
    beginSec("[3] Spawn -- remember kind + start at full HP");
    resetAll();
    int kindSeen[4] = {0,0,0,0}, spawned = 0; bool hpOK = true;
    for (int t = 0; t < 8; t++) { spawnTimer = 54; SpawnEnemies(); }   // 逼出 8 只
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;
        spawned++;
        kindSeen[(int)enemies[i].kind]++;
        if (enemies[i].hp != ConfigOf(enemies[i].kind).maxHp) hpOK = false;   // 忘设 hp 会挂
    }
    ck("Enemies actually spawned", spawned > 0);
    ck("Each spawns at full HP (hp == kind's maxHp)", hpOK);
    int kinds = (kindSeen[0]>0) + (kindSeen[1]>0) + (kindSeen[2]>0) + (kindSeen[3]>0);
    ck("Spawns rotate through kinds (kind is stored)", kinds >= 3);
    endSec();
}

// ==================== ④ UpdateEnemies：速度按种类 ====================
static void test_Speed() {
    beginSec("[4] Chase -- speed looked up by kind");
    resetAll();
    player.rect = { 900, 350, 40, 40 };
    enemies[0] = { { 100, 350, ConfigOf(EnemyKind::Runner).size, ConfigOf(EnemyKind::Runner).size }, EnemyKind::Runner, 1, true };
    enemies[1] = { { 100, 350, ConfigOf(EnemyKind::Heavy ).size, ConfigOf(EnemyKind::Heavy ).size }, EnemyKind::Heavy,  3, true };
    float rx = enemies[0].rect.x, hx = enemies[1].rect.x;
    UpdateEnemies();
    float rStep = enemies[0].rect.x - rx, hStep = enemies[1].rect.x - hx;
    ck("Enemy moved toward the player", rStep > 0 && hStep > 0);
    ck("Runner steps more than Heavy per frame (speed by kind)", rStep > hStep);
    ck("Step length equals the configured speed", fabsf(rStep - ConfigOf(EnemyKind::Runner).speed) < 1e-4f);
    endSec();
}
// ==================== ⑥ HandleHits：扣血 + 按种类给分 ====================
static void test_Hits() {
    beginSec("[6] Hit -- lose HP + score by kind");
    resetAll();
    enemies[0] = { { 300, 300, 48, 48 }, EnemyKind::Heavy, ConfigOf(EnemyKind::Heavy).maxHp, true };
    bullets[0] = { { 305, 305, 10, 10 }, 0, 0, true };
    HandleHits();
    ck ("Heavy survives 1 hit (hp 3->2)", enemies[0].active && enemies[0].hp == 2);
    cki("No score before it dies", 0, score);
    bullets[0] = { { 305, 305, 10, 10 }, 0, 0, true }; HandleHits();   // 2->1
    bullets[0] = { { 305, 305, 10, 10 }, 0, 0, true }; HandleHits();   // 1->0 死
    ck ("Heavy falls only on the 3rd hit", !enemies[0].active);
    cki("Heavy gives 30 (score by kind)", 30, score);

    resetAll();
    enemies[0] = { { 300, 300, 40, 40 }, EnemyKind::Grunt, ConfigOf(EnemyKind::Grunt).maxHp, true };
    bullets[0] = { { 305, 305, 10, 10 }, 0, 0, true };
    HandleHits();
    ck ("Grunt dies in 1 hit", !enemies[0].active);
    cki("Grunt gives 10", 10, score);

    resetAll();   // 幽灵敌人（已死）不该得分
    enemies[0] = { { 400, 400, 40, 40 }, EnemyKind::Grunt, 1, false };
    bullets[0] = { { 405, 405, 10, 10 }, 0, 0, true };
    HandleHits();
    cki("Hitting a dead enemy scores nothing", 0, score);
    ck ("Bullet not consumed by a ghost enemy", bullets[0].active);

    resetAll();   // 不相撞
    enemies[0] = { { 0, 0, 40, 40 }, EnemyKind::Elite, 6, true };
    bullets[0] = { { 800, 600, 10, 10 }, 0, 0, true };
    HandleHits();
    cki("No hit -> no score", 0, score);
    ck ("No hit -> Elite still full HP", enemies[0].hp == 6 && enemies[0].active);
    endSec();
}

int main() {
    initConsole();
    printf(C_DIM "==== Lesson 2 autograder: config / spawn / speed / hits ====" C_RST "\n");
    test_Config();
    test_Spawn();
    test_Speed();
    test_Hits();
    if (g_fail == 0)
        printf("\n" C_GRN "==== Total: %d passed -- all green, nice! ====" C_RST "\n", g_pass);
    else
        printf("\n==== Total: " C_GRN "%d" C_RST " passed, " C_RED "%d" C_RST " failed ====\n"
               "Read the red " C_RED "[FAIL]" C_RST " lines (expected/got) and line numbers; visual TODOs (1)(5) are checked by eye.\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
