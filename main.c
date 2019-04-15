#include <stdlib.h>  /* for rand() */
#include "xio/xio.h" /* for xhexdump */

int main() {
    /* 生成一组测试数据 */
    char data[64];
    for (int i = 0; i < sizeof(data); ++i)
        data[i] = (char)rand();

    /* 调用xhexdump格式化打印 */
    xhexdump((unsigned long)data, data, sizeof(data), sizeof(data[0]));
    return 0;
}