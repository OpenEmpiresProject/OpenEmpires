from abc import ABC, abstractmethod

from ai.core.rule.rule_engine import Context


class StrategicAdvisorBase(ABC):

    @abstractmethod
    def evaluate(self, context: Context) -> None:
        raise NotImplementedError()
