//===-- ElbrusTargetTransformInfo.h - Elbrus specific TTI -------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
/// This file a TargetTransformInfo::Concept conforming object specific to the
/// Elbrus target machine. It uses the target's detailed information to
/// provide more precise answers to certain TTI queries, while letting the
/// target independent and default TTI implementations handle the rest.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ELBRUS_TARGETTRANSFORMINFO_H
#define LLVM_LIB_TARGET_ELBRUS_TARGETTRANSFORMINFO_H

#include "ElbrusTargetMachine.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/CodeGen/BasicTTIImpl.h"
#include <optional>

namespace llvm {

class InstCombiner;

class ElbrusTTIImpl : public BasicTTIImplBase<ElbrusTTIImpl> {
  typedef BasicTTIImplBase<ElbrusTTIImpl> BaseT;
  typedef TargetTransformInfo TTI;
  friend BaseT;

  const ElbrusSubtarget *ST;
  const ElbrusTargetLowering *TLI;

  const ElbrusSubtarget *getST() const { return ST; }
  const ElbrusTargetLowering *getTLI() const { return TLI; }

public:
  explicit ElbrusTTIImpl(const ElbrusTargetMachine *TM, const Function &F)
      : BaseT(TM, F.getDataLayout()), ST(TM->getSubtargetImpl(F)),
        TLI(ST->getTargetLowering()) {}

  /// \name Scalar TTI Implementations
  /// @{
  TTI::PopcntSupportKind getPopcntSupport(unsigned TyWidth);

  /// @}

  /// \name Cache TTI Implementation
  /// @{
  std::optional<unsigned> getCacheSize(
    TargetTransformInfo::CacheLevel Level) const override;
  std::optional<unsigned> getCacheAssociativity(
    TargetTransformInfo::CacheLevel Level) const override;
  /// @}

  /// \name Vector TTI Implementations
  /// @{

