#ifndef MY_CASES_H
#define MY_CASES_H
#include "raylib.h"
struct CollisionCase {
    const char* name;
    Rectangle rect;
    bool expected;
    bool enabled;
};
// L3 index: 10 myCase.
// TODO(L3-10): Choose a world rectangle, predict HitsWall, then enable and explain your case.
// Prerequisite: task 06. Goal: test an edge case not covered by the two built-in cases.
inline CollisionCase myCase = {"My case", {0, 0, 0, 0}, false, false};
#endif
