"""
AI Rule Engine.
This module provides the foundational  capabilities to write rules for
AI. A rule primarily consists of pre-condition(s) and action(s). 

Constructs:
- RuleEngine
- GameState
- Rule
- Fact
- Memory
- Boolean operators
    - When (equals to And)
    - And
    - Or
    - Not
- always_true (built-in operation which evaluates to true)
- always_false (built-in operation which evaluates to false)
- disable_self (built-in action to disable the rule)
- update_memory (built-in action to update the memory)

Approach is;
- define `Fact`s and `Memory`s
- define custom `Action`s
- define `Rule`s using boolean conditions (comparing `Fact`s,`Memory`s) and actions (built-in or custom)
- register `Rule`s with RuleEngine
- in the game loop;
    - update `GameState`
    - call `RuleEngine`'s `execute`
    
Note: Frequency of execution can be any chosen value. It doesn't have to match actual frame-rate

Examples:
```
food = Fact("food")
class AssignIdleVillagerToBush:
    def take_action(self):
        # take necessary to talk the game and execute this action

# Just a lower case alias
assign_villager_to_bush = AssignIdleVillagerToBush

game_state = GameState()
rule_engine = RuleEngine(game_state)

rule_engine.set_rules([Rule(
    When(food < 100),
    Then(assign_villager_to_bush)
    Tags("economy", "dark_age_specific")
)])

# Inside game loop or custom loop
game_state.food = <get the value from game>
rule_engine.execute()

# Nested and other boolean operators
rule_engine.set_rules([Rule(
    Or(
        not_enough_food, 
        And(
            idle_villagers > 1, 
            food < 100
        )
    ),
    Then(assign_villager_to_bush)
    Tags("economy", "dark_age_specific")
)])

```
"""
import enum
from abc import ABC, abstractmethod
from dataclasses import dataclass
from typing import List, override, Any, cast, Optional
from ai.core.rule.common import *
from ai.core.rule.operations import When


class Then:
    def __init__(self, action: Action):
        self.action = action


class Tags:
    def __init__(self, *tags: str):
        self.tags = tags


class Rule:
    def __init__(self, when: When, then: Then, tags: Tags = None):
        self.when = when
        self.then = then
        self.disabled = False
        self.tags: List[str] = tags.tags if tags else ["default"]

    def disable(self):
        self.disabled = True

    def execute(self, context: Context):
        context.rule = self

        if not self.disabled and self.when.is_true(context):
            self.then.action.take_action(context)


class RuleEngine:
    def __init__(self, game_state: GameState):
        self.rules = []
        self.game_state = game_state
        self.context = Context(self.game_state)
        self.rules_by_tag = {}

    def add_rule(self, rule: Rule):
        for tag in rule.tags:
            self._register_tag(tag, rule)
        self.rules.append(rule)

    def set_rules(self, rules: List[Rule]):
        for rule in rules:
            for tag in rule.tags:
                self._register_tag(tag, rule)
        self.rules = rules

    def execute(self, tags: List[str] = None):
        rules = self.rules
        if tags:
            rules = self._get_rules_by_tags(tags)

        for rule in rules:
            if not rule.disabled:
                rule.execute(self.context)

    def execute_rules(self, rules: List[Rule]):
        for rule in rules:
            if not rule.disabled:
                rule.execute(self.context)

    def _register_tag(self, tag: str, rule: Rule):
        if tag not in self.rules_by_tag:
            self.rules_by_tag[tag] = set()

        self.rules_by_tag[tag].add(rule)

    def _get_rules_by_tag(self, tag: str) -> set[Rule]:
        return self.rules_by_tag.get(tag, set())

    def _get_rules_by_tags(self, tags: List[str]) -> set[Rule]:
        rules = set()
        for tag in tags:
            rules.update(self.rules_by_tag.get(tag, set()))
        return rules



