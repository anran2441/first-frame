# 学生手册：完成 6 个 TODO（第 2 课「怪物图鉴」）

你的任务是在 `src/main.cpp` 中补完 **6 个带 `★TODO` 的位置**。

这一课的核心新本领只有两个：

- **类型（`enum class`）**：给每种怪起个名字（普通 / 快速 / 重甲 / 精英）。
- **配置表（数据驱动）**：把每种怪的数值写进一张表，想改强弱只改表，逻辑不动。

已经写好、可当范例参考的：

- `UpdatePlayer`（移动）
- `NextKind`（出怪轮换）
- `SpawnEnemies` 里「找空位、取种类、查配置」的部分
- `ConfigOf` 里 **普通怪 Grunt 那一行**（照它补另外三行）

## 一个重要规则

**一次只完成一个 TODO。** 按下面的节奏：

> 写一个 → `make` → 试玩 / `make test` → 确认通过 → 再写下一个

顺着 ①→⑥ 做最顺，因为后面几步都要用到 ② 那张配置表。

---

# 一、怎么编译和测试

打开 **MSYS2 UCRT64 / MINGW64** 终端，进入工程根目录 `game/`。

```bash
make        # 编译游戏 → build/game.exe
make run    # 编译并运行游戏（试玩，看贴图和四种怪）
make test   # 运行自动评测，不开窗口，只报告通过 / 未通过
make clean  # 清掉 build/ 里的产物
```

`make test` 是一个 **Autograder（自动评分器）**：它自动准备场景、调用你写的函数，
再告诉你每一项「通过 / 未通过」。它能自动测 **②③④⑥**；画面类的 **①⑤**（贴图）
测不了，要靠 `make run` 用眼睛看。

第一次若提示缺 raylib，先装一次：

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-raylib
```

---

# 二、推荐完成顺序

```text
① 玩家/子弹换贴图
      ↓
② 填全配置表（核心）
      ↓
③ 刷怪记种类 + 满血
      ↓
④ 速度按种类查表
      ↓
⑤ 敌人换各自贴图
      ↓
⑥ 命中扣血 + 按种类给分
      ↓
