import unittest
from unittest.mock import Mock

from ai.timers import *


class CustomActionTests(unittest.TestCase):

    def setUp(self):
        game_state = GameState()
        self.engine = RuleEngine(game_state)

    def test_enable_timer_action(self):
        attack_timer = TimerFact("timer.attack")
        self.engine.game_state.elapsed_time = timedelta(seconds=0)

        rule = Rule(
            When(always_true),
            Then(enable_timer(attack_timer, 60))
        )

        self.engine.add_rule(rule)

        self.engine.game_state.elapsed_time = timedelta(seconds=100)

        self.engine.execute()
        self.assertIn("timer.attack", self.engine.context.timers)

        timer = self.engine.context.timers.get("timer.attack", None)
        self.assertIsNotNone(timer)
        self.assertFalse(timer.disabled)
        self.assertEqual(timer.started_at, timedelta(seconds=100))
        self.assertEqual(timer.get_expired_at(), timedelta(seconds=160))


    def test_timer_fire(self):
        attack_timer = TimerFact("timer.attack")
        self.engine.game_state.elapsed_time = timedelta(seconds=0)

        self.engine.add_rule(Rule(
            When(always_true),
            Then(enable_timer(attack_timer, 60))
        ))
        self.engine.execute()

        # Timer is not expired yet
        mock_action = Mock()
        self.engine.set_rules([Rule(
            When(attack_timer == TimerStatus.RUNNING),
            Then(mock_action)
        )])
        mock_action.take_action.assert_not_called()
        self.engine.execute()
        mock_action.take_action.assert_called_once()

        # Feed time to make timer expired
        self.engine.game_state.elapsed_time = timedelta(seconds=100)

        # Timer should be expired now
        mock_action_2 = Mock()
        self.engine.set_rules([Rule(
            When(attack_timer == TimerStatus.COMPLETED),
            Then(mock_action_2)
        )])
        mock_action_2.take_action.assert_not_called()
        self.engine.execute()
        mock_action_2.take_action.assert_called_once()

    def test_fact_evaluation_order_change(self):
        attack_timer = TimerFact("timer.attack")
        self.engine.game_state.elapsed_time = timedelta(seconds=0)

        rule = Rule(
            When(always_true),
            Then(enable_timer(attack_timer, 60))
        )

        self.engine.add_rule(rule)
        self.engine.execute()

        # Timer is not expired yet
        mock_action = Mock()
        self.engine.set_rules([Rule(
            When(TimerStatus.RUNNING == attack_timer),
            Then(mock_action)
        )])
        mock_action.take_action.assert_not_called()
        self.engine.execute()
        mock_action.take_action.assert_called_once()

    def test_fact_evaluation_only_against_status(self):
        attack_timer = TimerFact("timer.attack")
        mock_action = Mock()

        with self.assertRaises(ValueError):
            self.engine.set_rules([Rule(
                When(attack_timer == 123),
                Then(mock_action)
            )])

    def test_fact_evaluation_unsupported_comparators(self):
        attack_timer = TimerFact("timer.attack")
        mock_action = Mock()

        with self.assertRaises(TypeError):
            self.engine.set_rules([Rule(
                When(attack_timer >= TimerStatus.RUNNING),
                Then(mock_action)
            )])

    def test_disable_timer_action(self):
        attack_timer = TimerFact("timer.attack")
        self.engine.game_state.elapsed_time = timedelta(seconds=0)

        # Arrange - Start timer
        self.engine.set_rules([Rule(
            When(always_true),
            Then(enable_timer(attack_timer, 60))
        )])
        self.engine.execute()

        # Act - Disable timer
        self.engine.set_rules([Rule(
            When(always_true),
            Then(disable_timer(attack_timer))
        )])
        self.engine.execute()

        # Assert
        mock_action = Mock()
        self.engine.set_rules([Rule(
            When(attack_timer == TimerStatus.DISABLED),
            Then(mock_action)
        )])
        mock_action.take_action.assert_not_called()
        self.engine.execute()
        mock_action.take_action.assert_called_once()

if __name__ == '__main__':
    unittest.main()
