"""
This module provide ability manage timers with rules. GameState must have 
elapsed_time: timedelta property set and maintained for timers to function
properly.

This module provides following;
- TimerFact for fact declaration
- enable_timer built-in action
- disable_timer built-in action
- TimerStatus enum

Examples:
```
# Create the fact (name doesn't matter)
attack_timer = TimerFact("timer.attack")

# Using built-in actions
Rule(
    When(some_condition),
    Then(enable_timer(attack_timer, 60))
)

Rule(
    When(some_condition),
    Then(disable_timer(attack_timer))
)

# Using TimerFact
Rule(
    When(attack_timer == TimerStatus.RUNNING),
    Then(some_action)
)
```
"""
from datetime import timedelta
from enum import Enum, auto
from typing import Dict
from unittest import case
from dataclasses import dataclass

from .rule_engine import *


class TimerStatus(Enum):
    RUNNING = auto()
    COMPLETED = auto()
    DISABLED = auto()


class Timer:
    def __init__(self, name: str, duration: timedelta, started_at: timedelta):
        self.name = name
        self.duration = duration
        self.started_at = started_at # Relative rule engine start time
        self.disabled = False

    def get_expired_at(self) -> timedelta:
        return self.started_at + self.duration


class TimerFactEvaluator(OperationBase):
    def __init__(self, name: str, status: TimerStatus):
        self.name = name
        self.status_to_compare : TimerStatus = status

    def is_true(self, context: ContextBase) -> bool:
        elapsed_time = getattr(context.game_state, "elapsed_time", None)
        if elapsed_time is None:
            return False

        context = cast(Context, context)
        timers: Dict[str, Timer] = getattr(context, "timers", None)
        if not timers:
            return False

        timer = timers.get(self.name, None)
        if not timer:
            return False

        to_be_expired_time = timer.get_expired_at()

        match self.status_to_compare:
            case TimerStatus.RUNNING:
                return not timer.disabled and to_be_expired_time > elapsed_time
            case TimerStatus.COMPLETED:
                return not timer.disabled and to_be_expired_time <= elapsed_time
            case TimerStatus.DISABLED:
                return timer.disabled
            case _:
                return False


@dataclass(frozen=True)
class TimerFact:
    name: str

    def __eq__(self, other):
        if isinstance(other, TimerStatus):
            return TimerFactEvaluator(self.name, cast(TimerStatus, other))
        raise ValueError(f"TimerFact should compared only against TimerStatus")


class EnableTimerAction(Action):
    def __init__(self, timer_fact: TimerFact, duration_seconds: int):
        self.timer_fact = timer_fact
        self.duration = timedelta(seconds=duration_seconds)

    def take_action(self, context: Context):
        if not hasattr(context, "timers"):
            context.timers = {}

        elapsed_time = getattr(context.game_state, "elapsed_time", None)
        if elapsed_time is not None:
            context.timers[self.timer_fact.name] = Timer(self.timer_fact.name, self.duration, elapsed_time)


class DisableTimerAction(Action):
    def __init__(self, timer_fact: TimerFact):
        self.timer_fact = timer_fact

    def take_action(self, context: Context):
        if not hasattr(context, "timers"):
            return
        timer = context.timers.get(self.timer_fact.name, None)
        if timer:
            timer.disabled = True
        # Should we throw if timer not found? But would make it hard to use


enable_timer = EnableTimerAction
disable_timer = DisableTimerAction
