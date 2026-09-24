# 迷你肉鸽（raylib + C++）

一个俯视角小游戏的**脚手架**：移动、四方向射击、刷怪、追击都已搭好框架，
你只要填 4 个函数，它就会从半成品长成一个能玩的游戏。

## 跑起来

在 **MSYS2 终端**（标题带 UCRT64 或 MINGW64）里，进到本目录，然后：

```bash
make run
```

第一次若提示缺 raylib，先装一次（子系统要和终端一致，下面按 UCRT64）：

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-raylib
```

跑起来后：**WASD** 移动，**方向键**射击，死了按 **Enter** 重开。

## 你要做的

打开 `src/main.cpp`，把 4 个 `★TODO` 填上，顺序由易到难：

| 顺序 | 函数 | 填完能看到 |
|---|---|---|
| ① | `UpdateBullets` | 子弹会飞、出界回收 |
| ② | `FireBullets`   | 能射击 |
| ③ | `HandleHits`    | 能打死敌人、加分 |
| ④ | `UpdateEnemies` | 敌人追人、能致死 —— 成型 |

照已写好的 `UpdatePlayer`、`SpawnEnemies` 的写法来写。详细提示见 **`学生手册-TODO说明.md`**。

每填一个就自查：

```bash
make test      # 逐条报「通过 / 未通过」，全绿即对
```

## 命令一览

```bash
make        # 只编译       → build/game.exe
make run    # 编译并运行
make test   # 编译并跑自动评测
make clean  # 清理产物
```

## 目录

```
src/main.cpp            游戏源码（含 4 个 TODO）
tests/test_todos.cpp    自动评测
学生手册-TODO说明.md     每个 TODO 的详细提示
Makefile                构建脚本
```

> 环境：Windows + MSYS2。编译器（g++）和 raylib 由 MSYS2 提供，不随本仓库分发。
