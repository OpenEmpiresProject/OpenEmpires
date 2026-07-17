from .rule_engine import *

class DoNothingAction(Action):
    def take_action(self, context: Context):
        pass

do_nothing = DoNothingAction()


class DisableSelf(Action):
    @override
    def take_action(self, context: Context):
        context.rule.disable()

disable_self = DisableSelf()
