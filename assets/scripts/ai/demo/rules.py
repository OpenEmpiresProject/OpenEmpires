from ai.core.rule.rule_engine import always_true, Rule, When, Then
from ai.core.rule.utility_actions import do_nothing

rules = [
    Rule(
        When(always_true),
        Then(do_nothing)
    ),

]