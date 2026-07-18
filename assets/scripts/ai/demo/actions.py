from ai.core.rule.common import Action, Context

class DummyAction(Action):
    def take_action(self, context: Context):
        pass


assign_idle_villagers_to_farm = DummyAction()
cancel_military_productions = DummyAction()