#include "llvm/ADT/APInt.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/IRBuilder.h"
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#include <iostream>

/*

llvm������������

�����ͣ�void��
���ͣ�iN��
�����ͣ�float��double�ȣ�

   case   1: return cast<IntegerType>(Type::getInt1Ty(C)); ����bool(true or false)
   case   8: return cast<IntegerType>(Type::getInt8Ty(C));
   case  16: return cast<IntegerType>(Type::getInt16Ty(C));
   case  32: return cast<IntegerType>(Type::getInt32Ty(C));
   case  64: return cast<IntegerType>(Type::getInt64Ty(C));
   case 128: return cast<IntegerType>(Type::getInt128Ty(C));

*/
int main(int argc, char** argv) {
    
    llvm::LLVMContext context;
    llvm::IRBuilder<> builder(context);

    std::unique_ptr<llvm::Module> llvm_module(new llvm::Module("basic.cpp", context));

    //void(void)
    auto main = llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getVoidTy(context), {}, false),
        llvm::GlobalValue::LinkageTypes::ExternalLinkage,
        "main", *llvm_module);

    auto bb = llvm::BasicBlock::Create(context, "block", main);
    builder.SetInsertPoint(bb); 

    llvm::Value* vreg = builder.CreateAlloca(llvm::Type::getInt64Ty(context), llvm::ConstantInt::get(llvm::Type::getInt32Ty(context),llvm::APInt(32,0x28)), "vreg");


    //int32
    llvm::Value* t1 = builder.CreateAlloca(llvm::Type::getInt32Ty(context),0,"t1");
    //int32*
    llvm::Value* t2 = builder.CreateAlloca(llvm::Type::getInt32PtrTy(context), 0, "t2");
    //int8*
    llvm::Value* t3 = builder.CreateAlloca(llvm::Type::getInt8PtrTy(context), 0, "t3");

//t1 ��һ��Int32������,�������ｫInt64��ֵ0���������ͻ����쳣
    //builder.CreateStore(llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), llvm::APInt(64, 0)), t1);

//�ܲ��ܽ�Int64ת��ΪInt32
//�������CreateAlloca�������ʵ��һ��i32*(ָ��),ת�Ļ�ֻ�ܽ�ֵ����ת��
    //builder.CreateIntCast(t1, llvm::Type::getInt64Ty(context),false);
    //builder.CreateStore(llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), llvm::APInt(64, 0)), t1);
    
    //printf("type:");
    //t1->getType()->print(llvm::outs(), true, true);
    
    llvm::Value* t4 = builder.CreateStore(t1, t2);

    builder.CreateRetVoid();
    builder.ClearInsertionPoint();
    llvm_module->print(llvm::outs(), nullptr);


    return 0;
}
