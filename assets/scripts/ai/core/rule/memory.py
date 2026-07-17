from typing import cast
from dataclasses import dataclass

from ai.core.rule.common import *
from ai.core.rule.fact import *


@dataclass(frozen=True)
class Memory(Fact):
    @override
    def get_value(self, context: Context) -> Any:
        return context.memory_storage.get(self.name, None)

    # Note: Have to re-implement following again to get those evaluated correctly.
    # Otherwise, Fact's operator functions will not get called.

    def as_bool(self):
        return FactEvaluationOperation(self, FactEvaluationType.BOOL, True)

    def __gt__(self, other):
        return FactEvaluationOperation(self, FactEvaluationType.GT, other)

    def __lt__(self, other):
        return FactEvaluationOperation(self, FactEvaluationType.LT, other)

    def __ge__(self, other):
        return FactEvaluationOperation(self, FactEvaluationType.GE, other)

    def __le__(self, other):
        return FactEvaluationOperation(self, FactEvaluationType.LE, other)

    def __eq__(self, other):
        return FactEvaluationOperation(self, FactEvaluationType.EQ, other)

    def __ne__(self, other):
        return FactEvaluationOperation(self, FactEvaluationType.NE, other)


class UpdateMemory(Action):
    def __init__(self, memory: Memory, value: Any):
        self.memory = memory
        self.value = value

    def take_action(self, context: Context):
        context.memory_storage[self.memory.name] = self.value


update_memory = UpdateMemory
