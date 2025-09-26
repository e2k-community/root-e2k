//===-- ElbrusISelLowering.cpp - Elbrus DAG Lowering Implementation -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the interfaces that Elbrus uses to lower LLVM code into
// a selection DAG.
//
//===----------------------------------------------------------------------===//

#include "ElbrusRegisterInfo.h"
#include "ElbrusISelLowering.h"
#include "ElbrusTargetMachine.h"

using namespace llvm;

#define GET_REGINFO_MC_DESC
#define GET_REGINFO_TARGET_DESC
#include "ElbrusGenRegisterInfo.inc"

ElbrusTargetLowering::ElbrusTargetLowering( const TargetMachine &TM,
                                            const ElbrusSubtarget &STI)
    : TargetLowering(TM)
{
    // Set up the register classes.
    addRegisterClass(MVT::i8, &Elbrus::GR8RegClass);
    addRegisterClass(MVT::i16, &Elbrus::GR16RegClass);
    addRegisterClass(MVT::i32, &Elbrus::GR32RegClass);
    addRegisterClass(MVT::i64, &Elbrus::GR64RegClass);
    addRegisterClass(MVT::i128, &Elbrus::GR128RegClass);
    addRegisterClass(MVT::f32, &Elbrus::GR32RegClass);
    addRegisterClass(MVT::f64, &Elbrus::GR64RegClass);
    addRegisterClass(MVT::f80, &Elbrus::GR80RegClass);
    addRegisterClass(MVT::f128, &Elbrus::GR128RegClass);

    addRegisterClass(MVT::v4f32, &Elbrus::GR128RegClass);
    addRegisterClass(MVT::v2f64, &Elbrus::GR128RegClass);
    addRegisterClass(MVT::v16i8, &Elbrus::GR128RegClass);
    addRegisterClass(MVT::v8i16, &Elbrus::GR128RegClass);
    addRegisterClass(MVT::v4i32, &Elbrus::GR128RegClass);
    addRegisterClass(MVT::v2i64, &Elbrus::GR128RegClass);

    addRegisterClass(MVT::v8f32, &Elbrus::GR256RegClass);
    addRegisterClass(MVT::v4f64, &Elbrus::GR256RegClass);
    addRegisterClass(MVT::v32i8, &Elbrus::GR256RegClass);
    addRegisterClass(MVT::v16i16, &Elbrus::GR256RegClass);
    addRegisterClass(MVT::v8i32, &Elbrus::GR256RegClass);
    addRegisterClass(MVT::v4i64, &Elbrus::GR256RegClass);

    addRegisterClass(MVT::v16f32, &Elbrus::GR512RegClass);
    addRegisterClass(MVT::v8f64,  &Elbrus::GR512RegClass);
    addRegisterClass(MVT::v64i8,  &Elbrus::GR512RegClass);
    addRegisterClass(MVT::v32i16, &Elbrus::GR512RegClass);
    addRegisterClass(MVT::v16i32, &Elbrus::GR512RegClass);
    addRegisterClass(MVT::v8i64,  &Elbrus::GR512RegClass);

#if 0
    addRegisterClass(MVT::v1i1,   &Elbrus::VK1RegClass);
    addRegisterClass(MVT::v2i1,   &Elbrus::VK2RegClass);
    addRegisterClass(MVT::v4i1,   &Elbrus::VK4RegClass);
    addRegisterClass(MVT::v8i1,   &Elbrus::VK8RegClass);
    addRegisterClass(MVT::v16i1,  &Elbrus::VK16RegClass);

    addRegisterClass(MVT::v32i1,  &Elbrus::VK32RegClass);
    addRegisterClass(MVT::v64i1,  &Elbrus::VK64RegClass);
#endif

    computeRegisterProperties( STI.getRegisterInfo());
}

TargetLoweringBase::LegalizeTypeAction
ElbrusTargetLowering::getPreferredVectorAction(MVT VT) const {
  if (!VT.isScalableVector() && VT.getVectorNumElements() != 1 &&
      VT.getVectorElementType() != MVT::i1)
    return TypeWidenVector;

  return TargetLoweringBase::getPreferredVectorAction(VT);
}

