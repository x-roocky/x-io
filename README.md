## X-Intput/Output - 一组扩展IO及字符串处理的C函数

### 起因

好多年前，第一次看到[ChaN](http://elm-chan.org)前辈写的[*xprintf()*](http://elm-chan.org/fsw/strf/xprintf.html)，被惊艳到，简单的函数，优雅、好用。后来，我随着使用，加了一些功能。再后来，我用C99标准重构一次，就是[现在的版本](https://github.com/roocky-lab/x-io)。

### 功能

面向单片机(MCU)开发，对串口作为主要调试工具的场景，提供一组扩展IO、字符串处理的C函数：
- *xprintf()*: 重定向的printf
- *xhexdump()*: 格式化打印内存信息，类似Linux中*hexdump*命令
- *xgets()*: 解决内存泄漏风险的get函数
- *xpause()*: 按任意键暂停
- *xsplit()*: 字符串分割函数
- *xatol()*: 字符串转长整型，自动判断所属进制(二进制/十进制/十六进制)
- *xatoul()*: 同上，无符号长整型版本

### 例子

每个函数功能独立，以*main.c*中附带的*xhexdump()*使用为例：

```c
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
```

调用附带的Makefile运行，效果如下：
```shell
$ make run
./xio_test
7ffd39671e20  67 c6 69 73 51 ff 4a ec 29 cd ba ab f2 fb e3 46  |g.isQ.J.)......F|
7ffd39671e30  7c c2 54 f8 1b e8 e7 8d 76 5a 2e 63 33 9f c9 9a  ||.T.....vZ.c3...|
7ffd39671e40  66 32 0d b7 31 58 a3 5a 25 5d 05 17 58 e9 5e d4  |f2..1X.Z%]..X.^.|
7ffd39671e50  ab b2 cd c6 9b b4 54 11 0e 82 74 41 21 3d dc 87  |......T...tA!=..|
```

### FAQ

1. **我使用xx架构单片机，支持吗？**

    任何平台都支持，在"*xio/xio-arch.c*"中定义两个IO函数，示例：

    ```c
    /* 单字符输出接口 */
    int xputc(int c) {
        return uart_send_byte((char)c);
    }

    /* 单字符读入接口(要求无回显，成功返回读入字符，无数据返回EOF) */
    int xgetc(void) {
        return uart_recv_byte();
    }
    ```
    其中*uart_send_byte()*、 *uart_recv_byte()*指对应平台的串口发送和接收函数。
    定义完毕，包含"*xio/xio.h*"即可在程序中使用。

2. **为什么用打印调试，而不用单步调试？**

    嵌入式设备上的软件开发，虽然也有硬件调试器(比如JTAG)，但很少使用：
    - 外设通信协议调试的场景，指令响应周期很短，无法暂停使用单步调试
    - PCB面积、成本及安全的原因，量产硬件不会保留调试器接口
    - 部分项目单独的调试板会保留，但调试板很难和量产环境一致，很多需要现场调试

    总之，除了调试汇编指令(比如Boot)外，一般不会、也没条件用硬件调试器。

3. **为什么不直接使用标准C函数？**
    
    有几方面原因：
    - 单片机开发，通常没有标准 *stdout、stdin* , 本来*printf()*、 *scanf()*  等也不会直接使用
    - 标准库 *scanf()、gets()* 等函数原本就存在内存泄露风险，任何平台都不推荐再使用
    - 补充一些实用功能： *xpause()、xsplit()、xatol()、xatoul()*、*xhexdump()*

------

GitHub地址：https://github.com/roocky-lab/x-io
