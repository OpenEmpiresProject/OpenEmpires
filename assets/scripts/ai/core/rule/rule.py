from enum import Enum, IntEnum
from typing import List, cast

from ai.core.rule.common import Context
from ai.core.rule.operations import When


class Then:
    def __init__(self, *actions):
        self.actions = list(actions)


class Tags:
    def __init__(self, *tags: str):
        self.tags: List[str] = tags


class Name:
    def __init__(self, name: str):
        self.name = name


class TriggerMode(Enum):
    LEVEL_TRIGGER = 1
    EDGE_TRIGGER = 2


class Rule:

    def __init__(self, when: When, then: Then, *options):
        self.when = when
        self.then = then
        self.tags: List[str] = ["default"]
        self.name = None
        self.trigger = TriggerMode.LEVEL_TRIGGER
        self.disabled = False
        self.was_true = False

        for option in options:
            if isinstance(option, Tags):
                self.tags = cast(Tags, option).tags
            elif isinstance(option, Name):
                self.name = cast(Name, option).name
            elif isinstance(option, TriggerMode):
                self.trigger = cast(TriggerMode, option)

    def disable(self):
        self.disabled = True

    def execute(self, context: Context):
        context.rule = self

        if not self.disabled and self.when.is_true(context):
            # Since the condition satisfied, if the rule is set for
            # level triggering, actions should be executed always.
            # If it is Edge-Triggered, then only false->true transition
            # should execute actions.
            can_execute = False
            if self.trigger == TriggerMode.LEVEL_TRIGGER:
                can_execute = True
            elif self.trigger == TriggerMode.EDGE_TRIGGER:
                if not self.was_true: # state transition. i.e. edge
                    can_execute = True

            self.was_true = True

            if can_execute:
                for action in self.then.actions:
                    action.take_action(context)
                if context.verbose:
                    print(f"[RuleEngine] Rule {self.name} executed")
        else:
            # Do not change state if the rule is disabled.
            if not self.disabled:
                self.was_true = False

            if context.verbose:
                if self.disabled:
                    print(f"[RuleEngine] Rule {self.name} is disabled")
                else:
                    print(f"[RuleEngine] Rule {self.name} failed. Failed conditions;")
                    for error in context.errors:
                        print(f"[RuleEngine]        {error.get_message()}")


class EdgeTriggeredRule(Rule):
    def __init__(self, when: When, then: Then, *options):
        super().__init__(when, then, TriggerMode.EDGE_TRIGGER, *options)
