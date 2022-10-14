import lldb
import sys
import itertools


def __lldb_init_module(debugger, *_args):
    debugger.HandleCommand(
        "type synthetic add -x '^absl::flat_hash_map<.+>$' -l absl_pretty_printers.AbslFlatHashMapSynthProvider")
    debugger.HandleCommand(
        "type synthetic add -x '^absl::flat_hash_set<.+>$' -l absl_pretty_printers.AbslFlatHashSetSynthProvider")
    debugger.HandleCommand(
        "type synthetic add -x '^absl::InlinedVector<.+>$' -l absl_pretty_printers.AbslInlinedVectorSynthProvider")
    debugger.HandleCommand(
        "type summary add -x '^absl::InlinedVector<.+>$' -F absl_pretty_printers.AbslInlinedVectorSummaryProvider")


def absl_hash_values(val, slot_ty, pair_ty):
    if val.GetChildMemberWithName("size_").GetValueAsUnsigned(0) == 0:
        return

    ctrl = val.GetChildMemberWithName("ctrl_")
    ctrl_ty = ctrl.GetType().GetPointeeType()
    slots = val.GetChildMemberWithName("slots_")
    slot_size = slot_ty.GetByteSize()

    for i in range(val.GetChildMemberWithName("capacity_").GetValueAsUnsigned(0)):
        # note: not unsigned. certain 'negative' bit patterns map to tombstones/no value
        ctrl_t = ctrl.CreateChildAtOffset("", i, ctrl_ty).GetValueAsSigned(0)

        if ctrl_t >= 0:
            yield slots.CreateChildAtOffset(f"[{i}]", i * slot_size, pair_ty)


class AbslFlatHashMapSynthProvider:
    def __init__(self, valobj, dict):
        self.valobj = valobj

    def update(self):
        try:
            ty = self.valobj.GetType()

            self.size = self.valobj.GetChildMemberWithName('size_').GetValueAsUnsigned(0)

            # absl::flat_hash_map<K, V, Hash, Eq, Alloc>
            self.key_ty = ty.GetTemplateArgumentType(0)
            self.value_ty = ty.GetTemplateArgumentType(1)
            self.pair_ty = ty.GetTemplateArgumentType(4).GetTemplateArgumentType(0)
            self.ctrl = self.valobj.GetChildMemberWithName('ctrl_')
            self.data_ty = self.valobj.GetChildMemberWithName('slots_').GetType().GetPointeeType()
            self.pairs = [pair for pair in absl_hash_values(self.valobj, self.data_ty, self.pair_ty)]

        except:
            self.size = 0

        return False

    def get_child_index(self, name):
        try:
            return int(name.lstrip('[').rstrip(']'))
        except:
            return -1

    def get_child_at_index(self, index):
        if index < 0:
            return None

        if index >= self.num_children():
            return None

        return self.pairs[index]

    def num_children(self):
        if self.size is None:
            self.update()

        return self.size


class AbslFlatHashSetSynthProvider:
    def __init__(self, valobj, dict):
        self.valobj = valobj

    def update(self):
        try:
            ty = self.valobj.GetType()

            self.size = self.valobj.GetChildMemberWithName('size_').GetValueAsUnsigned(0)

            # absl::flat_hash_set<K, Hash, Eq, Alloc>
            self.key_ty = ty.GetTemplateArgumentType(0)
            self.data_ty = ty.GetTemplateArgumentType(4).GetTemplateArgumentType(0)
            self.ctrl = self.valobj.GetChildMemberWithName('ctrl_')
            self.pairs = [pair for pair in absl_hash_values(self.valobj, self.data_ty, self.key_ty)]

        except:
            self.size = 0

        return False

    def get_child_index(self, name):
        try:
            return int(name.lstrip('[').rstrip(']'))
        except:
            return -1

    def get_child_at_index(self, index):
        if index < 0:
            return None

        if index >= self.num_children():
            return None

        return self.pairs[index]

    def num_children(self):
        if self.size is None:
            self.update()

        return self.size


class AbslInlinedVectorSynthProvider:
    def __init__(self, valobj, dict):
        self.valobj = valobj
        self.size = None

    def num_children(self):
        if self.size is None:
            self.update()

        return self.size

    def get_child_at_index(self, index):
        if index < 0:
            return None
        if index >= self.num_children():
            return None
        try:
            return self.pdata.CreateChildAtOffset(
                '[' + str(index) + ']', index * self.data_size, self.data_ty)
        except:
            return None

    def update(self):
        # absl::InlinedVector<T, ...>
        self.data_ty = self.valobj.GetType().GetTemplateArgumentType(0)
        self.data_size = self.data_ty.GetByteSize()

        self.pdata = self.valobj.EvaluateExpression("data()")
        self.size = self.valobj.EvaluateExpression("size()").GetValueAsUnsigned()
        self.is_inlined = self.valobj.EvaluateExpression("storage_.metadata_.value").GetValueAsUnsigned() % 2 == 0

        capacity = self.valobj.EvaluateExpression(
            "storage_.GetInlinedCapacity()") if self.is_inlined else self.valobj.EvaluateExpression(
            "storage_.GetAllocatedCapacity()")

        self.capacity = capacity.GetValueAsUnsigned()

        return True


def AbslInlinedVectorSummaryProvider(valobj, dict):
    obj = AbslInlinedVectorSynthProvider(valobj, dict)
    obj.update()

    return f"(size = {obj.size}, capacity = {obj.capacity}, inline = {obj.is_inlined})"
