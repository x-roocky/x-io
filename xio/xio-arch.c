/* Linux/Unix系统默认输入、输出接口 */
#if defined(linux) || defined(unix)

#include <termio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

/* 设置度函数是否阻塞 */
#define XGETC_NOBLOCK   1

/* 保存stdin原有设置 */
static struct {
    int inited;
    struct termios tcattr;
#if XGETC_NOBLOCK == 1
    int fflag;
#endif
} stdin_store = {0};

/* 恢复stdin原有设置(仅退出时调用) */
static void stdin_restore(void) {
    if (stdin_store.inited) {
#if XGETC_NOBLOCK == 1
        (void)fcntl(STDIN_FILENO, F_SETFL, stdin_store.fflag);
#endif
        (void)tcsetattr(STDIN_FILENO, TCSANOW, &stdin_store.tcattr);
        stdin_store.inited = !(stdin_store.inited);
    }
}

/* 设置stdin为无回显/单字符返回/不阻塞 */
static void stdin_set(void) {
    /* 只初始化一次 */
    if (stdin_store.inited)
        return;
    /* 无回显/单字符返回 */
    struct termios tset;
    (void)tcgetattr(STDIN_FILENO, &(stdin_store.tcattr));
    tset = stdin_store.tcattr;
    tset.c_lflag &= ~(ECHO | ICANON);
    tset.c_cc[VMIN] = 1;
    tset.c_cc[VTIME] = 0;
    (void)tcsetattr(STDIN_FILENO, TCSANOW, &tset);
    /* 非阻塞读取 */
#if XGETC_NOBLOCK == 1
    stdin_store.fflag = fcntl(STDIN_FILENO, F_GETFL, 0);
    (void)fcntl(STDIN_FILENO, F_SETFL, stdin_store.fflag | O_NONBLOCK);
#endif
    /* 退出时恢复原设置 */
    stdin_store.inited = !(stdin_store.inited);
    atexit(stdin_restore);
}

/* 从控制台读入单字符，无回显/无死阻塞/单字符返回 */
int xgetc() {
    int c;
    stdin_set();
    if ((c = getchar()) < 0)
        usleep(50 * 1000); /* 非阻塞时避免空循环 */
    return c;
}

/* 向控制台输出单字符 */
int xputc(int c) {
    return fputc(c, stdout);
}


/* Windows系统默认输入、输出接口 */
#elif defined(WIN32)

#include <stdio.h>
#include <conio.h>

/* 单字符输出接口 */
int xputc(int c) {
    return putchar(c);
}

/* 单字符读入接口(要求无回显，无需回车立即返回) */
int xgetc(void) {
    return getch();
}

/* ESP32平台默认输入、输出接口 */
#elif defined(__xtensa__)

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* 单字符输出接口 */
int xputc(int c) {
    return fputc(c, stderr);
}

/* 单字符读入接口(要求无回显， 无需回车立即返回) */
int xgetc(void) {
    int c;
    if ((c = getchar()) < 0)
        vTaskDelay(((50 / portTICK_PERIOD_MS) > 0) ? (50 /portTICK_PERIOD_MS) : 1);
    return c;
}

#else

#error  "当前架构未定义单字节输入、输出接口，请定义再使用"

int xputc(int c) {
    return EOF;
}

int xgetc(void) {
    return EOF;
}

#endif /* defined(linux) || defined(unix) */