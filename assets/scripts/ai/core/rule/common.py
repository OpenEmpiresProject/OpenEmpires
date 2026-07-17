import enum
from abc import ABC, abstractmethod
from typing import Any, override


class GameState:
    def __init__(self):
        self.fact_values = dict()

    def set_value(self, key: str, value: Any) -> None:
        self.fact_values[key] = value


class Context:
    def __init__(self, game_state: GameState):
        self.game_state = game_state
        self.rule = None
        self.memory_storage = dict()


class FactEvaluationType(enum.IntEnum):
    GT = 1
    LT = 2
    GE = 3
    LE = 4
    NE = 5
    EQ = 6
    BOOL = 7


class Action:
    def take_action(self, context: Context):
        raise NotImplementedError()


class Operand(ABC):
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
