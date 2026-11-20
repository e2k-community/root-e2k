//=- llvm/CodeGen/Lccrt/LccrtEmitter.h - Lccrt-IR translation -*- C++ -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the LccrtEmitter class, which
// realizes translation from LLVM-IR to LCCRT-IR.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_CODEGEN_LCCRT_LCCRTADDRDEP_H
#define LLVM_LIB_CODEGEN_LCCRT_LCCRTADDRDEP_H

#include "llvm/Support/Compiler.h"
#include "llvm/ADT/bit.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/ModuleSlotTracker.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Pass.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/MC/MCValue.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCMachObjectWriter.h"

#ifdef LLVM_WITH_LCCRT
#include "lccrt.h"

#include <map>

namespace llvm {
class Triple;
class DataLayout;
class GlobalAlias;
class MCAsmLayout;

class LLVM_LIBRARY_VISIBILITY LccrtAddrDepEmitter {
    lccrt_context_ptr ctx;
    lccrt_module_ptr m;

    // Соответствие TBAA-тип -> индекс из TBAA-пула.
    std::map<const MDNode *, int64_t> TbaaToIndex;
    std::map<const MDNode *, lccrt_einfo_reference_t> TbaaToEinfo;

    // Стандартные einfo-типы.
    lccrt_einfo_tydescr_ptr ety_i64;
    lccrt_einfo_tydescr_ptr ety_raw;
    lccrt_einfo_tydescr_ptr ety_arri64;

    // Категория для TBAA-результатов.
    lccrt_einfo_category_t ecat_addrdep;
    // Данные TBAA-типа.
    lccrt_einfo_tydescr_ptr ety_tbaa; // основной элемент ациклического TBAA-графа
    lccrt_einfo_field_id_t efi_tbaa_name; // (необязательное) название
    lccrt_einfo_field_id_t efi_tbaa_idxs; // массив индексов вложенных TBAA-типов
    lccrt_einfo_field_id_t efi_tbaa_offs; // массив смещений вложенных TBAA-типов
    lccrt_einfo_tydescr_ptr ety_tbaas;
    // Пул TBAA-типов.
    lccrt_einfo_reference_t tbaa_pool;

    // Данные TBBA-тэга.
    lccrt_einfo_tydescr_ptr ety_tbaatag;
    lccrt_einfo_field_id_t efi_tbaatag_base; // индекс базового типа
    lccrt_einfo_field_id_t efi_tbaatag_accs; // индекс вложенного типа доступа
    lccrt_einfo_field_id_t efi_tbaatag_offi; // смещение (0 если первые два типа совпадают)

  public:
    LccrtAddrDepEmitter();
    virtual ~LccrtAddrDepEmitter();

  public:
    void initModule( lccrt_module_ptr m);
    void processInstruction( lccrt_oper_ptr oper, llvm::Instruction *I);

  private:
    int64_t evalTbaaType( Instruction *I, const MDNode *MD);
};

} // namespace llvm

#endif /* LLVM_WITH_LCCRT */

#endif /* LLVM_LIB_CODEGEN_LCCRT_LCCRTADDRDEP_H */
