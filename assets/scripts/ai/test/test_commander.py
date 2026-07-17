import unittest
from unittest.mock import Mock

from ai.core.rule.rule_engine import *
from ai.core.rule.memory import *
from ai.core.rule.fact import *
from ai.core.rule.operations import *
from ai.core.rule.utility_actions import do_nothing
from ai.core.strategy.commander import Commander
from ai.core.strategy.strategic_advisor_base import StrategicAdvisorBase


class CommandTests(unittest.TestCase):

    def test_commander_execute(self):
        mock_action = Mock()

        rules = [
            Rule(
                When(always_true),
                Then(mock_action)
            )
        ]
        commander = Commander()
        commander.set_rules(rules)

        commander.execute()

        mock_action.take_action.assert_called_once()

    def test_commander_evaluate(self):
        advisor = Mock()
        strategy = Mock()
        strategy.get_advisors.return_value = [advisor]

        commander = Commander()
        commander.set_strategy(strategy)

        commander.evaluate()

        advisor.evaluate.assert_called_once()


    def test_commander_advisor_impact(self):
        mock_action = Mock()
        goal_defensive = Memory("goals.defensive")

        rules = [
            Rule(
                When(goal_defensive == 1),
                Then(mock_action)
            )
        ]

        class TestAdvisor(StrategicAdvisorBase):
            def evaluate(self, context: Context) -> None:
                context.memory_storage["goals.defensive"] = 1

        strategy = Mock()
        strategy.get_advisors.return_value = [TestAdvisor()]

        commander = Commander()
        commander.set_rules(rules)
        commander.set_strategy(strategy)

        commander.evaluate()
        commander.execute()

        mock_action.take_action.assert_called_once()



if __name__ == '__main__':
    unittest.main()
