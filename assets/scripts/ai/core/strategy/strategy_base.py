from abc import ABC, abstractmethod
from typing import List

from ai.core.strategy.strategic_advisor_base import StrategicAdvisorBase


class StrategyBase(ABC):
    @abstractmethod
    def get_advisors(self) -> List[StrategicAdvisorBase]:
        raise NotImplementedError()