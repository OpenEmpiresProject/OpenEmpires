from dataclasses import dataclass
from typing import List

class Animation:
    name: str
    frame_count: int = 15
    speed: float = 10
    drs_file: str = "graphics.drs"
    slp_id: int
    repeatable: bool = True

    def __init__(self, **kwargs):
        for key, value in kwargs.items():
            setattr(self, key, value)


class BaseUnitType:
    name: str
    line_of_sight: float
    moving_speed: float
    animations: List[Animation]


class Villager(BaseUnitType):
    build_speed: int
    gather_speed: int
    resource_capacity: int

    def __init__(self, **kwargs):
        for key, value in kwargs.items():
            setattr(self, key, value)


all_units: List[BaseUnitType] = [
    Villager(
        name="villager",
        line_of_sight=256*5,
        moving_speed=256,
        build_speed=10,
        gather_speed=10,
        resource_capacity=100,
        animations=[
            Animation(name="idle", frame_count=15, speed=15, drs_file="graphics.drs", slp_id=1388),
            Animation(name="move", frame_count=15, speed=15, drs_file="graphics.drs", slp_id=1392),
            Animation(name="chop", frame_count=15, speed=15, drs_file="graphics.drs", slp_id=1434),
            Animation(name="mine", frame_count=15, speed=15, drs_file="graphics.drs", slp_id=1880),
            Animation(name="carry_lumber", frame_count=15, speed=15, drs_file="graphics.drs", slp_id=1883),
            Animation(name="carry_stone", frame_count=15, speed=15, drs_file="graphics.drs", slp_id=1879),
            Animation(name="carry_gold", frame_count=15, speed=15, drs_file="graphics.drs", slp_id=2218),
            Animation(name="build", frame_count=15, speed=15, drs_file="graphics.drs", slp_id=1874),
        ]
    )
]