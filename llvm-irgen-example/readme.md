ͨ��IR���������main.o��Ȼ����clang���ӳ�main.exe

```
PS C:\Users\asdf\source\repos\llvm-test\llvm-irgen-example> clang++ -g main.o -o main.exe -v
clang version 13.0.1
Target: x86_64-pc-windows-msvc
Thread model: posix
InstalledDir: E:\LLVM\bin
 "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Tools\\MSVC\\14.29.30037\\bin\\Hostx64\\x64\\link.exe" -out:main.exe -defaultlib:libcmt -defaultlib:oldnames "-libpath:E:\\LLVM\\lib\\clang\\13.0.1\\lib\\windows" -nologo -debug main.o
main.o : fatal error LNK1107: �ļ���Ч����: �޷��� 0xDB ����ȡ
clang++: error: linker command failed with exit code 1107 (use -v to see invocation)
```


