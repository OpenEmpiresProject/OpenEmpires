from typing import override, List

from ai.core.strategy.strategic_advisor_base import StrategicAdvisorBase
from ai.core.strategy.strategy_base import StrategyBase
from ai.demo.economy_advisor import EconomyAdvisor


class DemoStrategy(StrategyBase):

    @override
    def get_advisors(self) -> List[StrategicAdvisorBase]:
        return [EconomyAdvisor()]