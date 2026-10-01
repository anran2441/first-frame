#ifndef MY_CASES_H
#define MY_CASES_H
#include "raylib.h"
struct CollisionCase {
    const char* name;
    Rectangle rect;
    bool expected;
    bool enabled;
};
// L3 索引：10 myCase。
// TODO(L3-10): 选择一个世界坐标矩形，预测 HitsWall 结果，再启用并解释你的案例。
// 前置：任务 06。目标：测试两个内置案例未覆盖的边界情况。
inline CollisionCase myCase = {"My case", {0, 0, 0, 0}, false, false};
#endif
