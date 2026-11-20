//===- Mips.cpp -----------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ABIInfoImpl.h"
#include "TargetInfo.h"

using namespace clang;
using namespace clang::CodeGen;

//===----------------------------------------------------------------------===//
// Elbrus ABI Implementation.
//===----------------------------------------------------------------------===//

namespace {
class ElbrusABIInfo : public DefaultABIInfo {
public:
  ElbrusABIInfo(CodeGenTypes &CGT) : DefaultABIInfo(CGT) {}

  //ABIArgInfo classifyReturnType(QualType RetTy) const;
  ABIArgInfo classifyArgumentType(QualType RetTy) const;

  bool isRecordReturnIndirect( QualType Ty) const {
    const CXXRecordDecl *RD = Ty->getAsCXXRecordDecl();

    if ( !RD )
      return false;

    // Records with non-trivial destructors/copy-constructors should not be
    // passed by value.
    if (CGCXXABI::RecordArgABI RAA = getRecordArgABI(Ty, getCXXABI()))
      return true;

    return false;
  }

  void computeInfo(CGFunctionInfo &FI) const override {
    CGFunctionInfo::arg_iterator it, ie;
    QualType rtype = FI.getReturnType();

    DefaultABIInfo::computeInfo( FI);

    FI.getReturnInfo() = classifyReturnType( rtype);
    if ( !rtype->isVoidType() )
      FI.getReturnInfo() = ABIArgInfo::getDirect();

    if ( isAggregateTypeForABI( rtype) )
      if ( isRecordReturnIndirect( rtype) )
        FI.getReturnInfo() = ABIInfo::getNaturalAlignIndirect( rtype, 0);

    for ( it = FI.arg_begin(), ie = FI.arg_end(); it != ie; ++it )
      if ( isAggregateTypeForABI( it->type)
           && !isRecordReturnIndirect( it->type) )
      {
        it->info = ABIArgInfo::getDirect( CGT.ConvertType( it->type));
        it->info.setCoerceToType( CGT.ConvertType( it->type));
      }
  }

  RValue EmitVAArg(CodeGenFunction &CGF, Address VAListAddr, QualType Ty,
                   AggValueSlot Slot) const override {
    return CGF.EmitLoadOfAnyValue( CGF.MakeAddrLValue(
             EmitVAArgInstr(CGF, VAListAddr, Ty, classifyArgumentType(Ty)), Ty), Slot);
  }
};

class ElbrusTargetCodeGenInfo : public TargetCodeGenInfo {
public:
  ElbrusTargetCodeGenInfo( CodeGenTypes &CGT)
      : TargetCodeGenInfo( std::make_unique<ElbrusABIInfo>( CGT)) {}

  void setTargetAttributes(const Decl *D, llvm::GlobalValue *GV,
                           CodeGen::CodeGenModule &CGM) const override {
    const auto *FD = dyn_cast_or_null<FunctionDecl>(D);
    if (!FD) return;
    auto *Fn = cast<llvm::Function>(GV);

    if (FD->getAttr<ElbrusInterruptAttr>())
      Fn->addFnAttr("interrupt");
    if (FD->getAttr<ElbrusCheckStackAttr>())
      Fn->addFnAttr("interrupt");
    if (FD->getAttr<ElbrusNoLoopApbAttr>())
      Fn->addFnAttr("no-loop-apb");
    if (FD->getAttr<ElbrusNoSemiSpecLdAttr>())
      Fn->addFnAttr("no-semi-spec-ld");
  }
};

ABIArgInfo ElbrusABIInfo::classifyArgumentType( QualType Ty) const {
    ABIArgInfo r;

    Ty = useFirstFieldIfTransparentUnion(Ty);

    if ( isAggregateTypeForABI( Ty) )
    {
        // Records with non-trivial destructors/copy-constructors should not be
        // passed by value.
        if (CGCXXABI::RecordArgABI RAA = getRecordArgABI(Ty, getCXXABI()))
            return getNaturalAlignIndirect(Ty, RAA == CGCXXABI::RAA_DirectInMemory);

        return ABIArgInfo::getDirect();
    }

    // Treat an enum type as its underlying type.
    if (const EnumType *EnumTy = Ty->getAs<EnumType>())
        Ty = EnumTy->getDecl()->getIntegerType();

    if ( isPromotableIntegerTypeForABI( Ty) ) {
       r = ABIArgInfo::getExtend(Ty);
    } else {
       r = ABIArgInfo::getDirect();
    }

    return (r);
}

}

std::unique_ptr<TargetCodeGenInfo>
CodeGen::createElbrusTargetCodeGenInfo(CodeGenModule &CGM) {
  return std::make_unique<ElbrusTargetCodeGenInfo>(CGM.getTypes());
}
