// Part of the Carbon Language project, under the Apache License v2.0 with LLVM
// Exceptions. See /LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef CARBON_TOOLCHAIN_SEM_IR_GENERIC_H_
#define CARBON_TOOLCHAIN_SEM_IR_GENERIC_H_

#include "common/set.h"
#include "toolchain/sem_ir/ids.h"

namespace Carbon::SemIR {

// Information for a generic entity, such as a generic class, a generic
// interface, or generic function.
//
// Note that this includes both checked generics and template generics.
struct Generic : public Printable<Generic> {
  auto Print(llvm::raw_ostream& out) const -> void {
    out << "{decl: " << decl_id << ", bindings: " << bindings_id << "}";
  }

  // Returns the eval block for the specified region of the generic. This is a
  // block of instructions that should be evaluated to compute the values and
  // instructions needed by that region of the generic.
  auto GetEvalBlock(GenericInstIndex::Region region) const -> InstBlockId {
    return region == GenericInstIndex::Region::Declaration
               ? decl_block_id
               : definition_block_id;
  }

  // The following members always have values, and do not change throughout the
  // lifetime of the generic.

  // The first declaration of the generic entity.
  InstId decl_id;
  // A block containing the IDs of compile time bindings in this generic scope.
  // The index in this block will match the `bind_index` in the name binding
  // instruction's `EntityName`.
  InstBlockId bindings_id;
  // The self instance of this generic, which is an instance where every generic
  // parameter's argument is that same parameter. For example, the self instance
  // of `Vector(T:! type)` is `Vector(T)`.
  GenericInstanceId self_instance_id;

  // The following members are set at the end of the corresponding region of the
  // generic.

  // The eval block for the declaration region of the generic.
  InstBlockId decl_block_id = InstBlockId::Invalid;
  // The eval block for the definition region of the generic.
  InstBlockId definition_block_id = InstBlockId::Invalid;
};

// Provides storage for generics.
class GenericStore : public ValueStore<GenericId> {
 public:
  // Get the self-instance for a generic, or an invalid instance for an invalid
  // generic ID.
  auto GetSelfInstance(GenericId id) -> GenericInstanceId {
    return id.is_valid() ? Get(id).self_instance_id
                         : GenericInstanceId::Invalid;
  }
};

// An instance of a generic entity, such as an instance of a generic function.
// For each construct that depends on a compile-time parameter in the generic
// entity, this contains the corresponding non-generic value. This includes
// values for the compile-time parameters themselves.
struct GenericInstance : Printable<GenericInstance> {
  auto Print(llvm::raw_ostream& out) const -> void {
    out << "{generic: " << generic_id << ", args: " << args_id << "}";
  }

  // Returns the value block for this region of the specific. This is a block
  // containing values and instructions produced by evaluating the corresponding
  // eval block of the generic within the context of this specific. These are
  // the constant values and types and the instantiated template-dependent
  // instructions that are used in this region of the specific.
  auto GetValueBlock(GenericInstIndex::Region region) const -> InstBlockId {
    return region == GenericInstIndex::Region::Declaration
               ? decl_block_id
               : definition_block_id;
  }

  // The generic that this is an instance of.
  GenericId generic_id;
  // Argument values, corresponding to the bindings in `Generic::bindings_id`.
  InstBlockId args_id;

  // The following members are set when the corresponding region of the specific
  // is resolved.

  // The value block for the declaration region of the specific.
  InstBlockId decl_block_id = InstBlockId::Invalid;
  // The value block for the definition region of the specific.
  InstBlockId definition_block_id = InstBlockId::Invalid;
};

// Provides storage for deduplicated instances of generics.
class GenericInstanceStore : public Yaml::Printable<GenericInstanceStore> {
 public:
  // Adds a new generic instance, or gets the existing generic instance for a
  // specified generic and argument list. Returns the ID of the generic
  // instance. The argument IDs must be for instructions in the constant block,
  // and must be a canonical instruction block ID.
  auto GetOrAdd(GenericId generic_id, InstBlockId args_id) -> GenericInstanceId;

  // Gets the specified generic instance.
  auto Get(GenericInstanceId instance_id) const -> const GenericInstance& {
    return generic_instances_.Get(instance_id);
  }

  // Gets the specified generic instance.
  auto Get(GenericInstanceId instance_id) -> GenericInstance& {
    return generic_instances_.Get(instance_id);
  }

  // These are to support printable structures, and are not guaranteed.
  auto OutputYaml() const -> Yaml::OutputMapping {
    return generic_instances_.OutputYaml();
  }

  // Collects memory usage of members.
  auto CollectMemUsage(MemUsage& mem_usage, llvm::StringRef label) const
      -> void;

  auto array_ref() const -> llvm::ArrayRef<GenericInstance> {
    return generic_instances_.array_ref();
  }
  auto size() const -> size_t { return generic_instances_.size(); }

 private:
  // Context for hashing keys.
  class KeyContext;

  ValueStore<GenericInstanceId> generic_instances_;
  Carbon::Set<GenericInstanceId, 0, KeyContext> lookup_table_;
};

// Gets the substituted value of a constant within a specified instance of a
// generic. Note that this does not perform substitution, and will return
// `Invalid` if the substituted constant value is not yet known.
auto GetConstantInInstance(const File& sem_ir, GenericInstanceId instance_id,
                           ConstantId const_id) -> ConstantId;

// Gets the substituted constant value of an instruction within a specified
// instance of a generic. Note that this does not perform substitution, and will
// return `Invalid` if the substituted constant value is not yet known.
auto GetConstantValueInInstance(const File& sem_ir,
                                GenericInstanceId instance_id, InstId inst_id)
    -> ConstantId;

// Gets the substituted value of a type within a specified instance of a
// generic. Note that this does not perform substitution, and will return
// `Invalid` if the substituted type is not yet known.
auto GetTypeInInstance(const File& sem_ir, GenericInstanceId instance_id,
                       TypeId type_id) -> TypeId;

}  // namespace Carbon::SemIR

#endif  // CARBON_TOOLCHAIN_SEM_IR_GENERIC_H_
