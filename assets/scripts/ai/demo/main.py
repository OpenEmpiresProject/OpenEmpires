from ai.core.strategy.commander import Commander
from ai.demo.rules import rules
from ai.demo.strategy import DemoStrategy

if __name__ == "__main__":
    strategy = DemoStrategy()
    commander = Commander()
    commander.set_rules(rules)
    commander.set_strategy(strategy)
    commander.rule_engine.set_verbose(True)

    commander.game_state.set_value("food", 90)
    commander.game_state.set_value("idle_villagers", 1)


    commander.execute()