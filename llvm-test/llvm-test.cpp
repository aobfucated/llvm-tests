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
#include <llvm/IR/InlineAsm.h>
#include <llvm/Support/Alignment.h>

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

std::vector<AllocaInst*> virtual_registers;

int main()
{
    LLVMContext context;
    IRBuilder<> builder(context);//通过builder.getContext()可以获得这个context

    std::unique_ptr<Module> llvm_module(new Module("vmp2-example.cpp", context));

    //void(int64)
    auto main = Function::Create(FunctionType::get(Type::getVoidTy(context), {Type::getInt64Ty(context)}, false),
        GlobalValue::LinkageTypes::ExternalLinkage,
        "main", *llvm_module);


    auto bb = BasicBlock::Create(context, "vmp2block", main);
    builder.SetInsertPoint(bb);

    //vmp2的context 0x140个字节 以数组的形式创建0x28个uint64  sub rsp,0x140
    //Type* vreg_array_type = ArrayType::get(Type::getInt64Ty(context), 0x28);
    //Value* vreg = builder.CreateAlloca(vreg_array_type,0,"vreg");
    
    //sub rsp,0x140
    for (auto idx = 0u; idx < 0x28; ++idx) //将虚拟寄存器以vector的形式存放在栈上
    {
        virtual_registers.push_back(builder.CreateAlloca(llvm::IntegerType::get(context, 64), nullptr, (std::string("vreg") + std::to_string(idx)).c_str()));
        
        //给每个寄存器赋值，1.方便调试 2.防止llvm默认优化
        builder.CreateStore(ConstantInt::get(Type::getInt64Ty(context), APInt(64, idx)),virtual_registers[idx]);
    }
//vm-ctx有了,vm-stack咋办?
//分配一个数组?  
    //builder.CreateAlloca(PointerType::get(builder.getInt8Ty(), 0), 0, "vsp");
    Type* word_array_type = ArrayType::get(Type::getInt16Ty(context), 1); //测试byte ptr转word ptr
    Type* byte_array_type = ArrayType::get(Type::getInt8Ty(context), 0x100); //vmp2是两字节对齐的堆栈,搞一字节好控制
    
    Value* vstack = builder.CreateAlloca(byte_array_type,0,"vstack");
    Value* get_stack = builder.CreateLoad(byte_array_type, vstack, "stack_value");
    //outs() << get_stack->getType()->getArrayElementType()->getTypeID() << "\n";

    //学习llvm的GetElementPtr指令 https://www.codeleading.com/article/70285814012/   
    
//sregq 0x58 0   mov verg,0 -> mov     qword ptr [rsp + 0x58], 0x0
    
    //以数组的形式存储
    //Value* vreg_58 = builder.CreateStore(ConstantInt::get(Type::getInt64Ty(context), APInt(64, 0)), builder.CreateConstGEP2_64(vreg_array_type, vreg, 0,0x58/8));

    //存储到vector里 
    //这里一开始出了个问题,i64(0) i32(0) i16(0) i8(0),假如一开始分配的是i64的地址,那就只能store i64 
    builder.CreateStore(ConstantInt::get(Type::getInt64Ty(context),APInt(64,0)), virtual_registers[0x58 / 8]);

//LCONSTDWSXQ 0x5c00d5f5
//上面创建出来的指针默认是byte ptr,我们要根据我们的需求改成word ptr dword ptr qword ptr
    Value* gep = builder.CreateConstGEP2_64(byte_array_type,vstack,0,0); //从地址0存四字节
    gep->getType()->print(outs()); //i8*  
    //builder.CreateIntCast(gep, PointerType::getInt32PtrTy(context),false);
    //Value* gep_ptr_to_int = builder.CreatePtrToInt(gep, Type::getInt64Ty(context));
    auto gep_2 = builder.CreateBitCast(gep, Type::getInt32PtrTy(context));
    gep_2->getType()->print(outs()); //gep_2 i32*
//将byte ptr转dword ptr

    //auto _1 = builder.CreateStore(ConstantInt::get(Type::getInt8Ty(context),APInt(8, 0x25)),gep);
//gep已经从i8*转化为i32*
    auto _1 = builder.CreateStore(ConstantInt::get(Type::getInt32Ty(context), APInt(32, 0x5c00d5f5)), gep_2);
//设置对齐啥用?
    //_1->setAlignment(llvm::Align(1));

//SREGQ 0x28
    builder.CreateStore(ConstantInt::get(Type::getInt64Ty(context), APInt(64, 0)), virtual_registers[0x28 / 8]);

//LCONSTDWSXQ 0x140009dd8
    gep = builder.CreateConstGEP2_64(byte_array_type, vstack, 0, 4);//地址4开始存8字节
    gep_2 = builder.CreateBitCast(gep, Type::getInt64PtrTy(context));
    auto _2 = builder.CreateStore(ConstantInt::get(Type::getInt64Ty(context), APInt(64, 0x140009dd8)), gep_2);

//read 0x140000000  -> mov qword ptr [0x140000000]
    //gep_2->getType()->print(outs()); //gep_2 is i64*
    Value* address = ConstantInt::get(Type::getInt64Ty(context), APInt(64, 0x140000000));
    Value* addr_ptr = builder.CreateIntToPtr(address, PointerType::getInt64PtrTy(context));
    auto ptrptr = builder.CreateAlloca(PointerType::getInt64PtrTy(context),false,nullptr,"ptrptr");
    auto aaaa = builder.CreateStore(addr_ptr, ptrptr);
    builder.CreateLoad(Type::getInt64Ty(context), addr_ptr);

    addr_ptr->getType()->print(outs());
    auto load_des = builder.CreateLoad(Type::getInt64Ty(context), addr_ptr);

    gep = builder.CreateConstGEP2_64(byte_array_type, vstack, 0, 12);//12开始8字节
    gep_2 = builder.CreateBitCast(gep, Type::getInt64PtrTy(context));
    _2 = builder.CreateStore(load_des, gep_2);

    std::string asm_str;
    //这里汇编失败的话,下面complier to obj的时候就会失败
    asm_str.assign("push rax;push rcx;pushfq;mov rbp,rsp;sub rsp,140h;mov rdi,rsp;mov rcx,rsp;");
    llvm::InlineAsm* inlineAsm = llvm::InlineAsm::get(llvm::FunctionType::get(Type::getVoidTy(context), false), asm_str, "", false, false, llvm::InlineAsm::AD_Intel);

    //当前位置内嵌汇编 相当于_asm()
    builder.CreateCall(inlineAsm);
    

    //LCONSTDWSXQ 0x5c00d5f5
    //builder.CreateStore(ConstantInt::get(Type::getInt32Ty(context), APInt(32, 0x5c00d5f5)), vrsp);
    
    //bool llvm::Function::isIntrinsic()	const
    FunctionCallee read_register = llvm_module->getOrInsertFunction("read_register", FunctionType::get(IntegerType::get(context, 64), IntegerType::get(context, 64)));
    if (!read_register)
    {
        throw std::runtime_error("no function named returnaddress");
    }

    //CallInst* stack_ptr = builder.CreateCall(read_register, ConstantInt::get(Type::getInt64Ty(context), APInt(64, 0)));
    //stack_ptr->addFnAttr(Attribute::AlwaysInline);

    
    auto rcx = main->getArg(0);

    //sub rcx,x? 这部分倒是可以用inlineasm实现，但是inlineasm会影响优化
    auto newrcx = builder.CreateSub(rcx, ConstantInt::get(Type::getInt64Ty(context), APInt(64, 3)));


    auto aptr = builder.CreateAlloca(PointerType::getInt64Ty(context));
    builder.CreateStore(newrcx, aptr);

    outs() << "\n";
    newrcx->getType()->print(outs());
    aptr->getType()->print(outs());

    //rcx->getType()->print(outs()); //rcx == i64
    //auto afteradd = builder.CreateAdd(newrcx, ConstantInt::get(Type::getInt64Ty(context), APInt(64, 2)));
    //auto rcx_ptr = builder.CreateIntToPtr(afteradd, PointerType::getInt8PtrTy(context));

    //rcx_ptr->getType()->getScalarType()->print(outs());  //i8*
    //cast<PointerType>(rcx_ptr);
    //builder.CreateGEP(Type::getInt8Ty(context), rcx_ptr,0);
    //gep = builder.CreateConstGEP2_64(byte_array_type, rcx_ptr, 0, 0);//12开始8字节
    //builder.CreateStore(ConstantInt::get(Type::getInt8Ty(context), APInt(8, 0)), rcx_ptr); //mov [rcx+x],0






    builder.CreateRetVoid();
    builder.ClearInsertionPoint();

    //before optmize
    std::cout << "[-]optimize before: \n\n\n";
    llvm_module->print(outs(), nullptr);    

    std::unique_ptr<llvm::legacy::FunctionPassManager> fpm = std::make_unique<llvm::legacy::FunctionPassManager>(&(*llvm_module));

    //some optimizes

    fpm->add(llvm::createPromoteMemoryToRegisterPass());
    //fpm->run(*main);

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

    std::ofstream o("main.o", std::ios::binary);
    std::ostream_iterator<uint8_t> oi(o);
    std::copy(obj.begin(), obj.end(), oi);


    return 0;
}