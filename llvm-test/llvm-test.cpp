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
#include <llvm/Transforms/Utils.h> 
#include <llvm/Transforms/InstCombine/InstCombine.h> 
#include <llvm/Transforms/Scalar.h> 
#include <llvm/Transforms/Scalar/GVN.h> 
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Passes/PassBuilder.h>

#include "llvm/Support/Host.h"
#include "llvm/MC/TargetRegistry.h" 
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Object/ObjectFile.h"

#include <memory>
#include <iostream>
#include <fstream>
#include <string>
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

    std::unique_ptr<Module> llvm_module(new Module("vmp2-example.cpp", context));

    //void(void)
    auto main = Function::Create(FunctionType::get(Type::getVoidTy(context), {}, false),
        GlobalValue::LinkageTypes::ExternalLinkage,
        "main", *llvm_module);


    auto bb = BasicBlock::Create(context, "vmp2block", main);
    builder.SetInsertPoint(bb);

    //vmp2的context 0x140个字节
    Type* vreg_array_type = ArrayType::get(Type::getInt64Ty(context), 0x28);
    Value* vreg = builder.CreateAlloca(vreg_array_type,0,"vreg");
    
    //学习llvm的GetElementPtr指令 https://www.codeleading.com/article/70285814012/   
    
    //sregq 0x58 0   mov verg,0
    Value* vreg_58 = builder.CreateStore(ConstantInt::get(Type::getInt64Ty(context), APInt(64, 0)), builder.CreateConstGEP2_64(vreg_array_type, vreg, 0,0x58/8));

    //LCONSTDWSXQ 0x5c00d5f5
    //builder.CreateStore(ConstantInt::get(Type::getInt32Ty(context), APInt(32, 0x5c00d5f5)), vrsp);



    builder.CreateRetVoid();
    builder.ClearInsertionPoint();

    //before optmize
    std::cout << "[-]optimize before: \n\n\n";
    llvm_module->print(outs(), nullptr);    

    std::unique_ptr<llvm::legacy::FunctionPassManager> fpm = std::make_unique<llvm::legacy::FunctionPassManager>(&(*llvm_module));

    //some optimizes

    fpm->add(llvm::createPromoteMemoryToRegisterPass());
    fpm->run(*main);

    std::cout << "[-]optimize after: \n\n\n";
    llvm_module->print(outs(), nullptr);

    std::string target_triple = llvm::sys::getDefaultTargetTriple();

    llvm_module->setTargetTriple(target_triple); 
    llvm::legacy::PassManager pass;

    llvm::Triple triple(target_triple);
    triple.setObjectFormat(Triple::COFF);

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

    //std::ofstream o("main.o", std::ios::binary);
    //std::ostream_iterator<uint8_t> oi(o);
    //std::copy(obj.begin(), obj.end(), oi);


    return 0;
}