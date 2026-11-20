//===-- LccrtAddrDep.cpp - Lccrt emittion code ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the LccrtAddrDepEmitter class.
//
//===----------------------------------------------------------------------===//

#include <string.h>

#include <iostream>
#include <iomanip>

#include "LccrtEmitter.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Operator.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/CodeGen/Lccrt.h"

#ifdef LLVM_WITH_LCCRT
using namespace llvm;

#define DEBUG_TYPE "lccrt"

#ifdef NDEBUG
#define assert_define( x)
#else /* !NDEBUG */
#define assert_define( x) x
#endif /* NDEBUG */

LccrtAddrDepEmitter::LccrtAddrDepEmitter() {
    ctx = 0;
    m = 0;

    return;
} /* LccrtAddrDepEmitter::LccrtAddrDepEmitter */

LccrtAddrDepEmitter::~LccrtAddrDepEmitter() {
} /* LccrtAddrDepEmitter::~LccrtAddrDepEmitter */

/**
 * Подготовка к трансляции данных анализа для нового модуля.
 */
void
LccrtAddrDepEmitter::initModule( lccrt_module_ptr vm) {
    const char *names[256];
    lccrt_einfo_tydescr_ptr tydes[256];

    m = vm;
    ctx = lccrt_module_get_context( m);

    ecat_addrdep = lccrt_module_new_einfo_category( m, "ldst_addrdep");

    ety_i64 = lccrt_einfo_make_tydescr_i64( m);
    ety_raw = lccrt_einfo_make_tydescr_raw( m);
    ety_arri64 = lccrt_einfo_make_tydescr_array( m, ety_i64);

    names[0] = "name";    tydes[0] = ety_raw;
    names[1] = "idents";  tydes[1] = ety_arri64;
    names[2] = "offsets"; tydes[2] = ety_arri64;
    ety_tbaa = lccrt_einfo_make_tydescr_struct( m, "ldst_addrdep.tbaa", 3, names, tydes);
    efi_tbaa_name = lccrt_einfo_find_tydescr_field( ety_tbaa, "name");
    efi_tbaa_idxs = lccrt_einfo_find_tydescr_field( ety_tbaa, "idents");
    efi_tbaa_offs = lccrt_einfo_find_tydescr_field( ety_tbaa, "offsets");
    ety_tbaas = lccrt_einfo_make_tydescr_array( m, ety_tbaa);

    names[0] = "basety"; tydes[0] = ety_i64;
    names[1] = "accsty"; tydes[1] = ety_i64;
    names[2] = "offset"; tydes[2] = ety_i64;
    ety_tbaatag = lccrt_einfo_make_tydescr_struct( m, "ldst_addrdep.tbaatag", 3, names, tydes);
    efi_tbaatag_base = lccrt_einfo_find_tydescr_field( ety_tbaatag, "basety");
    efi_tbaatag_accs = lccrt_einfo_find_tydescr_field( ety_tbaatag, "accsty");
    efi_tbaatag_offi = lccrt_einfo_find_tydescr_field( ety_tbaatag, "offset");

    // Создаем пул для создания и индексации данных tbaa-типов.
    tbaa_pool = lccrt_einfo_new_array( ety_tbaas, 0);
    lccrt_module_set_einfo( m, ecat_addrdep, tbaa_pool);

    return;
} /* LccrtAddrDepEmitter::initModule */

/**
 * Транслировать TBAA-тип.
 */
