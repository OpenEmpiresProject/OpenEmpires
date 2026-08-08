from enum import Enum
from typing import List

from ai.core.rule.common import Context
from ai.core.rule.operations import When


class Then:
    def __init__(self, *actions):
        self.actions = list(actions)


class Tags:
    def __init__(self, *tags: str):
        self.tags = tags


class TriggerMode(Enum):
    LEVEL_TRIGGER = 1
    EDGE_TRIGGER = 2


class Rule:
    def __init__(self, when: When, then: Then, tags: Tags | None = None, Name: str | None = None, Trigger: TriggerMode = TriggerMode.LEVEL_TRIGGER):
        self.when = when
        self.then = then
        self.disabled = False
        self.tags: List[str] = tags.tags if tags else ["default"]
        self.name = Name
        self.trigger = Trigger
        self.was_true = False

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
    def __init__(self, when: When, then: Then, tags: Tags | None = None, Name: str | None = None):
        super().__init__(when, then, tags, Name, Trigger=TriggerMode.EDGE_TRIGGER)
