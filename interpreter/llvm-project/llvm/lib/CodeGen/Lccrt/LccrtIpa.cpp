//===-- LccrtIpa.cpp - Lccrt emittion ipa-results ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the LccrtModuleIpaEmitter/LccrtFunctionIpaEmitter classes.
//
//===----------------------------------------------------------------------===//

#include <iostream>

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instruction.h"
#include "LccrtIpa.h"

#ifdef LLVM_WITH_LCCRT
using namespace llvm;

MetaAliases *
wrapInArray(void *aliases, unsigned int length)
{
    auto result = (MetaAliases *) malloc(sizeof( MetaAliases));
    if (result) {
        result->items = aliases;
        result->length = length;
    }
    return result;
}

void
func_metadata_delete(lccrt_function_ptr f, uintptr_t value)
{
    auto *a = (MetaAliases *) value;
    auto *aliases = (AliasResult *) a->items;
    free(aliases);
    free(a);
}

void
oper_metadata_delete(lccrt_oper_ptr oper, uintptr_t value)
{
    auto id = (unsigned int *) value;
    free(id);
}

IPAResults *
LccrtModuleIpaEmitter::findIPAResults( Pass *pass)
{
    LccrtIpaPass *ipa_pass = pass->getAnalysisIfAvailable<LccrtIpaPass>();

    return (ipa_pass ? ipa_pass->getIPAResults() : 0);
} /* LccrtModuleIpaEmitter::findIPAResults */

LccrtModuleIpaEmitter::LccrtModuleIpaEmitter( Pass *parentPass) :
    parentPass( parentPass),
    ipaResults( findIPAResults( parentPass)),
    metadata(0)
{
} /* LccrtModuleIpaEmitter::LccrtModuleIpaEmitter */

void
LccrtModuleIpaEmitter::open( lccrt_module_ptr m, const Module *)
{
#if 0
    lccrt_mmdt_t *mmdt = (lccrt_mmdt_t *) malloc(sizeof(lccrt_mmdt_t));
    if (mmdt) {
        mmdt->delete_module = 0;
        mmdt->delete_func = func_metadata_delete;
        mmdt->delete_oper = oper_metadata_delete;
        metadata = lccrt_module_new_metadata( m, "llvm_ipa", 0, mmdt);
    }
#endif
} /* LccrtModuleIpaEmitter::open */

void
LccrtModuleIpaEmitter::close()
{
    return;
} /* LccrtModuleIpaEmitter::close */

intptr_t
LccrtModuleIpaEmitter::getMetadata() const
{
    return metadata;
}

const FunctionIPAResults *
LccrtModuleIpaEmitter::getFunctionIPAResults(GlobalValue::GUID functionGUID) const
{
    if (!ipaResults) {
        return nullptr;
    }
    auto found = ipaResults->find(functionGUID);
    if (found != ipaResults->end()) {
        return &found->second;
    }
    return nullptr;
}

LccrtFunctionIpaEmitter::LccrtFunctionIpaEmitter( LccrtModuleIpaEmitter *mipa,
                                                  lccrt_function_ptr f,
                                                  const Function *F) :
    mipa(mipa),
    metadata(0),
    functionIPAResults(nullptr),
    aliases(nullptr)
{
#if 0
    if (mipa) {
        metadata = mipa->getMetadata();
        functionIPAResults = F->hasName() ? mipa->getFunctionIPAResults(F->getGUID()) : 0;
    }
    void *a = nullptr;
    if (functionIPAResults) {
        a = functionIPAResults->getAliases();
    }
    if (a) {
        aliases = wrapInArray(a, functionIPAResults->length());
    }
    if (aliases) {
        lccrt_function_set_metadata( f, metadata, (uintptr_t) aliases);
    }
#endif
} /* LccrtFunctionIpaEmitter::LccrtFunctionIpaEmitter */

void
LccrtFunctionIpaEmitter::setOperIpaResult( lccrt_oper_ptr oper, const Instruction &Inst)
{
#if 0
    if (!metadata || !functionIPAResults) {
        return;
    }
    if (!isa<LoadInst>(Inst) && !isa<StoreInst>(Inst)) {
        return;
    }
    auto length = functionIPAResults->length();
    for (size_t i = 0; i < length; ++i) {
        auto loc = MemoryLocation::get(&Inst);
        if (functionIPAResults->values[i] == loc) {
            auto id = (unsigned int *) malloc(sizeof(unsigned int));
            if (id) {
                *id = i;
                lccrt_oper_set_metadata( oper, metadata, (uintptr_t) id);
                return;
            }
        }
    }
    //std::string name;
    //raw_string_ostream ros(name);
    //op->printAsOperand(ros, false);
    //std::cout << ros.str() << '\n';
#endif
} /* LccrtFunctionIpaEmitter::LccrtFunctionIpaEmitter */
#endif /* LLVM_WITH_LCCRT */
