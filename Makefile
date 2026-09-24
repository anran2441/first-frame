# ============================================================
#  Makefile —— 一条命令搞定 编译 / 运行 / 测试
# ------------------------------------------------------------
#  在 MSYS2 终端（标题带 UCRT64 或 MINGW64）、工程根目录 game/ 下：
#      make          只编译游戏      → build/game.exe
#      make run      编译并运行游戏
#      make test     编译并运行自动评测（检查 4 个 TODO 写得对不对）
#      make clean    清掉 build/ 里的产物
# ============================================================

CXX      := g++
CXXFLAGS := -std=c++17

# ---- 跨平台：Windows(MSYS2) 与 Linux 的差异自动区分 ----
ifeq ($(OS),Windows_NT)
    EXE       := .exe
    GAMELIBS  := -lraylib -lopengl32 -lgdi32 -lwinmm
    WINFLAG   := -mwindows          # 游戏窗口不额外弹黑色控制台
else
    EXE       :=
    GAMELIBS  := -lraylib -lm
    WINFLAG   :=
endif

GAME := build/game$(EXE)
TEST := build/test_todos$(EXE)

# 默认目标：编译游戏
all: $(GAME)

# 游戏：改了 src/main.cpp 就重新编译
$(GAME): src/main.cpp | build
	$(CXX) $(CXXFLAGS) src/main.cpp -o $(GAME) $(WINFLAG) $(GAMELIBS)

# 编译并运行游戏
run: $(GAME)
	./$(GAME)

# 自动评测：改了 main.cpp 或测试文件都会重新编译（测试不加 -mwindows，要看输出）
$(TEST): tests/test_todos.cpp src/main.cpp | build
	$(CXX) $(CXXFLAGS) tests/test_todos.cpp -o $(TEST) $(GAMELIBS)

test: $(TEST)
	@./$(TEST) || true

build:
	mkdir -p build

clean:
	rm -f $(GAME) $(TEST)

.PHONY: all run test clean