四种怪各有脾气，游戏成型
```
---

# TODO ① 玩家 / 子弹换贴图（在 `DrawGame`）

把画方块换成画贴图。贴图放在 `assets/`，用一句话画：

```cpp
DrawSprite(SpriteId::Player, player.rect);      // 玩家
DrawSprite(SpriteId::Bullet, bullets[i].rect);  // 子弹（在 for 里）
```

`SpriteId` 是「贴图名字表」，读图、缩放这些细节都藏在 `src/assets.h`，你只管调用。
看到**品红色方块** = 贴图没加载成功（名字拼错或图缺失）。这步靠 `make run` 用眼睛验。

---

# TODO ② 填全配置表（在 `ConfigOf`）—— 本课核心

`enum class EnemyKind` 给四种怪起了名字；`EnemyConfig` 是「一行数据」：
`{ 贴图, 速度, 满血, 分数, 大小 }`。`ConfigOf(kind)` 就是**查表**——给种类返回那一行。

普通怪 Grunt 已给作范例，照它补三行，满足这些**关系**（具体数字自己定）：

```text
快速 Runner：比普通快、脆(1 血)、分数略高
重甲 Heavy ：比普通慢、肉(血量 > 1，建议 3)、分数更高
精英 Elite ：血量最高(建议 6)、够猛、分数最高
```

每种怪的 `sprite` 别填错、别填重（Runner→Runner、Heavy→Heavy、Elite→Elite）。
常见坑：某个 `case` 忘了 `return`；或复制粘贴后 `sprite` 没改。`make test` 的
「② 配置表」组会检查快慢、血量、分数关系，以及四种贴图各不相同。

---

# TODO ③ 刷怪：记种类 + 满血（在 `SpawnEnemies`）

取种类、查配置老师已写好（`k` 和 `cfg`）。你把占位的两行改成真正存进这只怪：

```cpp
enemies[i].kind = k;         // 记住这次是哪种（原来写死 Grunt）
enemies[i].hp   = cfg.maxHp; // 满血开场（原来写死 1）
```

不记 `kind`，后面 ④⑤⑥ 全乱；不设满 `hp`，重甲精英开场就 1 血。
测：「③ 刷怪」组——会轮换出多种、每只 `hp == 该种 maxHp`。
---

# TODO ④ 速度按种类查表（在 `UpdateEnemies`）

把写死的速度换成查表：

```cpp
float sp = ConfigOf(enemies[i].kind).speed;  // 原来是 float sp = 1.5f;
```

下面追人的四个 `if` 一个字都不用改——这就是**数据驱动**：逻辑不变，行为随表而变。
测：`[4] Chase` 组——快速一帧走得比重甲多、步长正好等于表里的速度。

---

# TODO ⑤ 敌人换各自贴图（在 `DrawGame`）

把画敌人的红方块，换成「按种类查出的贴图」：

```cpp
DrawSprite(ConfigOf(enemies[i].kind).sprite, enemies[i].rect);
```

这步把 ① 的 `DrawSprite` 和 ② 的查表合起来用。四种怪长得一样，多半是这步没改、
或 ② 里 `sprite` 填重了。靠 `make run` 用眼睛验。

---

# TODO ⑥ 命中：扣血 + 按种类给分（在 `HandleHits`）

把「一枪一个、都给 10 分」改成「扣血才死、按种类给分」：

```cpp
enemies[j].hp -= 1;                            // 先扣 1 血
if (enemies[j].hp <= 0) {                      // 血空了才死
    enemies[j].active = false;
    score += ConfigOf(enemies[j].kind).score;  // 按“它是哪种”给分
}
```

子弹那句 `bullets[i].active = false;` 保持不动。这样重甲(3 血)要打 3 下、精英(6 血)
要打 6 下、普通(1 血)一下就倒。常见坑：忘了先做 ③(hp 没设满，重甲开场就 1 血)；
或加分还写死 `+10` 没查表。测：`[6] Hit` 组。

---

# 最终验收

6 个 TODO 完成后，从干净状态跑一遍：

```bash
make clean && make && make test && make run
```

你应该看到：

```text
✅ 玩家、子弹是贴图        ✅ 四种怪长相各不相同
✅ 快速怪明显更快          ✅ 重甲慢吞吞、要连打几下才倒
✅ 精英最肉、给分最多      ✅ 杀不同的怪加不同的分
```

判定：`make test` 里 `[2] [3] [4] [6]` 全绿（`[PASS]`）+ 眼睛看 ①⑤ 对。

> **关于评测输出**：`make test` 的结果是**英文**（`[PASS]` / `[FAIL]` + 组名 `[2] [3] [4] [6]`），
> 这样在任何终端里都不会乱码。组号 `[2]…[6]` 就对应上面 TODO 的 ②…⑥。

---

# Debug 小技巧

测试变红别急，按顺序查：

```text
1. 哪一条 [FAIL]？读它的英文说明 + (expected/got)
        ↓
2. 属于 [2]/[3]/[4]/[6] 哪组？就只改那个 TODO
        ↓
3. 是不是漏做了前置？②→③→④/⑥ 有依赖（种类、满血、查表）
        ↓
4. 改完 make test 再看一遍
```

程序员的大部分时间不是一次写对，而是「写一点 → 测试 → 发现问题 → 修改 → 再测试」，
这就是 **Debug（调试）**。

---

# 今天真正学会的东西

```text
enum class     → 给东西分类型（普通/快速/重甲/精英）
struct         → 把一组数据打包成「一行」
配置表 / 查表   → 数据驱动：强弱写在表里，想改平衡只改表
switch / case  → 按种类返回不同结果
DrawSprite     → 用贴图作画（细节藏在门面 assets.h）
Debug          → 根据测试定位错误
```

一句能受用很久的话：**想改平衡，只改一张表，逻辑一行不动。** 后面几课的道具、掉落都会复用它。
