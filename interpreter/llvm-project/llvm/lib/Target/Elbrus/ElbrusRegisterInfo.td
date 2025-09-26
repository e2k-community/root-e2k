//=== ElbrusRegisterInfo.td - Describe the Elbrus Register File - tablegen -==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file describes the Elbrus Register file, defining the registers themselves,
// aliases between the registers, and the register classes built out of the
// registers.
//
//===----------------------------------------------------------------------===//

class E2KReg<string n> : Register<n> {
  let Namespace = "Elbrus";
}

//===----------------------------------------------------------------------===//
//  Register definitions...
//

def R0 : E2KReg<"r0">;

//===----------------------------------------------------------------------===//
// Register Class Definitions... now that we have all of the pieces, define the
// top-level register classes.  The order specified in the register list is
// implicitly defined to be the register allocation order.
//

def GR8  : RegisterClass<"Elbrus", [i8], 8,
                         (add R0)>;

def GR16 : RegisterClass<"Elbrus", [i16], 16,
                         (add R0)>;

def GR32 : RegisterClass<"Elbrus", [i32], 32,
                         (add R0)>;

def GR64 : RegisterClass<"Elbrus", [i64], 64,
                         (add R0)>;

def GR128 : RegisterClass<"Elbrus", [v4f32, v2f64, v16i8, v8i16, v4i32, v2i64, f128],
                          128, (add R0)>;

def GR80  : RegisterClass<"Elbrus", [f80], 32, (add R0)>;

def GR256 : RegisterClass<"Elbrus", [v8f32, v4f64, v32i8, v16i16, v8i32, v4i64],
                          256, (add R0)>;

def GR512 : RegisterClass<"Elbrus", [v16f32, v8f64, v64i8, v32i16, v16i32, v8i64],
                          512, (add R0)>;

