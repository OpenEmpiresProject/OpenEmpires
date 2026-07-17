from dataclasses import dataclass
from typing import Any, Optional, override

from ai.core.rule.common import *


class FactEvaluationOperation(OperationBase):
    def __init__(self, fact: Fact, evaluation_type: FactEvaluationType, value: Any):
        self.fact = fact
        self.evaluation_type = evaluation_type
        if not isinstance(value, Operand):
            self.value_to_compare = ValueOperand(value)
        else:
            self.value_to_compare = value

        if self.evaluation_type == FactEvaluationType.BOOL:
            if not fact.data_type == bool:
                raise TypeError(f"Fact data type must be bool")
        else:
            if fact.data_type and not isinstance(value, fact.data_type):
                raise TypeError(f"{value} value must be of type {fact.data_type}")

    @override
    def is_true(self, context: Context) -> bool:
        left_value = self.fact.get_value(context)
        if left_value is None:
            print(f"{self.fact.name} not found in the storage")
            return False

        right_value = self.value_to_compare.get_value(context)

        if type(left_value) != type(right_value):
            raise TypeError(f"Incompatible left and right operand data types left:{left_value}, right:{right_value}")

        match self.evaluation_type:
            case FactEvaluationType.GT:
                return left_value > right_value
            case FactEvaluationType.LT:
                return left_value < right_value
            case FactEvaluationType.GE:
                return left_value >= right_value
            case FactEvaluationType.LE:
                return left_value <= right_value
            case FactEvaluationType.NE:
                return left_value != right_value
            case FactEvaluationType.EQ:
                return left_value == right_value
            case FactEvaluationType.BOOL:
                return left_value == True
            case _:
                raise TypeError(f"Unknown operation {self.evaluation_type}")


@dataclass(frozen=True)
class Fact(Operand):
    name: str
    data_type: Optional[Any] = None

    @override
    def get_value(self, context: Context) -> Any:
        return context.game_state.fact_values.get(self.name, None)

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