  unsigned getNumberOfRegisters(unsigned ClassID) const override;
  unsigned getMaximumVF( unsigned ElemWidth, unsigned Opcode) const override;
  TypeSize getRegisterBitWidth(TargetTransformInfo::RegisterKind K) const override;
  unsigned getLoadStoreVecRegBitWidth(unsigned AS) const override;
  unsigned getMaxInterleaveFactor(ElementCount VF);
  InstructionCost getArithmeticInstrCost(
      unsigned Opcode, Type *Ty, TTI::TargetCostKind CostKind,
      TTI::OperandValueInfo Op1Info = {TTI::OK_AnyValue, TTI::OP_None},
      TTI::OperandValueInfo Op2Info = {TTI::OK_AnyValue, TTI::OP_None},
      ArrayRef<const Value *> Args = {},
      const Instruction *CxtI = nullptr) const override;
  InstructionCost getAltInstrCost(VectorType *VecTy, unsigned Opcode0,
                                  unsigned Opcode1,
                                  const SmallBitVector &OpcodeMask,
                                  TTI::TargetCostKind CostKind) const override;
  InstructionCost getShuffleCost(TTI::ShuffleKind Kind, VectorType *Tp, VectorType *SrcTy,
                                 ArrayRef<int> Mask, TTI::TargetCostKind CostKind,
                                 int Index, VectorType *SubTp,
                                 ArrayRef<const Value *> Args = {},
                                 const Instruction *CxtI = nullptr) const override;
  InstructionCost getCastInstrCost(unsigned Opcode, Type *Dst, Type *Src,
                                   TTI::CastContextHint CCH,
                                   TTI::TargetCostKind CostKind,
                                   const Instruction *I = nullptr) const override;
  InstructionCost getCmpSelInstrCost(unsigned Opcode, Type *ValTy, Type *CondTy,
                                     CmpInst::Predicate VecPred,
                                     TTI::TargetCostKind CostKind,
                                     TTI::OperandValueInfo Op1Info = {TTI::OK_AnyValue, TTI::OP_None},
                                     TTI::OperandValueInfo Op2Info = {TTI::OK_AnyValue, TTI::OP_None},
                                     const Instruction *I = nullptr) const override;
  using BaseT::getVectorInstrCost;
  InstructionCost getVectorInstrCost(unsigned Opcode, Type *Val,
                                     TTI::TargetCostKind CostKind,
                                     unsigned Index, Value *Op0, Value *Op1);
  InstructionCost getScalarizationOverhead(
      VectorType *Ty, const APInt &DemandedElts, bool Insert, bool Extract,
      TTI::TargetCostKind CostKind, bool ForPoisonSrc = true,
      ArrayRef<Value *> VL = {}) const override;
  InstructionCost getReplicationShuffleCost(Type *EltTy, int ReplicationFactor,
                                            int VF,
                                            const APInt &DemandedDstElts,
                                            TTI::TargetCostKind CostKind) const override;
  InstructionCost
  getMemoryOpCost(unsigned Opcode, Type *Src, Align Alignment,
                  unsigned AddressSpace, TTI::TargetCostKind CostKind,
                  TTI::OperandValueInfo OpInfo = {TTI::OK_AnyValue, TTI::OP_None},
                  const Instruction *I = nullptr) const override;
  InstructionCost
  getMemIntrinsicInstrCost(const MemIntrinsicCostAttributes &MICA,
                           TTI::TargetCostKind CostKind) const override;
  InstructionCost getMaskedMemoryOpCost(unsigned Opcode, Type *Src,
                                        Align Alignment, unsigned AddressSpace,
                                        TTI::TargetCostKind CostKind) const;
  InstructionCost getGatherScatterOpCost(unsigned Opcode, Type *DataTy,
                                         const Value *Ptr, bool VariableMask,
                                         Align Alignment,
                                         TTI::TargetCostKind CostKind,
                                         const Instruction *I) const;
  InstructionCost getPointersChainCost(ArrayRef<const Value *> Ptrs,
                                       const Value *Base,
                                       const TTI::PointersChainInfo &Info,
                                       Type *AccessTy,
                                       TTI::TargetCostKind CostKind);
  InstructionCost getAddressComputationCost(Type *PtrTy, ScalarEvolution *SE,
                                            const SCEV *Ptr);

  unsigned getAtomicMemIntrinsicMaxElementSize() const override;

  InstructionCost getIntrinsicInstrCost(const IntrinsicCostAttributes &ICA,
                                        TTI::TargetCostKind CostKind) const override;

  bool shouldExpandReduction( const IntrinsicInst *II) const override;

  InstructionCost getArithmeticReductionCost(unsigned Opcode, VectorType *Ty,
                                             std::optional<FastMathFlags> FMF,
                                             TTI::TargetCostKind CostKind) const override;

  InstructionCost getMinMaxCost(Intrinsic::ID IID, Type *Ty,
                                TTI::TargetCostKind CostKind,
                                FastMathFlags FMF) const;

  InstructionCost getMinMaxReductionCost(Intrinsic::ID IID, VectorType *Ty,
                                         FastMathFlags FMF,
                                         TTI::TargetCostKind CostKind) const override;

  InstructionCost getInterleavedMemoryOpCost(
      unsigned Opcode, Type *VecTy, unsigned Factor, ArrayRef<unsigned> Indices,
      Align Alignment, unsigned AddressSpace, TTI::TargetCostKind CostKind,
      bool UseMaskForCond = false, bool UseMaskForGaps = false);
  InstructionCost getInterleavedMemoryOpCostAVX512(
      unsigned Opcode, FixedVectorType *VecTy, unsigned Factor,
      ArrayRef<unsigned> Indices, Align Alignment, unsigned AddressSpace,
      TTI::TargetCostKind CostKind, bool UseMaskForCond = false,
      bool UseMaskForGaps = false);

  InstructionCost getIntImmCost(int64_t);

  InstructionCost getIntImmCost(const APInt &Imm, Type *Ty,
                                TTI::TargetCostKind CostKind);

