// ============================================================
//  第 2 课 · 自动评测：检查配置表相关的 4 个可测 TODO
//  跑法：make test        （测你的 src/main.cpp）
//        make test-sol    （老师验证参考答案）
//  可自动测：② 配置表数值、③ 刷怪种类+满血、④ 追人速度按种类、⑥ 扣血+按种类给分
//  画面类的 ①(玩家/子弹贴图) ⑤(敌人贴图) 靠眼睛看——跑起来四种怪长相不同即对。
//  输出用英文（ASCII）；颜色只在终端里才上，被重定向 / CI 抓走时自动变纯文本，
//  所以存文件、贴 Summary 都不会出现乱码。说明与手册的 TODO 序号一一对应。
//  原理：include src/main.cpp（UNIT_TEST 跳过它的 main），直接调用里面的函数。
// ============================================================
#include "raylib.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>

#ifndef SRC_MAIN
#define SRC_MAIN "../src/main.cpp"      // 默认测孩子的文件；make test-sol 会改指到参考答案
#endif
#define UNIT_TEST
#include SRC_MAIN

// 颜色只在「输出真的送到一个终端」时才发。被管道 / 重定向 / CI 抓走时退回纯 ASCII——
// 那些 \033[..m 在不是终端的地方就是乱码（CI 的 Summary 是个 Markdown 文本框，
// 里面的转义序列会糊成 "?[31m"，所以那边必须干净）。
// NO_COLOR 是通行约定：设了这个环境变量就一律不上色。
#ifdef _WIN32
extern "C" __declspec(dllimport) void* __stdcall GetStdHandle(unsigned long n);
extern "C" __declspec(dllimport) int   __stdcall GetConsoleMode(void* h, unsigned long* mode);
extern "C" __declspec(dllimport) int   __stdcall SetConsoleMode(void* h, unsigned long mode);
static bool consoleSupportsAnsi() {
    void* h = GetStdHandle((unsigned long)-11);
    unsigned long m = 0;
    if (!GetConsoleMode(h, &m)) return false;          // 句柄不是控制台（被重定向了）
    if (!SetConsoleMode(h, m | 0x0004)) return false;  // 这个控制台不吃 ANSI 转义
    return true;                                       // 两步都成，才敢发颜色
}
#else
#include <unistd.h>
static bool consoleSupportsAnsi() { return isatty(1) != 0; }
#endif

static const char* C_GRN = "";
static const char* C_RED = "";
static const char* C_DIM = "";
static const char* C_RST = "";
static void setColor(bool on) {
    C_GRN = on ? "\033[32m" : "";
    C_RED = on ? "\033[31m" : "";
    C_DIM = on ? "\033[2m" : "";
    C_RST = on ? "\033[0m" : "";
}

