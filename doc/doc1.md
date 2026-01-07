## gdb连接不上QEMU虚拟机

配置 gdb 出现如下问题
![alt text](./docpic/image.png)
可能是因为 `CygWin` 中安装的 `gdb` 与 `qemu` 虚拟机的 `gdb stub` 不兼容，`x86_64_elf_tools_windows` 中的 `x86_64-elf-gdb.exe` 可以使用。

**解决办法**

配置 `launch.json` 文件

**1.Windows**

这里的miDebuggerPath改为x86_64_elf_tools_windows安装在电脑的位置
```
"miDebuggerPath": "D:/ProgramEnv/x86_64_elf_tools_windows/bin/x86_64-elf-gdb.exe",
```

**2.Ubuntu**
查找系统 `gdb` 路径
```bash
whereis gdb
```
把 `miDebuggerPath` 改为终端输出的路径即可。


[返回README](../README.md)