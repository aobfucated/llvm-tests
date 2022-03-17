#include <iostream>
#include <llvm/IR/LLVMContext.h> //llvm::LLVMContext
#include <llvm/IR/IRBuilder.h> //llvm::IRBuild
#include <llvm/IR/Module.h> //llvm::Module

//
//本项目的目的;
//  有了IR(main.ll)的前提,怎么通过llvm编译成x86的代码?
//

namespace llvm
{
    extern "C" void LLVMInitializeX86TargetInfo();
    extern "C" void LLVMInitializeX86Target();
    extern "C" void LLVMInitializeX86TargetMC();
    extern "C" void LLVMInitializeX86AsmParser();
    extern "C" void LLVMInitializeX86AsmPrinter();
} // namespace llvm

int main(int argc,char *argv[])
{
    llvm::LLVMContext llvm_ctx;
    llvm::Module llvm_module("fib", llvm_ctx);
    llvm::IRBuilder<> ir_builder(llvm_ctx);

    llvm::LLVMInitializeX86TargetInfo();
    llvm::LLVMInitializeX86Target();
    llvm::LLVMInitializeX86TargetMC();
    llvm::LLVMInitializeX86AsmParser();
    llvm::LLVMInitializeX86AsmPrinter();
     
    

    

    return 0;
}

