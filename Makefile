# コンパイラの設定
CC			= gcc
CFLAGS		= -O3
CFLAGS_G	= -g
LIBS		= -lm
TARGET		= mahjongHandsCalc
SRCS		= $(wildcard src/*.c)
OBJS		= $(SRCS:src/%.c=obj/%.o)
OBJS_G		= $(SRCS:src/%.c=obj/%.dbg.o)

# 実行
run: $(TARGET).exe
	./$(TARGET).exe

test: $(TARGET)_debug.exe
	gdb ./$(TARGET)_debug.exe

# コンパイル
$(TARGET).exe: $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) $(LIBS) -o $(TARGET).exe

$(TARGET)_debug.exe: $(OBJS_G)
	$(CC) $(OBJS_G) $(CFLAGS_G) $(LIBS) -o $(TARGET)_debug.exe

# オブジェクトファイルの生成
obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

obj/%.dbg.o: src/%.c
	$(CC) $(CFLAGS_G) -c $< -o $@

# クリーン
clean:
	rm -f $(OBJS) $(OBJS_G) $(TARGET).exe $(TARGET)_debug.exe