// Part of the Carbon Language project, under the Apache License v2.0 with LLVM
// Exceptions. See /LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef CARBON_TOOLCHAIN_CHECK_EVAL_H_
#define CARBON_TOOLCHAIN_CHECK_EVAL_H_

#include "toolchain/check/context.h"
#include "toolchain/sem_ir/ids.h"
#include "toolchain/sem_ir/inst.h"

namespace Carbon::Check {

// Determines the phase of the instruction `inst`, and returns its constant
// value if it has constant phase. If it has runtime phase, returns
// `SemIR::ConstantId::NotConstant`.
auto TryEvalInst(Context& context, SemIR::InstId inst_id, SemIR::Inst inst)
    -> SemIR::ConstantId;

// Evaluates the eval block for a region of a specific. Produces a block
// containing the evaluated constant values of the instructions in the eval
// block.
auto TryEvalBlockForSpecific(Context& context,
                             SemIR::GenericInstanceId specific_id,
                             SemIR::GenericInstIndex::Region region)
    -> SemIR::InstBlockId;

}  // namespace Carbon::Check

#endif  // CARBON_TOOLCHAIN_CHECK_EVAL_H_