static int g_pass = 0, g_fail = 0, g_secPass = 0, g_secFail = 0;
static void P(bool ok) { if (ok) { g_pass++; g_secPass++; } else { g_fail++; g_secFail++; } }
static void ck_(const char* name, bool ok, int line) {
    if (ok) printf("  %s[PASS]%s %s\n", C_GRN, C_RST, name);
    else    printf("  %s[FAIL]%s %s  %s(test_todos.cpp:%d)%s\n", C_RED, C_RST, name, C_DIM, line, C_RST);
    P(ok);
}
static void cki_(const char* name, int expect, int actual, int line) {
    bool ok = (expect == actual);
    if (ok) printf("  %s[PASS]%s %s\n", C_GRN, C_RST, name);
    else    printf("  %s[FAIL]%s %s (expected %d, got %d) %s(test_todos.cpp:%d)%s\n",
                   C_RED, C_RST, name, expect, actual, C_DIM, line, C_RST);
    P(ok);
}
#define ck(name, ok)      ck_(name, ok, __LINE__)
#define cki(name, e, a)   cki_(name, e, a, __LINE__)
static void beginSec(const char* t) { g_secPass = g_secFail = 0; printf("\n%s%s%s\n", C_DIM, t, C_RST); }
static void endSec() {
    const char* c = (g_secFail == 0) ? C_GRN : C_RED;
    printf("   %s-- group passed %d / %d --%s\n", c, g_secPass, g_secPass + g_secFail, C_RST);
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
//  重要：血量/分数的具体数值由孩子自己定（手册只规定「关系」）。所以这一组
//  一律拿 ConfigOf(...) 读他自己的表来对照，**绝不写死 3 血 / 30 分**，
//  否则「按规则填对但换了个数」会被误判成错。数值间的关系由 [2] 组负责。

// 放一颗子弹贴在 j 号敌人正中，跑一次 HandleHits（= 打中一发）
static void hitOnce(int j) {
    bullets[0] = { { enemies[j].rect.x + enemies[j].rect.width  / 2 - 5,
                     enemies[j].rect.y + enemies[j].rect.height / 2 - 5, 10, 10 }, 0, 0, true };
    HandleHits();
}

static void test_Hits() {
    beginSec("[6] Hit -- lose HP + score by kind");

    // ---- 重甲：要打「它表里那么多血」下才倒，倒下时给「它表里那个分」----
    const EnemyConfig hc = ConfigOf(EnemyKind::Heavy);
    resetAll();
    enemies[0] = { { 300, 300, hc.size, hc.size }, EnemyKind::Heavy, hc.maxHp, true };
    hitOnce(0);                                          // 第 1 发
    cki("Heavy loses exactly 1 HP per hit", hc.maxHp - 1, enemies[0].hp);
    ck ("Heavy survives the 1st hit (needs maxHp > 1)", enemies[0].active);
    cki("No score before it dies", 0, score);

    bool aliveUntilLast = true;
    for (int n = 2; n < hc.maxHp; n++) {                  // 第 2 .. 第 (maxHp-1) 发
        hitOnce(0);
        if (!enemies[0].active) aliveUntilLast = false;   // 提前死 = 扣多了 / 判早了
    }
    ck ("Heavy stays alive while it still has HP left", aliveUntilLast);
    hitOnce(0);                                          // 第 maxHp 发 —— 这下该倒
    ck ("Heavy falls on the hit that empties its HP", !enemies[0].active);
    cki("Heavy's score comes from its own config row", hc.score, score);

    // ---- 1 血的怪：一发就倒，给的是它表里的分（证明加分是查表，不是写死 10）----
    const EnemyConfig gc = ConfigOf(EnemyKind::Grunt);
    resetAll();
    enemies[0] = { { 300, 300, gc.size, gc.size }, EnemyKind::Grunt, 1, true };
    hitOnce(0);
    ck ("A 1-HP enemy dies in one hit", !enemies[0].active);
    cki("Grunt's score comes from its own config row", gc.score, score);

    // ---- 精英：同一套规则也得成立（分数跟着种类走）----
    const EnemyConfig ec = ConfigOf(EnemyKind::Elite);
    resetAll();
    enemies[0] = { { 300, 300, ec.size, ec.size }, EnemyKind::Elite, ec.maxHp, true };
    for (int n = 0; n < ec.maxHp; n++) hitOnce(0);
    ck ("Elite falls after exactly maxHp hits", !enemies[0].active);
    cki("Elite's score comes from its own config row", ec.score, score);

    resetAll();   // 幽灵敌人（已死）不该得分
    enemies[0] = { { 400, 400, gc.size, gc.size }, EnemyKind::Grunt, 1, false };
    hitOnce(0);
    cki("Hitting a dead enemy scores nothing", 0, score);
    ck ("Bullet not consumed by a ghost enemy", bullets[0].active);

    resetAll();   // 不相撞
    enemies[0] = { { 0, 0, ec.size, ec.size }, EnemyKind::Elite, ec.maxHp, true };
    bullets[0] = { { 800, 600, 10, 10 }, 0, 0, true };
    HandleHits();
    cki("No hit -> no score", 0, score);
    ck ("No hit -> Elite still at full HP", enemies[0].hp == ec.maxHp && enemies[0].active);
    endSec();
}

int main() {
    setColor(consoleSupportsAnsi() && getenv("NO_COLOR") == nullptr);
    printf("%s==== Lesson 2 autograder: config / spawn / speed / hits ====%s\n", C_DIM, C_RST);
    test_Config();
    test_Spawn();
    test_Speed();
    test_Hits();
    if (g_fail == 0)
        printf("\n%s==== Total: %d passed -- all green, nice! ====%s\n", C_GRN, g_pass, C_RST);
    else
        printf("\n==== Total: %s%d%s passed, %s%d%s failed ====\n"
               "Read the red %s[FAIL]%s lines (expected/got) and line numbers; visual TODOs (1)(5) are checked by eye.\n",
               C_GRN, g_pass, C_RST, C_RED, g_fail, C_RST, C_RED, C_RST);
    return g_fail == 0 ? 0 : 1;
}
