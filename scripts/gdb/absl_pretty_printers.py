# ======---------------------------------------------------------------====== #
#  Copyright (c) 2022 Evan Cox <evanacox00@gmail.com>.                        #
#                                                                             #
#  Licensed under the Apache License, Version 2.0 (the "License");            #
#  you may not use this file except in compliance with the License.           #
#  You may obtain a copy of the License at                                    #
#                                                                             #
#      http://www.apache.org/licenses/LICENSE-2.0                             #
#                                                                             #
#  Unless required by applicable law or agreed to in writing, software        #
#  distributed under the License is distributed on an "AS IS" BASIS,          #
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   #
#  See the License for the specific language governing permissions and        #
#  limitations under the License.                                             #
# ======---------------------------------------------------------------====== #

import gdb.printing
import sys
import itertools


def absl_get_nodes(val):
    size = val["size_"]

    if size == 0:
        return

    table = val
    capacity = int(table["capacity_"])
    ctrl = table["ctrl_"]

    for item in range(capacity):
        ctrl_t = int(ctrl[item])
        if ctrl_t >= 0:
            yield table["slots_"][item]


def format_count(i):
    return '[%d]' % i


class AbslFlatHashSetPrinter(object):
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return "absl::flat_hash_set<%s> with %s elems " % (
            self.val.type.template_argument(0), self.val["size_"])

    @staticmethod
    def display_hint():
        return 'array'

    def children(self):
        counter = map(format_count, itertools.count())

        return zip(counter, absl_get_nodes(self.val))


class AbslFlatHashMapPrinter(object):
    def __init__(self, val):
        self.val = val

    @staticmethod
    def display_hint():
        return 'array'

    def to_string(self):
        return "absl::flat_hash_map<%s, %s> with %s elems " % (
            self.val.type.template_argument(0), self.val.type.template_argument(1),
            self.val["size_"])

    def children(self):
        counter = map(format_count, itertools.count())
        into_pair = map(lambda slot: slot['value'], absl_get_nodes(self.val))

        return zip(counter, into_pair)


class AbslSpanPrinter(object):
    def __init__(self, val):
        self.val = val

    @staticmethod
    def display_hint():
        return 'array'

    def to_string(self):
        return "absl::Span<%s> with %s elems " % (
            self.val.type.template_argument(0), self.val["len_"])

    def children(self):
        for i in range(int(self.val['len_'])):
            yield (format_count(i), self.val['ptr_'][i])


def build_pretty_printer():
    pp = gdb.printing.RegexpCollectionPrettyPrinter("abseil")
    pp.add_printer('flat_hash_map', 'absl::flat_hash_map', AbslFlatHashMapPrinter)
    pp.add_printer('flat_hash_set', 'absl::flat_hash_set', AbslFlatHashSetPrinter)
    pp.add_printer('Span', 'absl::Span', AbslSpanPrinter)
    return pp


gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer(), True)
print("Abseil GDB pretty-printers loaded")
