from ai.core.rule.common import here
from ai.core.rule.memory import update_memory
from ai.core.rule.operations import always_true, always_false, When
from ai.core.rule.rule import Name, Rule, Then
from ai.core.rule.utility_actions import do_nothing
from ai.demo.actions import assign_idle_villagers_to_farm, cancel_military_productions
from ai.demo.facts import *

rules = [
    Rule(
        When(always_false),
        Then(do_nothing),
        Name(here())
    ),
    Rule(
        When(food < 100, idle_villagers > 0),
        Then(
            assign_idle_villagers_to_farm,
            update_memory(warning_starving, True)),
        Name(here())
    ),
    Rule(
        When(warning_starving),
        Then(
            cancel_military_productions),
        Name(here())
    ),
]