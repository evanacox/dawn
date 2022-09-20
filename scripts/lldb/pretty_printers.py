import lldb
import sys
import itertools


def __lldb_init_module(debugger, *_args):
    """Register pretty printers."""
    debugger.HandleCommand(
        "type synthetic add -x '^dawn::OptionalPtr<.+>$' -l pretty_printers.OptionalPtrPrinter")

    debugger.HandleCommand(
        "type synthetic add -x '^dawn::APInt$' -l pretty_printers.APIntPrinter")


class APIntPrinter:
    def __init__(self, valobj, *_args):
        self.valobj = valobj
        self.update()

    def num_children(self):
        return 3

    def get_child_index(self, name):
        if name == "value":
            return 0
        elif name == "width":
            return 1
        elif name == "raw":
            return 2
        else:
            return None

    def get_child_at_index(self, index):
        if index == 0:
            return self.valobj.CreateValueFromData("value", self.value)
        elif index == 1:
            return self.valobj.CreateValueFromData("width", self.width)
        elif index == 2:
            return self.valobj.CreateValueFromData("raw", self.lo | (self.hi << 64))
        else:
            return None

    def has_children(self):
        return True

    def update(self):
        self.lo = self.valobj.GetChildMemberWithName("value_").GetChildMemberWithName("lo_").GetValueAsUnsigned()
        self.hi = self.valobj.GetChildMemberWithName("value_").GetChildMemberWithName("hi_").GetValueAsUnsigned()
        self.width = self.hi >> (7 * 8)
        self.value = self.lo & (2 ** self.width)


class OptionalPtrPrinter:
    def __init__(self, valobj, *_args):
        self.valobj = valobj
        self.update()

    def num_children(self):
        return 1

    def get_child_index(self, name):
        if name == "value":
            return 0
        else:
            return None

    def get_child_at_index(self, index):
        if index == 0:
            return self.value
        else:
            return None

    def has_children(self):
        return True

    def update(self):
        self.is_init = self.valobj.GetChildMemberWithName("ptr_").GetValueAsUnsigned() != 0
        self.value = None

        if self.is_init:
            temp_type = self.valobj.GetType().GetTemplateArgumentType(0)
            storage = self.valobj.GetChildMemberWithName("m_storage")
            self.value = storage.Cast(temp_type)
