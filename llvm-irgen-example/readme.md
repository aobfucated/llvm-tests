通过IR编译出来的main.o，然后用clang链接成main.exe

```
PS C:\Users\asdf\source\repos\llvm-test\llvm-irgen-example> clang++ -g main.o -o main.exe -v
clang version 13.0.1
Target: x86_64-pc-windows-msvc
Thread model: posix
InstalledDir: E:\LLVM\bin
 "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Tools\\MSVC\\14.29.30037\\bin\\Hostx64\\x64\\link.exe" -out:main.exe -defaultlib:libcmt -defaultlib:oldnames "-libpath:E:\\LLVM\\lib\\clang\\13.0.1\\lib\\windows" -nologo -debug main.o
main.o : fatal error LNK1107: 文件无效或损坏: 无法在 0xDB 处读取
clang++: error: linker command failed with exit code 1107 (use -v to see invocation)
```


PS C:\Users\asdf\source\repos\llvm-test\llvm-irgen-example> llvm-objdump.exe -a main.o

main.o: file format coff-x86-64

PS C:\Users\asdf\source\repos\llvm-test\llvm-irgen-example> clang++ main.o

```
; Function Attrs: mustprogress noinline norecurse nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  store i32 1, i32* %2, align 4
  store i32 2, i32* %3, align 4
  %4 = load i32, i32* %2, align 4
  %5 = load i32, i32* %3, align 4
  %6 = add nsw i32 %4, %5
  ret i32 %6
}```

```
.text:0000000140001000 main            proc near               ; CODE XREF: __scrt_common_main_seh(void)+107↓p
.text:0000000140001000                 mov     eax, 3
.text:0000000140001005                 retn
.text:0000000140001005 main            endp
```