int64_t
LccrtAddrDepEmitter::evalTbaaType( Instruction *I, const MDNode *MD) {
    lccrt_einfo_reference_t estr;
    int64_t r = -1;
    int num_args = MD->getNumOperands();
    auto it = TbaaToIndex.find( MD);

    if ( it != TbaaToIndex.end() ) {
        // TBAA-тип уже транслирован.
        r = it->second;

    } else if ( (num_args >= 1) && ((num_args - 1) % 2 == 0) ) {
        int i;
        lccrt_einfo_reference_t eref;
        int64_t idx0 = lccrt_einfo_get_num_args( tbaa_pool);
        lccrt_einfo_reference_t eaidx = lccrt_einfo_new_array( ety_arri64, num_args - 1);
        lccrt_einfo_reference_t eaoff = lccrt_einfo_new_array( ety_arri64, num_args - 1);
        MDString *str1 = (num_args == 0) ? 0 : dyn_cast<MDString>( MD->getOperand( 0));
        std::string str2 = str1->getString().data();

        // Работаем только с известными видами TBAA-информации.
        if ( num_args == 1 ) {
            if ( (str2 == "Simple C/C++ TBAA") || (str2 == "Simple C++ TBAA") )
            {
            } else {
                if ( 0 ) {
                    llvm::dbgs() << str2 << "\n";
                    errorDump( I);
                }
            }
        }

        // Запоминаем результаты трансляции.
        TbaaToIndex[MD] = idx0;

        // Создаем описание очередного типа.
        estr = lccrt_einfo_new_raw_by_string( m, str1->getString().data());
        eref = lccrt_einfo_new_struct( ety_tbaa);
        lccrt_einfo_set_field( eref, efi_tbaa_name, estr);
        lccrt_einfo_set_field( eref, efi_tbaa_idxs, eaidx);
        lccrt_einfo_set_field( eref, efi_tbaa_offs, eaoff);
        lccrt_einfo_push_elem( tbaa_pool, eref);

        // Транслируем все элементы типа.
        for ( i = 1; i < num_args; i += 2 ) {
            const MDOperand &opi0 = MD->getOperand( i + 0);
            const MDOperand &opi1 = MD->getOperand( i + 1);
            int64_t idxi = evalTbaaType( I, dyn_cast<MDNode>( opi0));
            int64_t offi = mdconst::extract<ConstantInt>( opi1)->getZExtValue();

            // Транслируем текущий элемент.
            if ( idxi < 0 ) {
                // Не удалось транслировать очередной элемент.
                break;
            } else {
                // Запоминаем результат трансляции очередного элемента.
                lccrt_einfo_set_elem( eaidx, i, lccrt_einfo_new_i64( idxi));
                lccrt_einfo_set_elem( eaoff, i, lccrt_einfo_new_i64( offi));
            }
        }

        if ( i == num_args ) {
            // Все элементы уcпешно транслированы.
            r = idx0;
        } else {
            // Транслировать элементы не удалось.
            TbaaToIndex[MD] = -1;
        }
    } else {
        errorDump( I);
    }

    return (r);
} /* LccrtAddrDepEmitter::evalTbaaType */

/**
 * Транслировать данные анализов для текущей llvm-операции.
 */
void
LccrtAddrDepEmitter::processInstruction( lccrt_oper_ptr oper, llvm::Instruction *I) {
    MDNode *MD = I->getMetadata( LLVMContext::MD_tbaa);

    if ( MD ) {
        int64_t idx0 = -1;
        int64_t idx1 = -1;
        int64_t offi = -1;
        auto baseTy = dyn_cast_or_null<MDNode>( MD->getOperand( 0));
        auto accessTy = dyn_cast_or_null<MDNode>( MD->getOperand( 1));
        uint64_t offset = mdconst::extract<ConstantInt>( MD->getOperand( 2))->getZExtValue();
        auto it = TbaaToEinfo.find( MD);

        //llvm::dbgs() << *I << "\n";
        if ( it != TbaaToEinfo.end() ) {
            lccrt_oper_set_einfo( oper, ecat_addrdep, it->second);
        } else {
            if ( (MD->getNumOperands() != 3) || !baseTy ) {
                errorDump( I);
            } else if ( baseTy == accessTy ) {
                if ( offset != 0 ) {
                    errorDump( I);
                } else {
                    idx0 = evalTbaaType( I, baseTy);
                    idx1 = idx0;
                    offi = 0;
                }
            } else {
                idx0 = evalTbaaType( I, baseTy);
                idx1 = evalTbaaType( I, accessTy);
                offi = offset;
            }

            if ( (idx0 >= 0) && (idx1 >= 0) ) {
                lccrt_einfo_reference_t eref;

                eref = lccrt_einfo_new_struct( ety_tbaatag);
                lccrt_einfo_set_field( eref, efi_tbaatag_base, lccrt_einfo_new_i64( idx0));
                lccrt_einfo_set_field( eref, efi_tbaatag_accs, lccrt_einfo_new_i64( idx1));
                lccrt_einfo_set_field( eref, efi_tbaatag_offi, lccrt_einfo_new_i64( offi));
                TbaaToEinfo[MD] = eref;
                lccrt_oper_set_einfo( oper, ecat_addrdep, eref);
            }
        }
    }

    return;
} /* LccrtAddrDepEmitter::processInstruction */

#endif /* LLVM_WITH_LCCRT */
