#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Value.h>
#include <llvm/Transforms/Utils.h> //createPromoteMemoryToRegisterPass
#include <llvm/Transforms/InstCombine/InstCombine.h> //createInstructionCombiningPass
#include <llvm/Transforms/Scalar.h> //createReassociatePass createCFGSimplificationPass
#include <llvm/Transforms/Scalar/GVN.h> //createGVNPass
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Passes/PassBuilder.h>

#include "llvm/Support/Host.h" //getDefaultTargetTriple
//#include "llvm/Support/TargetRegistry.h" //llvm_ks::  这个头文件和下面的不能一起include
#include "llvm/MC/TargetRegistry.h" //llvm::
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Object/ObjectFile.h"

#include <memory>
#include <iostream>
#include <fstream>

//https://github.com/ustc-compiler/2017fall/blob/master/llvm-irgen-example/llvm-irgen-example.cpp


namespace llvm
{
    extern "C" void LLVMInitializeX86TargetInfo();
    extern "C" void LLVMInitializeX86Target();
    extern "C" void LLVMInitializeX86TargetMC();
    extern "C" void LLVMInitializeX86AsmParser();
    extern "C" void LLVMInitializeX86AsmPrinter();
}

using namespace llvm;


int main()
{
    LLVMContext context;
    IRBuilder<> builder(context);

    //一个Module对应一个cpp文件(编译单元)
    std::unique_ptr<Module> llvm_module(new Module("main.cpp", context));

    //创建一个int main(int argc,char*[])函数
    std::vector<Type*> params;
    params.push_back(Type::getInt32Ty(context));

    auto main = Function::Create(FunctionType::get(Type::getInt32Ty(context), params, false),
        GlobalValue::LinkageTypes::ExternalLinkage,
        "main", *llvm_module);

    
    // 函数创建后,要给入口点一个BasicBlock
    auto bb = BasicBlock::Create(context, "main_entry", main);
    builder.SetInsertPoint(bb);

    Value* t_1 = builder.CreateAlloca(Type::getInt32Ty(context),0,"ret_t");
    Value* t_2 = builder.CreateAlloca(Type::getInt32Ty(context),0,"a");
    Value* t_3 = builder.CreateAlloca(Type::getInt32Ty(context),0,"b");

    builder.CreateStore(ConstantInt::get(Type::getInt32Ty(context), APInt(32, 0)),t_1);
    builder.CreateStore(ConstantInt::get(Type::getInt32Ty(context), APInt(32, 1)), t_2);
    builder.CreateStore(ConstantInt::get(Type::getInt32Ty(context), APInt(32, 2)), t_3);
    
    Value* t_4 = builder.CreateLoad(Type::getInt32Ty(context), t_2);
    Value* t_5 = builder.CreateLoad(Type::getInt32Ty(context), t_3);

    Value* ret = builder.CreateAdd(t_4, t_5, "", 0, 1);

    // 一个BasicBlock的ir完成
    builder.CreateRet(ret);
    builder.ClearInsertionPoint();

    //before optmize
    std::cout << "[-]optimize before: \n";
    llvm_module->print(outs(), nullptr);

    //after optmize

    //https://llvm.org/docs/NewPassManager.html

    //老的pass接口 llvm::legacy::

    std::unique_ptr<llvm::legacy::FunctionPassManager> fpm = std::make_unique<llvm::legacy::FunctionPassManager>(&(*llvm_module));

    //some optimizes

    fpm->add(llvm::createPromoteMemoryToRegisterPass());
    //fpm->add(llvm::createInstructionCombiningPass());
    //pm->add(llvm::createReassociatePass());
    //fpm->add(llvm::createGVNPass());
    //fpm->add(llvm::createCFGSimplificationPass());
    fpm->run(*main);

    std::cout << "[-]optimize after : \n";
    llvm_module->print(outs(), nullptr);

    std::string target_triple = llvm::sys::getDefaultTargetTriple();
    std::cout << "[-]default target is " << target_triple << std::endl;

    std::cout << "[-]current target is " << llvm_module->getTargetTriple() << std::endl;

    //编译IR到本地x86代码
    //https://stackoverflow.com/questions/34822212/generate-machine-code-directly-via-llvm-api
    //https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl08.html
    //https://cache.one/read/15430686

    llvm_module->setTargetTriple(target_triple); //set x86 
    llvm::legacy::PassManager pass;

    llvm::Triple triple(target_triple);
    std::cout << "[-]default object type = " << triple.getObjectFormat() << std::endl;
    triple.setObjectFormat(Triple::COFF);

    
    //
    //初始化一些东西,不然的话下面的一些api类成员都是nullptr,要崩溃
    //
    LLVMInitializeX86TargetInfo();
    LLVMInitializeX86Target();
    LLVMInitializeX86TargetMC();
    LLVMInitializeX86AsmParser();
    LLVMInitializeX86AsmPrinter();

    std::string Error;
    llvm::TargetOptions opt;
    auto reloc_model = llvm::Optional< llvm::Reloc::Model>();
    const llvm::Target* target = llvm::TargetRegistry::lookupTarget(target_triple, Error);

    llvm::TargetMachine* target_machine = target->createTargetMachine(target_triple, "generic", "", opt, reloc_model);
    llvm_module->setDataLayout(target_machine->createDataLayout());

    llvm::SmallVector< char, 128 > buff;
    llvm::raw_svector_ostream dest(buff);
    target_machine->addPassesToEmitFile(pass, dest, nullptr, llvm::CGFT_ObjectFile);
    auto result = pass.run(*llvm_module);
    
    std::vector<uint8_t> obj = std::vector<uint8_t>(buff.begin(), buff.end());

    std::ofstream o("main.o",std::ios::binary);
    std::ostream_iterator<uint8_t> oi(o);
    std::copy(obj.begin(), obj.end(), oi);
    

    return 0;
}