import lldb
import sys
import itertools


def __lldb_init_module(debugger, *_args):
    """Register pretty printers."""
    debugger.HandleCommand(
        "type synthetic add -x '^std::(.*::)*span<.+>$' -l std_pretty_printers.StdSpanSynthProvider")
    debugger.HandleCommand(
        "type summary add -x '^std::(.*::)*span<.+>$' -l std_pretty_printers.StdSpanSummaryProvider")


class StdSpanSynthProvider:
    def __init__(self, valobj, internal_dict):
        self.valobj = valobj
        self.update()

    def num_children(self):
        return self.size.GetValueAsUnsigned(0)

    def get_child_index(self, name):
        try:
            return int(name.lstrip('[').rstrip(']'))
        except:
            return -1

    def get_child_at_index(self, index):
        # Do bounds checking.
        if index < 0:
            return None

        if index >= self.num_children():
            return None

        offset = index * self.type_size

        return self.data.CreateChildAtOffset('[' + str(index) + ']',
                                             offset, self.data_type)

    def update(self):
        self.data = self.valobj.EvaluateExpression('data()')
        self.size = self.valobj.EvaluateExpression('size()')
        self.data_type = self.valobj.GetType().GetTemplateArgumentType(0)
        self.type_size = self.data_type.GetByteSize()


def StdSpanSummaryProvider(valobj, dict):
    synth = StdSpanSynthProvider(valobj, dict)

    synth.update()

    return f"(size = {synth.size})"
