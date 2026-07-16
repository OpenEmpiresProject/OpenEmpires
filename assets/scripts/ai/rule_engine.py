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


class GameState:
    pass


class ContextBase:
    def __init__(self, game_state: GameState):
        self.game_state = game_state
        self.rule = None


class FactEvaluationType(enum.IntEnum):
    GT = 1
    LT = 2
    GE = 3
    LE = 4
    NE = 5
    EQ = 6


class OperationBase(ABC):
    @abstractmethod
    def is_true(self, context: ContextBase) -> bool:
        raise NotImplementedError()


class FactEvaluationOperation(OperationBase):
    def __init__(self, name: str, evaluation_type: FactEvaluationType, value: Any, data_type: Optional[Any] = None):
        self.fact_name = name
        self.evaluation_type = evaluation_type
        self.value_to_compare = value

        if data_type and not isinstance(value, data_type):
            raise TypeError(f"{value} value must be of type {data_type}")

    def _is_true(self, fact_storage: Any) -> bool:
        if not hasattr(fact_storage, self.fact_name):
            print(f"{self.fact_name} not found in the storage")
            return False

        match self.evaluation_type:
            case FactEvaluationType.GT:
                return getattr(fact_storage, self.fact_name) > self.value_to_compare
            case FactEvaluationType.LT:
                return getattr(fact_storage, self.fact_name) < self.value_to_compare
            case FactEvaluationType.GE:
                return getattr(fact_storage, self.fact_name) >= self.value_to_compare
            case FactEvaluationType.LE:
                return getattr(fact_storage, self.fact_name) <= self.value_to_compare
            case FactEvaluationType.NE:
                return getattr(fact_storage, self.fact_name) != self.value_to_compare
            case FactEvaluationType.EQ:
                return getattr(fact_storage, self.fact_name) == self.value_to_compare
            case _:
                raise TypeError(f"Unknown operation {self.evaluation_type}")

    @override
    def is_true(self, context: ContextBase) -> bool:
        return self._is_true(context.game_state)


class AndOperation(OperationBase):
    def __init__(self, *operations):
        self.operations : List[OperationBase] = list(operations)

    @override
    def is_true(self, context: ContextBase) -> bool:
        for operation in self.operations:
            if not operation.is_true(context):
                return False
        return True


class OrOperation(OperationBase):
    def __init__(self, *operations):
        self.operations : List[OperationBase] = list(operations)

    @override
    def is_true(self, context: ContextBase) -> bool:
        for operation in self.operations:
            if operation.is_true(context):
                return True
        return False


class NotOperation(OperationBase):
    def __init__(self, operation):
        self.operation : OperationBase = operation

    @override
    def is_true(self, context: ContextBase) -> bool:
        return not self.operation.is_true(context)


class AlwaysFalseOperation(OperationBase):
    @override
    def is_true(self, context: ContextBase) -> bool:
        return False


class AlwaysTrueOperation(OperationBase):
    @override
    def is_true(self, context: ContextBase) -> bool:
        return True


@dataclass(frozen=True)
class Fact:
    name: str
    data_type: Optional[Any] = None

    def __gt__(self, other):
        return FactEvaluationOperation(self.name, FactEvaluationType.GT, other, self.data_type)

    def __lt__(self, other):
        return FactEvaluationOperation(self.name, FactEvaluationType.LT, other, self.data_type)

    def __ge__(self, other):
        return FactEvaluationOperation(self.name, FactEvaluationType.GE, other, self.data_type)

    def __le__(self, other):
        return FactEvaluationOperation(self.name, FactEvaluationType.LE, other, self.data_type)

    def __eq__(self, other):
        return FactEvaluationOperation(self.name, FactEvaluationType.EQ, other, self.data_type)

    def __ne__(self, other):
        return FactEvaluationOperation(self.name, FactEvaluationType.NE, other, self.data_type)


@dataclass(frozen=True)
class Memory:
    name: str
    data_type: Optional[Any] = None

    def __gt__(self, other):
        return MemoryEvaluationOperation(self.name, FactEvaluationType.GT, other, self.data_type)

    def __lt__(self, other):
        return MemoryEvaluationOperation(self.name, FactEvaluationType.LT, other, self.data_type)

    def __ge__(self, other):
        return MemoryEvaluationOperation(self.name, FactEvaluationType.GE, other, self.data_type)

    def __le__(self, other):
        return MemoryEvaluationOperation(self.name, FactEvaluationType.LE, other, self.data_type)

    def __eq__(self, other):
        return MemoryEvaluationOperation(self.name, FactEvaluationType.EQ, other, self.data_type)

    def __ne__(self, other):
        return MemoryEvaluationOperation(self.name, FactEvaluationType.NE, other, self.data_type)


class MemoryStorage:
    pass


class Context(ContextBase):
    def __init__(self, game_state: GameState):
        super().__init__(game_state)
        self.memory_storage = MemoryStorage()

    def get_memory(self, memory: Memory, default: Any = None) -> Any:
        return getattr(self.memory_storage, memory.name, default)


class MemoryEvaluationOperation(FactEvaluationOperation):
    def __init__(self, name: str, evaluation_type: FactEvaluationType, value: Any, data_type: Optional[Any] = None):
        super().__init__(name, evaluation_type, value, data_type)

    @override
    def is_true(self, context: ContextBase) -> bool:
        context_with_memory = cast(Context, context)
        return super()._is_true(context_with_memory.memory_storage)


class Action:
    def take_action(self, context: Context):
        raise NotImplementedError()


class DisableSelf(Action):
    @override
    def take_action(self, context: Context):
        context.rule.disable()


class UpdateMemory(Action):
    def __init__(self, memory: Memory, value: Any):
        self.memory = memory
        self.value = value

    def take_action(self, context: Context):
        setattr(context.memory_storage, self.memory.name, self.value)


class Target:
    pass


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

    def _register_tag(self, tag: str, rule: "Rule"):
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


Not = NotOperation
Or = OrOperation
And = AndOperation
When = AndOperation
always_false = AlwaysFalseOperation()
always_true = AlwaysTrueOperation()
disable_self = DisableSelf()
update_memory = UpdateMemory
