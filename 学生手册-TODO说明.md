# 学生手册：完成 4 个 TODO

你的任务是在 `src/main.cpp` 中补完 **4 个带 `★TODO` 的函数**。

已经写好的：

- `UpdatePlayer`
- `SpawnEnemies`

就是你的参考答案。

遇到不会写的地方，先看它们是怎么：

- `for` 循环遍历数组
- 用 `if` 判断条件
- 用 `continue` 跳过无效对象
- 修改 `rect.x / rect.y`
- 找到空位后 `break`

## 一个重要规则

**一次只完成一个 TODO。**

建议按照下面的节奏：

> 写一个 → 编译 → 测试 → 确认通过 → 再写下一个

不要一次把 4 个函数全部写完。

---

# 一、怎么编译和测试

打开 **MSYS2 UCRT64 / MINGW64** 终端，进入工程根目录 `game/`。

```bash
make
```

编译游戏：

```text
build/game.exe
```

运行游戏：

```bash
make run
```

运行自动测试：

```bash
make test
```

清理编译结果：

```bash
make clean
```

`make test` 不会打开游戏窗口。

它会自动准备测试场景，然后调用你写的函数，并告诉你：

```text
通过
未通过
```

全部通过，就说明主要逻辑写对了。

这类工具叫：

**Autograder（自动评分器）**

大学里的编程作业也经常这样测试。

---

# 二、推荐完成顺序

按照从简单到复杂完成：

1. `UpdateBullets`
2. `FireBullets`
3. `HandleHits`
4. `UpdateEnemies`

完成过程大概会变成：

```text
① 子弹会飞
      ↓
② 玩家能射击
      ↓
③ 子弹能消灭敌人
      ↓
④ 敌人会追玩家
      ↓
游戏基本完成
```

---

# TODO ① UpdateBullets

## 目标

让子弹：

```text
飞行 → 飞出屏幕 → 自动回收
```

---

## 第一步：遍历所有子弹

你需要用 `for`：

```cpp
for (...) {
    ...
}
```

范围：

```text
0 ~ MAX_BULLETS - 1
```

也就是检查子弹池里的每一颗子弹。

---

## 第二步：跳过没有使用的子弹

每颗子弹都有：

```cpp
active
```

如果：

```cpp
active == false
```

说明这个位置现在没有真正的子弹。

所以可以：

```cpp
if (!bullets[i].active)
    continue;
```

`continue` 的意思是：

> 当前这一轮不用继续执行了，直接检查下一颗子弹。

---

## 第三步：让子弹移动

一颗子弹有：

```cpp
vx
vy
```

它们表示：

```text
vx = x 方向速度
vy = y 方向速度
```

所以每一帧：

```text
新的 x = 原来的 x + vx
新的 y = 原来的 y + vy
```

想一想应该修改：

```cpp
bullets[i].rect.x
bullets[i].rect.y
```

---

## 第四步：检查是否飞出屏幕

屏幕大小：

```cpp
SCREEN_W
SCREEN_H
```

如果子弹：

```text
跑到左边
跑到右边
跑到上边
跑到下边
```

任意一种情况发生，就说明它已经飞出屏幕。

这时：

```cpp
active = false;
```

相当于把这颗子弹放回“子弹池”。

---

## 完成标准

运行：

```bash
make test
```

确认 `UpdateBullets` 相关测试通过。

### 思考题

为什么飞出屏幕后不直接 `delete` 子弹，而只是：

```cpp
active = false;
```

---

# TODO ② FireBullets

## 目标

按方向键时发射一颗子弹。

```text
↑ 向上
↓ 向下
← 向左
→ 向右
```

---

## 第一步：确定射击方向

先准备：

```text
vx
vy
```

根据按键决定速度。

例如：

```text
右：vx = 8
左：vx = -8
上：vy = -8
下：vy = 8
```

这里使用：

```cpp
IsKeyPressed(...)
```

例如：

```cpp
IsKeyPressed(KEY_RIGHT)
```

注意两个函数的区别：

```cpp
IsKeyPressed()
```

按下一次，只触发一次。

```cpp
IsKeyDown()
```

只要一直按着，就会连续触发。

这里应该使用 **`IsKeyPressed`**。

---

## 第二步：找一颗空闲子弹

子弹数量是有限的。

所以不要创建新的子弹，而是去：

```cpp
bullets[]
```

里面寻找：

```cpp
active == false
```

的子弹。

这一部分可以直接参考：

```cpp
SpawnEnemies
```

看看它是怎么：

```text
遍历数组
→ 找空位置
→ 初始化
→ active = true
→ break
```

---

## 第三步：设置子弹初始位置

让子弹从玩家附近出现：

```cpp
{
    player.rect.x + 12,
    player.rect.y + 12,
    8,
    8
}
```

然后设置：

```text
vx
vy
active
```

---

## 最容易犯的错误

找到一颗空闲子弹之后，一定要：

```cpp
break;
```

否则一次按键可能会把：

```text
第 1 颗
第 2 颗
第 3 颗
……
```

全部一起发射出去。

---

## 完成标准

运行：

```bash
make run
```

实际测试：

```text
↑ ↓ ← →
```

