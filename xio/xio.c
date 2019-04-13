#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <ctype.h>
#include <stdarg.h>
#include <errno.h>
#include "xio.h"

#define xassert(_expr)          assert((_expr))
#define XPRINTF_BUFFER_SIZE     512

/* 输出字符串(不同于标准puts，行尾不自动添加\n) */
int xputs(const char *str) {
    if (str) {
        const char *p = str;
        while (*p != '\0')
            (void)xputc(*p++);
        return (int)(p - str);
    } else {
        xassert(str);
        return EOF;
    }
}

/* 格式化输出，用法同printf */
int xprintf(const char *format, ...) {
    static char buffer[XPRINTF_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    (void)vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    return xputs(buffer);
}

/* 用16进制方式打印一行数据 */
static void dumpline(unsigned long address, const void *buffer, int count, int width, int columns) {
    int i;
    xassert(buffer);
    xassert(count > 0);
    xassert((width == 1) || (width == 2) || (width == 4));
    if (count > columns)
        count = columns;
    xprintf("%08lx ", address);
    if (width == 1) {
        const unsigned char *bytes = buffer;
        for (i = 0; i < count; ++i)        /* 16进制打印 */
            xprintf(" %02x", (unsigned int)(bytes[i]));
        for (; i < columns; ++i)           /* 对齐 */
            xputs("   ");
        xputs("  |");
        for (i = 0; i < count; ++i)        /* ASCII码打印 */
            xputc(isprint(bytes[i]) ? bytes[i] : '.');
        for (; i < columns; ++i)           /* 对齐 */
            xputs(" ");
        xputs("|");
    } else if (width == 2) {
        const unsigned short *words = buffer;
        for (i = 0; i < count; ++i)         /* 16进制打印 */
            xprintf(" %04x", (unsigned int)(words[i]));
    } else if (width == 4) {
        const uint32_t *dwords = buffer;
        for (i = 0; i < count; ++i)         /* 16进制打印 */
            xprintf(" %08lx", (unsigned long)(dwords[i]));
    }
    xputc('\n');
}

/* 用十六进制编码来查看数据内容 */
int xhexdump(unsigned long address, /* 行首显示地址值的起始（仅用于指示，不非与实际buffer地址相同） */
             const void *buffer,    /* 要查看的数据指针 */
             int count,             /* 要查看的条目总数 */
             int width) {           /* 每个条目的字节长度(1字节/2字节/4字节) */
    if (buffer && (count >= 0) && ((width == 1) || (width == 2) || (width == 4))) {
        int columns = (width < 4) ? 16 : 4; /* 1-2字节每行16列，4字节每行8列 */
        for (int i = 0; i < count; i += columns)
            dumpline(address + i, buffer + i * width, count - i, width, columns);
        return count;
    } else {
        xassert(buffer);
        xassert(count >= 0);
        xassert((width == 1) || (width == 2) || (width == 4));
        return 0;
    }
}

/* 读入字符串(大体同标准gets，额外添加size参数避免缓冲区溢出) */
char *xgets(char *buffer, int size) {
    if (!buffer || (size <= 0)) {
        xassert(buffer);
        xassert(size > 0);
        return NULL;
    }

    /* 按照判断条件，逐字符读取 */
    int count = 0;
    for (;;) {
        int c = xgetc();
        if ((c == '\r') || (c == '\n')) { /* 遇结束符返回 */
            buffer[count] = '\0';
            (void)xputs("\r\n");
            return buffer;
        } else if (((c == 0x08) || (c == 0x7f)) && (count > 0)) { /* 删除键，置空退格一个字符 */
            static const char erase_seq[] = "\b \b"; /* erase sequence */
            buffer[--count] = '\0';
            (void)xputs(erase_seq);
        } else if ((count < (size - 1)) && isprint(c)) { /* 读入字符 */
            buffer[count] = (char)c;
            buffer[++count] = '\0';
            (void)xputc(c);
        } else { /* 读入缓冲已满 */
            /* 丢弃 */
        }
    }
}

/* 暂停当前进程，按任意键继续 */
int xpause(const char *prompt) {
    char key;
    xputs(prompt);
    return (xgets(&key, sizeof(key)) != NULL) ? 0 : -1;
}

/*  把字符串转换成长整型，支持2进制(0b/0B开头)、10进制(纯数字)、16进制(0x/0X开头)
 *  若转换发生错误，*err为错误码，成功*err为0 */
long xatol(const char *str, int *err) {
    /* 确认参数有效性 */
    if (!err) {
        xassert(err);
        return 0;
    }
    if (!str) {
        xassert(str);
        *err = EINVAL;
        return 0;
    }

    /* 分析字符格式采用二进制/十进制/十六进制转换 */
    size_t count = strlen(str);
    char *endptr = NULL;
    long result;
    if (count <= 2) /* 两字符内一律按十进制转换 */
        result = strtol(str, &endptr, 10);
    else if ((str[0] == '0') && (tolower(str[1]) == 'x')) /* 0x/0X开头一律按十六进制转换 */
        result = strtol(str + 2, &endptr, 16);
    else if ((str[0] == '0') && (tolower(str[1]) == 'b')) /* 0b/0B开头一律按十六进制转换 */
        result = strtol(str + 2, &endptr, 2);
    else /* 其它情况也一并按十进制转换 */
        result = strtol(str, &endptr, 10);

    /* 确认转换结果，失败返回相应指示 */
    if (errno == ERANGE) /* 溢出 */
        *err = ERANGE;
    else if ((*endptr != '\0') || (count == 0)) /* 非法字符串 */
        *err = EINVAL;
    else /* 成功 */
        *err = 0;
    return result;
}

/*  把字符串转换成长整型，支持2进制(0b/0B开头)、10进制(纯数字)、16进制(0x/0X开头)
 *  若转换发生错误，*err为错误码，成功*err为0 */
unsigned long xatoul(const char *str, int *err) {
    /* 确认参数有效性 */
    if (!err) {
        xassert(err);
        return 0;
    }
    if (!str) {
        xassert(str);
        *err = EINVAL;
        return 0;
    }

    /* 分析字符格式采用二进制/十进制/十六进制转换 */
    size_t count = strlen(str);
    char *endptr = NULL;
    unsigned long result;
    if (count <= 2) /* 两字符内一律按十进制转换 */
        result = strtoul(str, &endptr, 10);
    else if ((str[0] == '0') && (tolower(str[1]) == 'x')) /* 0x/0X开头一律按十六进制转换 */
        result = strtoul(str + 2, &endptr, 16);
    else if ((str[0] == '0') && (tolower(str[1]) == 'b')) /* 0b/0B开头一律按十六进制转换 */
        result = strtoul(str + 2, &endptr, 2);
    else /* 其它情况也一并按十进制转换 */
        result = strtoul(str, &endptr, 10);

    /* 确认转换结果，失败返回相应指示 */
    if (errno == ERANGE) /* 溢出 */
        *err = ERANGE;
    else if ((*endptr != '\0') || (count == 0)) /* 非法字符串 */
        *err = EINVAL;
    else if (str[0] == '-') /* 不支持负数 */
        *err = ERANGE;
    else /* 成功 */
        *err = 0;
    return result;
}

/* 把一个字符串分割成字符串数组 */
int xsplit(char *str, const char *delim, int argc_max, char *argv[]) {
    if (!str || !delim || (argc_max <= 0) || !argv) {
        xassert(str);
        xassert(delim);
        xassert(argc_max > 0);
        xassert(argv);
        return -EINVAL;
    }

    int count = 0;
    while (count < argc_max) {
        char *s = strtok(str, delim);
        if (!s)
            break;
        argv[count++] = s;
        str = NULL;
    }

    return count;
}