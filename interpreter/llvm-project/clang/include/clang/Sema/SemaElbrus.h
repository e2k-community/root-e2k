//===----- SemaElbrus.h ---- Elbrus target-specific routines --*- C++ -*---===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
/// This file declares semantic analysis functions specific to Elbrus.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_SEMA_SEMAELBRUS_H
#define LLVM_CLANG_SEMA_SEMAELBRUS_H

#include "clang/Sema/SemaBase.h"

namespace clang {
class Decl;
class ParsedAttr;

class SemaElbrus : public SemaBase {
public:
  SemaElbrus(Sema &S);

  bool checkFuncDecl(Decl *D, const ParsedAttr &AL);
  void handleInterruptAttr(Decl *D, const ParsedAttr &AL);
  void handleElbrusCheckStackAttr(Decl *D, const ParsedAttr &AL);
  void handleElbrusNoLoopApbAttr(Decl *D, const ParsedAttr &AL);
  void handleElbrusNoSemiSpecLdAttr(Decl *D, const ParsedAttr &AL);
};

} // namespace clang

#endif // LLVM_CLANG_SEMA_SEMAELBRUS_H
