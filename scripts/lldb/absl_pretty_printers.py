import lldb
import sys
import itertools


def __lldb_init_module(debugger, *_args):
    debugger.HandleCommand(
        "type synthetic add -x '^absl::flat_hash_set<.+>$' -l absl_pretty_printers.AbslFlatHashMapSynthProvider")
    debugger.HandleCommand(
        "type synthetic add -x '^absl::InlinedVector<.+>$' -l absl_pretty_printers.AbslInlinedVectorSynthProvider")
    debugger.HandleCommand(
        "type summary add -x '^absl::InlinedVector<.+>$' -F absl_pretty_printers.AbslInlinedVectorSummaryProvider")


def absl_hash_values(val):
    size = val.GetChildMemberWithName("size_")

    if size.GetValueAsUnsigned(0) == 0:
        return

    table = val
    capacity = table.GetChildMemberWithName("capacity_").GetValueAsUnsigned(0)
    ctrl = table.GetChildMemberWithName("ctrl_")

    for item in range(capacity):
        ctrl_t = int(ctrl[item])
        if ctrl_t >= 0:
            yield table["slots_"][item]


class AbslFlatHashMapSynthProvider:
    def __init__(self, valobj, dict):
        self.valobj = valobj
        self.count = None
        self.ctrl = None
        self.key_ty = None
        self.value_ty = None
        self.data_ty = None
        self.data_size = None

    def update(self):
        self.count = None

        try:
            ty = self.valobj.GetType()

            self.ctrl = self.valobj.GetChildMemberWithName('ctrl_')

            # absl::flat_hash_map<K, V, Hash, Eq, Alloc>
            self.key_ty = ty.GetTemplateArgumentType(1)
            self.value_ty = ty.GetTemplateArgumentType(2)
            self.data_ty = self.valobj.GetChildMemberWithName('slots_').GetType().GetPointeeType()
            self.data_size = self.data_ty.GetByteSize()

        except:
            self.count = 0

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

        try:
            offset = index
            current = self.next

            while offset > 0:
                current = current.GetChildMemberWithName('_M_nxt')
                offset = offset - 1

            return current.CreateChildAtOffset('[' + str(index) + ']', self.skip_size, self.data_type)
        except:
            logger >> "Cannot get child"
            return None

    def num_children(self):
        if self.count is None:
            self.count = self.valobj.GetChildMemberWithName('size_').GetValueAsUnsigned(0)

        return self.count


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
