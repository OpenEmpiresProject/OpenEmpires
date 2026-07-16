from .rule_engine import *

class DoNothingAction(Action):
    def take_action(self, context: Context):
        pass

do_nothing = DoNothingAction()
