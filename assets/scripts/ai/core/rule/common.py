import enum
from abc import ABC, abstractmethod
from typing import Any, override, List
import inspect
import os


class GameState:
    def __init__(self):
        self.fact_values = dict()

    def set_value(self, key: str, value: Any) -> None:
        self.fact_values[key] = value


class FactEvaluationType(enum.IntEnum):
    GT = 1
    LT = 2
    GE = 3
    LE = 4
    NE = 5
    EQ = 6
    BOOL = 7

    @property
    def pretty_name(self) -> str:
        match self:
            case FactEvaluationType.GT:
                return ">"
            case FactEvaluationType.LT:
                return "<"
            case FactEvaluationType.GE:
                return ">="
            case FactEvaluationType.LE:
                return "<="
            case FactEvaluationType.NE:
                return "!="
            case FactEvaluationType.EQ:
                return "=="
            case FactEvaluationType.BOOL:
                return "=="

class Error:
    def __init__(self, left_name: str, right_name: str, evaluation_type: FactEvaluationType, left_value: Any = None, right_value: Any = None):
        self.left_name: str = left_name
        self.right_name: str = right_name
        self.left_value: Any = left_value
        self.right_value: Any = right_value
        self.evaluation_type: FactEvaluationType = evaluation_type

    def get_message(self):
        if self.right_name != "" and self.right_name is not None:
            formatted_right = f"{self.right_name} ({self.right_value})"
        elif self.right_value is not None:
            formatted_right = self.right_value
        else:
            formatted_right = ""
        evaluation_type_str = self.evaluation_type.pretty_name if self.evaluation_type is not None else ""
        left_value_str = f"({self.left_value})" if self.left_value is not None else ""
        return f"{self.left_name} {left_value_str} {evaluation_type_str} {formatted_right}"


class Context:
    def __init__(self, game_state: GameState):
        self.game_state = game_state
        self.rule = None
        self.memory_storage = dict()
        self.errors: List[Error] = []
        self.verbose = False


class Action:
    def take_action(self, context: Context):
        raise NotImplementedError()


class Operand(ABC):
    name: str = ""

    @abstractmethod
    def get_value(self, context: Context) -> Any:
        raise NotImplementedError()


class ValueOperand(Operand):
    def __init__(self, value: Any):
        self.value = value

    @override
    def get_value(self, context: Context) -> Any:
        return self.value


class OperationBase(ABC):
    @abstractmethod
    def is_true(self, context: Context) -> bool:
        raise NotImplementedError()


def here():
    frame = inspect.currentframe().f_back
    frame = inspect.currentframe().f_back
    filename = os.path.basename(frame.f_code.co_filename)

    return f"{filename}:{frame.f_lineno}"