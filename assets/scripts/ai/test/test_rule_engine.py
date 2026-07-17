import dataclasses
import unittest
from unittest.mock import Mock

from ai.core.rule.timers import *
from ai.core.rule.utility_actions import *
from ai.core.rule.operations import *


Food = Fact("food")
Wood = Fact("wood")
Stone = Fact("stone")


class Target:
    pass


class AssignIdleVillagerTo(Action):
    def __init__(self, target: Target):
        self.target = target
        self.target.attended = False

    def take_action(self, context: Context):
        self.target.attended = True


class RuleEngineTests(unittest.TestCase):
    def setUp(self):
        pass

    def test_fact_immutability(self):
        with self.assertRaises(dataclasses.FrozenInstanceError):
            fact = Fact("food")
            fact.name = "stone"

    def test_fact_evaluation_construction(self):
        fact = Fact("something")
        fact_eval = FactEvaluationOperation(fact, FactEvaluationType.LT, 123)
        self.assertEqual(fact_eval.fact.name, "something")
        self.assertEqual(fact_eval.value_to_compare.value, 123)
        self.assertEqual(fact_eval.evaluation_type, FactEvaluationType.LT)

    def test_fact_evaluation_fact_not_found(self):
        game_state = GameState()
        context = Context(game_state)

        fact = Fact("something")
        fact_eval = FactEvaluationOperation(fact, FactEvaluationType.LT, 123)
        self.assertFalse(fact_eval.is_true(context))

    def test_fact_evaluation_happy_path(self):
        game_state = GameState()
        game_state.set_value("something", 100)
        context = Context(game_state)

        fact = Fact("something")
        fact_eval = FactEvaluationOperation(fact, FactEvaluationType.LT, 123)
        self.assertTrue(fact_eval.is_true(context))

    def test_and_operation_single_fact_pass(self):
        game_state = GameState()
        game_state.set_value("food", 100)
        context = Context(game_state)

        op = AndOperation(Food < 101)
        self.assertTrue(op.is_true(context))

    def test_and_operation_multiple_facts_pass(self):
        game_state = GameState()
        game_state.set_value("food", 100)
        game_state.set_value("wood", 200)
        context = Context(game_state)

        op = AndOperation(Food < 101, Wood < 201)
        self.assertTrue(op.is_true(context))

    def test_and_operation_multiple_facts_fail(self):
        game_state = GameState()
        game_state.set_value("food", 100)
        game_state.set_value("wood", 200)
        context = Context(game_state)

        op = AndOperation(Food < 101, Wood < 199)
        self.assertFalse(op.is_true(context))

    def test_or_operation_single_fact_pass(self):
        game_state = GameState()
        game_state.set_value("food", 100)
        context = Context(game_state)

        op = OrOperation(Food < 101)
        self.assertTrue(op.is_true(context))

    def test_or_operation_multiple_facts_pass(self):
        game_state = GameState()
        game_state.set_value("food", 100)
        game_state.set_value("wood", 200)
        context = Context(game_state)

        op = OrOperation(Food < 99, Wood < 201)
        self.assertTrue(op.is_true(context))

    def test_or_operation_multiple_facts_fail(self):
        game_state = GameState()
        game_state.set_value("food", 100)
        game_state.set_value("wood", 200)
        context = Context(game_state)

        op = OrOperation(Food < 99, Wood < 199)
        self.assertFalse(op.is_true(context))

    def test_not_operation_pass(self):
        game_state = GameState()
        game_state.set_value("food", 100)
        context = Context(game_state)

        op = NotOperation(Food < 99)
        self.assertTrue(op.is_true(context))

    def test_not_operation_fail(self):
        game_state = GameState()
        game_state.set_value("food", 100)
        context = Context(game_state)

        op = NotOperation(Food < 101)
        self.assertFalse(op.is_true(context))

    def test_multiple_nested_operations_pass(self):
        game_state = GameState()
        game_state.set_value("food", 100)
        game_state.set_value("dood", 200)
        game_state.set_value("stone", 300)
        context = Context(game_state)

        op = AndOperation(Food < 101, OrOperation(Wood < 201, Stone > 299))
        self.assertTrue(op.is_true(context))

    def test_multiple_nested_operations_fail(self):
        game_state = GameState()
        game_state.set_value("food", 100)
        game_state.set_value("dood", 200)
        game_state.set_value("stone", 300)
        context = Context(game_state)

        op = AndOperation(Food < 99, OrOperation(Wood < 201, Stone > 299))
        self.assertFalse(op.is_true(context))

    def test_always_true(self):
        game_state = GameState()
        context = Context(game_state)

        op = AndOperation(always_true)
        self.assertTrue(op.is_true(context))

    def test_always_false(self):
        game_state = GameState()
        context = Context(game_state)

        op = AndOperation(always_false)
        self.assertFalse(op.is_true(context))

    def test_rule_happy_path(self):
        game_state = GameState()
        game_state.set_value("food", 100)
        context = Context(game_state)

        bush = Target()
        bush.attended = False

        rule = Rule(
            When(
                Food < 101
            ),
            Then(
                AssignIdleVillagerTo(bush)
            )
        )

        self.assertFalse(bush.attended)
        rule.execute(context)
        self.assertTrue(bush.attended)

    def test_disable_self(self):
        game_state = GameState()
        game_state.set_value("food", 100)
        context = Context(game_state)

        # Facts are true, should be disabled after first run
        rule1 = Rule(
            When(
                Food < 101
            ),
            Then(
                disable_self
            )
        )

        # Facts are false, shouldn't disabled
        rule2 = Rule(
            When(
                Food < 99
            ),
            Then(
                disable_self
            )
        )

        self.assertFalse(rule1.disabled)
        rule1.execute(context)
        self.assertTrue(rule1.disabled)

        self.assertFalse(rule2.disabled)
        rule2.execute(context)
        self.assertFalse(rule2.disabled)

    def test_memory_write(self):
        game_state = GameState()
        context = Context(game_state)

        goals_defend = Memory("goals.defend")

        rule = Rule(
            When(
                always_true
            ),
            Then(
                update_memory(goals_defend, 1)
            )
        )

        memory_value = context.memory_storage.get(goals_defend.name)
        self.assertEqual(memory_value, None)
        rule.execute(context)

        memory_value = context.memory_storage.get(goals_defend.name)
        self.assertEqual(memory_value, 1)

    def test_memory_read(self):
        game_state = GameState()
        context = Context(game_state)

        goals_defend = Memory("goals.defend")

        rule1 = Rule(
            When(
                always_true
            ),
            Then(
                update_memory(goals_defend, 1)
            )
        )

        do_nothing_mock = Mock()

        rule2 = Rule(
            When(
                goals_defend == 1
            ),
            Then(
                do_nothing_mock
            )
        )

        rule1.execute(context)
        memory_value = context.memory_storage.get(goals_defend.name, None)
        self.assertEqual(memory_value, 1)

        rule2.execute(context)
        do_nothing_mock.take_action.assert_called_once()

    def test_rule_tags(self):
        rule = Rule(
            When(
                always_true
            ),
            Then(
                do_nothing
            ),
            Tags("economy", "booming")
        )

        rule_engine = RuleEngine(GameState())
        rule_engine.add_rule(rule)
        rules = rule_engine._get_rules_by_tag("economy")
        self.assertTrue(len(rules) == 1)
        self.assertTrue(next(iter(rules)) == rule)

    def test_rule_engine_execute(self):
        action = Mock()

        rule = Rule(
            When(
                always_true
            ),
            Then(
                action
            )
        )

        rule_engine = RuleEngine(GameState())
        rule_engine.add_rule(rule)

        rule_engine.execute()
        action.take_action.assert_called_once()

    def test_rule_engine_execute_despite_tags(self):
        action = Mock()

        rule = Rule(
            When(
                always_true
            ),
            Then(
                action
            ),
            Tags("economy", "booming")
        )

        rule_engine = RuleEngine(GameState())
        rule_engine.add_rule(rule)

        rule_engine.execute()
        action.take_action.assert_called_once()

    def test_rule_engine_execute_with_tags(self):
        farm_action = Mock()
        attack_action = Mock()

        rules = [
            Rule(
                When(
                    always_true
                ),
                Then(
                    farm_action
                ),
                Tags("economy", "booming")
            ),
            Rule(
                When(
                    always_true
                ),
                Then(
                    attack_action
                ),
                Tags("military", "defense")
            )
        ]

        rule_engine = RuleEngine(GameState())
        rule_engine.set_rules(rules)

        rule_engine.execute(["economy", "nothing"])
        farm_action.take_action.assert_called_once()
        attack_action.take_action.assert_not_called()

    def test_fact_comparison_against_non_primitive(self):
        class Ages(Enum):
            DARK = auto()
            CASTLE = auto()
        age = Fact("age")

        game_state = GameState()
        game_state.set_value("age", Ages.DARK)
        context = Context(game_state)

        op = AndOperation(age == Ages.DARK)
        self.assertTrue(op.is_true(context))

    def test_fact_strong_typing(self):
        class Ages(Enum):
            DARK = auto()
            CASTLE = auto()
        age = Fact("age", Ages)

        with self.assertRaises(TypeError):
            AndOperation(age == 123)

        with self.assertRaises(TypeError):
            AndOperation(age == "abc")

        game_state = GameState()
        game_state.set_value("age", Ages.DARK)
        context = Context(game_state)

        op = AndOperation(age == Ages.DARK)
        self.assertTrue(op.is_true(context))

    def test_memory_strong_typing(self):
        class Ages(Enum):
            DARK = auto()
            CASTLE = auto()
        goal_age = Memory("goal.target_age", Ages)

        with self.assertRaises(TypeError):
            AndOperation(goal_age == 123)

        with self.assertRaises(TypeError):
            AndOperation(goal_age == "abc")

        game_state = GameState()
        context = Context(game_state)
        context.memory_storage["goal.target_age"] = Ages.DARK

        op = AndOperation(goal_age == Ages.DARK)
        self.assertTrue(op.is_true(context))

    def test_fact_evaluation_bool(self):
        defensive = Fact("defensive", bool)

        game_state = GameState()
        game_state.set_value("defensive", True)
        context = Context(game_state)

        fact_eval = FactEvaluationOperation(defensive, FactEvaluationType.BOOL, True)
        self.assertTrue(fact_eval.is_true(context))

        game_state.set_value("defensive", False)
        self.assertFalse(fact_eval.is_true(context))


    def test_fact_as_bool_operator(self):
        defensive = Fact("defensive", bool)

        game_state = GameState()
        game_state.set_value("defensive", True)
        context = Context(game_state)

        op = AndOperation(defensive.as_bool())
        self.assertTrue(op.is_true(context))

        # Fact as a bool cast style
        op = AndOperation(defensive)
        self.assertTrue(op.is_true(context))

        op = OrOperation(defensive)
        self.assertTrue(op.is_true(context))

        op = NotOperation(defensive)
        self.assertFalse(op.is_true(context))

    def test_none_bool_fact_error(self):
        idle_villagers = Fact("idle_villagers")

        game_state = GameState()
        game_state.idle_villagers = 1
        context = Context(game_state)

        with self.assertRaises(TypeError):
            AndOperation(idle_villagers.as_bool())

        with self.assertRaises(TypeError):
            AndOperation(idle_villagers)

        with self.assertRaises(TypeError):
            OrOperation(idle_villagers)

        with self.assertRaises(TypeError):
            NotOperation(idle_villagers)


    def test_memory_evaluation_bool(self):
        game_state = GameState()
        context = Context(game_state)
        context.memory_storage["defensive"] = True

        defensive = Memory("defensive", bool)
        memory_eval = FactEvaluationOperation(defensive, FactEvaluationType.BOOL, True)
        self.assertTrue(memory_eval.is_true(context))

        context.memory_storage["defensive"] = False
        self.assertFalse(memory_eval.is_true(context))


    def test_memory_as_bool_operator(self):
        defensive = Memory("defensive", bool)

        game_state = GameState()
        context = Context(game_state)
        context.memory_storage["defensive"] = True

        op = AndOperation(defensive.as_bool())
        self.assertTrue(op.is_true(context))

        # Fact as a bool cast style
        op = AndOperation(defensive)
        self.assertTrue(op.is_true(context))

        op = OrOperation(defensive)
        self.assertTrue(op.is_true(context))

        op = NotOperation(defensive)
        self.assertFalse(op.is_true(context))

    def test_none_bool_memory_error(self):
        idle_villagers = Memory("idle_villagers")

        game_state = GameState()
        context = Context(game_state)
        context.memory_storage["idle_villagers"] = True

        with self.assertRaises(TypeError):
            AndOperation(idle_villagers.as_bool())

        with self.assertRaises(TypeError):
            AndOperation(idle_villagers)

        with self.assertRaises(TypeError):
            OrOperation(idle_villagers)

        with self.assertRaises(TypeError):
            NotOperation(idle_villagers)

    def test_fact_as_both_operands(self):
        archers = Fact("archers")
        militia = Fact("militia")

        game_state = GameState()
        game_state.set_value("archers", 100)
        game_state.set_value("militia", 200)
        context = Context(game_state)

        op = AndOperation(archers < militia)
        self.assertTrue(op.is_true(context))

        op = AndOperation(archers <= militia)
        self.assertTrue(op.is_true(context))

        op = AndOperation(archers != militia)
        self.assertTrue(op.is_true(context))

        op = AndOperation(archers > militia)
        self.assertFalse(op.is_true(context))

        op = AndOperation(archers == militia)
        self.assertFalse(op.is_true(context))

        op = AndOperation(archers >= militia)
        self.assertFalse(op.is_true(context))


    def test_fact_incompatible_types(self):
        archers = Fact("archers")
        militia = Fact("militia")

        game_state = GameState()
        game_state.set_value("archers", 100)
        game_state.set_value("militia", 100.0)
        context = Context(game_state)

        op = AndOperation(archers < militia)

        with self.assertRaises(TypeError):
            op.is_true(context)

    def test_memory_as_both_operands(self):
        archers = Memory("archers")
        militia = Memory("militia")

        game_state = GameState()
        context = Context(game_state)
        context.memory_storage["archers"] = 100
        context.memory_storage["militia"] = 200

        op = AndOperation(archers < militia)
        self.assertTrue(op.is_true(context))

        op = AndOperation(archers <= militia)
        self.assertTrue(op.is_true(context))

        op = AndOperation(archers != militia)
        self.assertTrue(op.is_true(context))

        op = AndOperation(archers > militia)
        self.assertFalse(op.is_true(context))

        op = AndOperation(archers == militia)
        self.assertFalse(op.is_true(context))

        op = AndOperation(archers >= militia)
        self.assertFalse(op.is_true(context))

    def test_fact_and_memory_as_operands(self):
        archers = Fact("archers")
        militia = Memory("militia")

        game_state = GameState()
        game_state.set_value("archers", 100)
        context = Context(game_state)
        context.memory_storage["militia"] = 200

        # True

        op = AndOperation(archers < militia)
        self.assertTrue(op.is_true(context))

        op = AndOperation(archers <= militia)
        self.assertTrue(op.is_true(context))

        op = AndOperation(archers != militia)
        self.assertTrue(op.is_true(context))

        # False

        op = AndOperation(archers > militia)
        self.assertFalse(op.is_true(context))

        op = AndOperation(archers == militia)
        self.assertFalse(op.is_true(context))

        op = AndOperation(archers >= militia)
        self.assertFalse(op.is_true(context))

        # True; Switch sides

        op = AndOperation(militia > archers)
        self.assertTrue(op.is_true(context))

        op = AndOperation(militia >= archers)
        self.assertTrue(op.is_true(context))

        op = AndOperation(militia != archers)
        self.assertTrue(op.is_true(context))

        # False; Switch sides

        op = AndOperation(militia < archers)
        self.assertFalse(op.is_true(context))

        op = AndOperation(militia == archers)
        self.assertFalse(op.is_true(context))

        op = AndOperation(militia <= archers)
        self.assertFalse(op.is_true(context))

if __name__ == '__main__':
    unittest.main()