确认四个方向都能发射。

---

# TODO ③ HandleHits

## 目标

实现：

```text
子弹 + 敌人
     ↓
发生碰撞
     ↓
子弹消失
敌人消失
分数 +10
```

---

## 第一步：遍历子弹

外层使用一个 `for`：

```text
遍历所有 bullets
```

没有激活的子弹：

```cpp
continue;
```

---

## 第二步：遍历敌人

在子弹循环里面，再写一个 `for`：

```text
遍历所有 enemies
```

所以结构大概是：

```text
for 每一颗子弹
    for 每一个敌人
```

这叫：

**Nested Loop（嵌套循环）**

也就是“两层 `for`”。

---

## 第三步：判断碰撞

raylib 已经提供好了矩形碰撞函数：

```cpp
CheckCollisionRecs(...)
```

你只需要把：

```text
子弹的 rect
敌人的 rect
```

传进去。

不需要自己计算坐标重叠。

---

## 第四步：处理命中

如果碰撞成功：

```text
子弹消失
敌人消失
score + 10
```

也就是修改：

```text
bullet.active
enemy.active
score
```

然后：

```cpp
break;
```

为什么？

因为这颗子弹已经消失了，没有必要继续检查其他敌人。

---

## 完成标准

运行：

```bash
make test
```

然后：

```bash
make run
```

确认：

```text
射击敌人
↓
敌人消失
↓
分数增加 10
```

---

# TODO ④ UpdateEnemies

## 目标

让敌人：

```text
追玩家
↓
碰到玩家
↓
玩家掉血
↓
敌人消失
↓
HP <= 0
↓
Game Over
```

---

## 第一步：遍历敌人

和前面的代码一样：

```text
for 遍历所有敌人
```

如果：

```cpp
active == false
```

直接：

```cpp
continue;
```

---

## 第二步：让敌人靠近玩家

这里暂时不需要：

```text
勾股定理
三角函数
向量
开平方
```

只需要比较大小。

### X 方向

如果玩家在敌人右边：

```text
player.x > enemy.x
```

敌人的 `x` 增加：

```text
enemy.x += speed
```

如果玩家在左边：

```text
enemy.x -= speed
```

---

### Y 方向

同样：

```text
玩家在下面 → y 增加
玩家在上面 → y 减少
```

最终就是：

```text
比较 x
比较 y
```

让敌人一点一点靠近玩家。

---

## 第三步：检查敌人与玩家碰撞

继续使用：

```cpp
CheckCollisionRecs(...)
```

这次比较：

```text
enemy.rect
player.rect
```

---

## 第四步：扣血

碰到玩家以后：

```text
玩家 HP - 10
敌人消失
```

然后检查：

```cpp
player.hp <= 0
```

如果成立：

```cpp
gameOver = true;
```

---

## 完成标准

运行：

```bash
make run
```

观察：

```text
敌人会追你
敌人碰到你会消失
你的血量减少
血量归零后 Game Over
```

然后：

```bash
make test
```

确认自动测试通过。

---

# 最终验收

4 个 TODO 全部完成后，依次运行：

```bash
make clean
make
make test
make run
```

你应该能看到：

```text
✅ 玩家可以移动
✅ 玩家可以射击
✅ 子弹可以飞行
✅ 子弹飞出屏幕后被回收
✅ 子弹可以消灭敌人
✅ 消灭敌人获得 10 分
✅ 敌人会追玩家
✅ 敌人碰到玩家会扣血
✅ HP 归零后 Game Over
```

如果这些全部完成，这一关就通过了。

---

# Debug 小技巧

遇到测试失败，不要一次乱改很多地方。

按这个顺序检查：

```text
1. 哪一个测试失败？
        ↓
2. 是哪一个 TODO？
        ↓
3. for 循环范围对不对？
        ↓
4. active 判断对不对？
        ↓
5. rect.x / rect.y 改对了吗？
        ↓
6. 是否忘记 continue / break？
        ↓
7. 修改后重新 make test
```

程序员的大部分时间，并不是一次把代码写对。

而是：

```text
写一点
↓
测试
↓
发现问题
↓
修改
↓
再测试
```

这就是 **Debug（调试）**。

---

# 自动测试的一个限制

`FireBullets` 依赖真实键盘输入。

自动测试无法真正替你按：

```text
↑ ↓ ← →
```

所以自动测试主要检查：

> 没有按键的时候，程序不能自己乱生成子弹。

真正的射击效果，需要运行：

```bash
make run
```

然后手动测试。

其他三个 TODO：

```text
UpdateBullets
HandleHits
UpdateEnemies
```

都可以通过 `make test` 比较完整地检查。

---

# 今天真正要学会的东西

完成这 4 个 TODO，不只是为了把游戏做出来。

你实际上练习了：

```text
for 循环          → 遍历很多对象
if 判断           → 根据情况执行不同逻辑
continue          → 跳过当前对象
break             → 提前结束循环
数组               → 保存很多子弹和敌人
状态 active        → 管理对象是否正在使用
碰撞检测           → 判断两个游戏对象是否接触
函数               → 把不同游戏逻辑拆开
Debug              → 根据测试定位程序错误
```
