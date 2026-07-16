from typing import override

from ai.core.rule.rule_engine import Context
from ai.core.strategy.strategic_advisor_base import StrategicAdvisorBase


class EconomyAdvisor(StrategicAdvisorBase):
    @override
    def evaluate(self, context: Context) -> None:
        pass