  InstructionCost getCFInstrCost(unsigned Opcode, TTI::TargetCostKind CostKind,
                                 const Instruction *I = nullptr) const override;

  InstructionCost getIntImmCostInst(unsigned Opcode, unsigned Idx,
                                    const APInt &Imm, Type *Ty,
                                    TTI::TargetCostKind CostKind,
                                    Instruction *Inst = nullptr);
  InstructionCost getIntImmCostIntrin(Intrinsic::ID IID, unsigned Idx,
                                      const APInt &Imm, Type *Ty,
                                      TTI::TargetCostKind CostKind);
  /// Return the cost of the scaling factor used in the addressing
  /// mode represented by AM for this target, for a load/store
  /// of the specified type.
  /// If the AM is supported, the return value must be >= 0.
  /// If the AM is not supported, it returns a negative value.
  InstructionCost getScalingFactorCost(Type *Ty, GlobalValue *BaseGV,
                                       StackOffset BaseOffset, bool HasBaseReg,
                                       int64_t Scale, unsigned AddrSpace) const override;

  bool isLSRCostLess(const TargetTransformInfo::LSRCost &C1,
                     const TargetTransformInfo::LSRCost &C2);
  bool canMacroFuseCmp();
  bool isLegalMaskedLoad(Type *DataType, Align Alignment, unsigned AddrSpace = 0,
                         TTI::MaskKind MaskKind =
                             TTI::MaskKind::VariableOrConstantMask) const override;
  bool isLegalMaskedStore(Type *DataType, Align Alignment, unsigned AddrSpace = 0,
                          TTI::MaskKind MaskKind =
                              TTI::MaskKind::VariableOrConstantMask) const override;
  bool isLegalNTLoad(Type *DataType, Align Alignment);
  bool isLegalNTStore(Type *DataType, Align Alignment);
  bool isLegalBroadcastLoad(Type *ElementTy, ElementCount NumElements) const override;
  bool forceScalarizeMaskedGather(VectorType *VTy, Align Alignment);
  bool forceScalarizeMaskedScatter(VectorType *VTy, Align Alignment) {
    return forceScalarizeMaskedGather(VTy, Alignment);
  }
  bool isLegalMaskedGatherScatter(Type *DataType, Align Alignment);
  bool isLegalMaskedGather(Type *DataType, Align Alignment);
  bool isLegalMaskedScatter(Type *DataType, Align Alignment);
  bool isLegalMaskedExpandLoad(Type *DataType, Align Alignment);
  bool isLegalMaskedCompressStore(Type *DataType, Align Alignment);
  bool isLegalAltInstr(VectorType *VecTy, unsigned Opcode0, unsigned Opcode1,
                       const SmallBitVector &OpcodeMask) const override;
  bool hasDivRemOp(Type *DataType, bool IsSigned);
  bool isExpensiveToSpeculativelyExecute(const Instruction *I) const override;
  bool isFCmpOrdCheaperThanFCmpZero(Type *Ty);
  bool areInlineCompatible(const Function *Caller,
                           const Function *Callee) const override;
  bool areTypesABICompatible(const Function *Caller, const Function *Callee,
                             ArrayRef<Type *> Type) const override;

  uint64_t getMaxMemIntrinsicInlineSizeThreshold() const override {
    return 64;
  }

  TTI::MemCmpExpansionOptions enableMemCmpExpansion(bool OptSize,
                                                    bool IsZeroCmp) const override;
  bool prefersVectorizedAddressing() const override;
  bool supportsEfficientVectorElementLoadStore() const override;
  bool enableInterleavedAccessVectorization();

private:
  bool supportsGather() const;
  InstructionCost getGSVectorCost(unsigned Opcode, TTI::TargetCostKind CostKind,
                                  Type *DataTy, const Value *Ptr,
                                  Align Alignment, unsigned AddressSpace);

  int getGatherOverhead() const;
  int getScatterOverhead() const;

  /// @}
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_ELBRUS_TARGETTRANSFORMINFO_H
