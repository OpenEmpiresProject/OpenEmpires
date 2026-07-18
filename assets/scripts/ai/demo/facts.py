from enum import Enum

from ai.core.rule.fact import Fact
from ai.core.rule.memory import Memory


class Age(Enum):
    DARK = 1
    CASTLE = 2

food = Fact("food")
wood = Fact("wood")
idle_villagers = Fact("idle_villagers")
age = Fact("age", Age)

warning_starving = Memory("warning_starving", bool)
