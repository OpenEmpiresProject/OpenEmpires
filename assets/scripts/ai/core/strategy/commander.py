from typing import List

from ai.core.rule.rule_engine import Context, GameState, RuleEngine, Rule
from ai.core.strategy.strategic_advisor_base import StrategicAdvisorBase
from ai.core.strategy.strategy_base import StrategyBase


class Commander:
    def __init__(self):
        self.strategy = None
        self.advisors: List[StrategicAdvisorBase] = []
        self.game_state = GameState()
        self.rule_engine = RuleEngine(self.game_state)

    def set_strategy(self, strategy: StrategyBase) -> None:
        self.strategy = strategy
        self.advisors = strategy.get_advisors()

    def set_rules(self, rules: List[Rule]) -> None:
        self.rule_engine.set_rules(rules)

    def get_game_state(self) -> GameState:
        return self.game_state

    def evaluate(self) -> None:
        for advisor in self.advisors:
            advisor.evaluate(self.rule_engine.context)

    def execute(self):
        self.rule_engine.execute()