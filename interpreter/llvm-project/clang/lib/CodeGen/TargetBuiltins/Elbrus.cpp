//===---------- Elbrus.cpp - Emit LLVM Code for builtins ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This contains code to emit Builtin calls as LLVM code.
//
//===----------------------------------------------------------------------===//

#include "CGBuiltin.h"
#include "clang/Basic/TargetBuiltins.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/IntrinsicsElbrus.h"
#include "llvm/TargetParser/ElbrusTargetParser.h"

using namespace clang;
using namespace CodeGen;
using namespace llvm;

Value *CodeGenFunction::EmitElbrusCpuInit( const CallExpr *E) {
  return 0;
#if 0
  Function *F = CGM.getIntrinsic( Intrinsic::e2k_cpu_init);

  return Builder.CreateCall( F, {});
#endif
}

Value *CodeGenFunction::EmitElbrusCpuIs( const CallExpr *E) {
  const Expr *CPUExpr = E->getArg(0)->IgnoreParenCasts();
  StringRef CPUStr = cast<clang::StringLiteral>( CPUExpr)->getString();
  llvm::Module &M = CGM.getModule();
#if 0
  Function *F = CGM.getIntrinsic( Intrinsic::e2k_cpu_is);
  Value *CPUValue = Builder.CreateGlobalStringPtr( CPUStr, "", 0, &M);

  return Builder.CreateCall( F, {CPUValue});
#endif

  int CPUIdent = -1;
  int CPUShift = -1;

  struct {
      const char *name;
      int shift;
      int ident;
  } cpuinfos[] = {
      {"elbrus-v3",  0, 3},
      {"elbrus-4c",  1, 4},
      {"elbrus-v4",  0, 4},
      {"elbrus-8c",  1, 6},
      {"elbrus-1c+", 1, 7},
      {"elbrus-v5",  0, 5},
      {"elbrus-8c2", 1, 8},
      {"elbrus-v6",  0, 6},
      {"elbrus-12c", 1, 9},
      {"elbrus-16c", 1, 10},
      {"elbrus-2c3", 1, 11},
      {"elbrus-v7",  0, 7},
      {"elbrus-48c", 1, 12},
      {"elbrus-8v7", 1, 13},
      {"elbrus-e2k_v7p", 1, 12},
      {"elbrus-e2k_v7d", 1, 13},
      {0, -1, -1},
  };

  for ( int i = 0; cpuinfos[i].name; ++i ) {
      if ( CPUStr == cpuinfos[i].name ) {
          CPUIdent = cpuinfos[i].ident;
          CPUShift = cpuinfos[i].shift;
          break;
      }
  }

  if ( (CPUIdent < 0) || (CPUShift < 0) ) {
      return 0;
  }

  llvm::Type *TyI32 = Builder.getInt32Ty();
  llvm::Value *GV = M.getOrInsertGlobal( "__cpu_info", TyI32);
  llvm::Value *C0 = ConstantInt::get( TyI32, 0);
  llvm::Value *C1 = ConstantInt::get( TyI32, 1);
  llvm::Value *CN = ConstantInt::get( TyI32, CPUIdent);
  llvm::Value *CF = ConstantInt::get( TyI32, CPUShift);
  CharUnits ALIGN = getContext().toCharUnitsFromBits( 32);

  llvm::Value *GEP = Builder.CreateGEP( TyI32, GV, {CF});
  llvm::Value *LD = Builder.CreateAlignedLoad( TyI32, GEP, ALIGN);
  llvm::Value *CMP = Builder.CreateICmpEQ( LD, CN);
  llvm::Value *SEL = Builder.CreateSelect( CMP, C1, C0);

  return SEL;
}

Value *
CodeGenFunction::EmitElbrusBuiltinExpr( unsigned BuiltinID, const CallExpr *E) {
  if ( BuiltinID == Builtin::BI__builtin_cpu_init ) {
    return EmitElbrusCpuInit( E);
  } else if ( BuiltinID == Builtin::BI__builtin_cpu_is ) {
    return EmitElbrusCpuIs( E);
  }
#if 0
  if (BuiltinID == Builtin::BI__builtin_cpu_supports)
    return EmitX86CpuSupports(E);
  if (BuiltinID == Builtin::BI__builtin_cpu_init)
    return EmitX86CpuInit();
#endif

  return 0;
}

