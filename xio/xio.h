#ifndef __XIO_H__
#define __XIO_H__

#ifdef	__cplusplus
extern "C" {
#endif

/* 单字符输出(用户按具体平台自行实现，功能同标准putchar，务必确保已定义在xio_arch.c) */
int xputc(int c);

/* 单字符读入(用户按具体平台自行实现，功能与标准getchar有区别，此函数不回显、单字符立即返回，务必确保已定义在xio_arch.c) */
int xgetc(void);

/* 输出字符串(不同于标准puts，行尾不自动添加\n) */
int xputs(const char *str);

/* 向控制台格式化打印，用法同printf */
int xprintf(const char *format, ...);

/* 用十六进制编码来查看数据内容 */
int xhexdump(unsigned long address, /* 行首显示地址值的起始（仅用于指示，不非与实际buffer地址相同） */
             const void *buffer,    /* 要查看的数据指针 */
             int count,             /* 要查看的条目总数 */
             int width);            /* 每个条目的字节长度(1字节/2字节/4字节) */

/* 读入字符串(大体同标准gets，额外添加size参数避免缓冲区溢出) */
char *xgets(char *buffer, int size);

/* 暂停当前进程，按任意键继续 */
int xpause(const char *prompt);

/* 把一个字符串分割成字符串数组 */
int xsplit(char *str, const char *delim, int argc_max, char *argv[]);

/*  把字符串转换成长整型，支持2进制(0b/0B开头)、10进制(纯数字)、16进制(0x/0X开头)
 *  转换成功*err为0，失败*err为ERANGE或EINVAL */
long xatol(const char *str, int *err);

/*  把字符串转换成长整型，支持2进制(0b/0B开头)、10进制(纯数字)、16进制(0x/0X开头)
 *  若转换发生错误，*err为错误码，成功*err为0 */
unsigned long xatoul(const char *str, int *err);

#ifdef __cplusplus
}
#endif

#endif /* __XIO_H__ */