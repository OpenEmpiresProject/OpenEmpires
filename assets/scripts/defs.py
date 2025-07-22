from dataclasses import dataclass
from typing import Dict, List

class Animation:
    name: str
    frame_count: int = 15
    speed: int = 10
    drs_file: str = "graphics.drs"
    slp_id: int
    repeatable: bool = True

    def __init__(self, **kwargs):
        for key, value in kwargs.items():
            setattr(self, key, value)


class Unit:
    name: str
    line_of_sight: int
    moving_speed: int
    animations: List[Animation]


class Villager(Unit):
    build_speed: int
    gather_speed: int
    resource_capacity: int

    def __init__(self, **kwargs):
        for key, value in kwargs.items():
            setattr(self, key, value)


all_units: List[Unit] = [
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

class Graphic:
    drs_file: str = "graphics.drs"
    slp_id: int

    def __init__(self, **kwargs):
        for key, value in kwargs.items():
            setattr(self, key, value)


class NaturalResource:
    name: str
    resource_amount: int
    graphics: Dict[str, Graphic] # Graphics by theme

    def __init__(self, **kwargs):
        for key, value in kwargs.items():
            setattr(self, key, value)


all_natural_resources: List[NaturalResource] = [
    NaturalResource(
        name="gold",
        resource_amount=1000,
        graphics={"default":Graphic(slp_id=4479)}
    ),
    NaturalResource(
        name="stone",
        resource_amount=1000,
        graphics={"default":Graphic(slp_id=1034)}
    ),
    NaturalResource(
        name="wood",
        resource_amount=100,
        graphics={"oak":Graphic(slp_id=435)}
    ),
]

class Building:
    name: str
    line_of_sight: int
    size: str
    graphics: Dict[str, Graphic] # Graphics by theme


class ResourceDropOff:
    accepted_resources: List[str]


class SingleResourceDropOffPoint(Building, ResourceDropOff):
    def __init__(self, **kwargs): self.__dict__.update(kwargs)


all_buildings: List[Building] = [
    SingleResourceDropOffPoint(
        name="mill", 
        line_of_sight=256*5,
        size="small",
        accepted_resources=["food"], 
        graphics={"default":Graphic(slp_id=3483)}
    ),
    SingleResourceDropOffPoint(
        name="wood_camp", 
        line_of_sight=256*5,
        size="small",
        accepted_resources=["wood"], 
        graphics={"default":Graphic(slp_id=3505)}),
    SingleResourceDropOffPoint(
        name="mine_camp", 
        line_of_sight=256*5,
        size="small",
        accepted_resources=["gold", "stone"], 
        graphics={"default":Graphic(slp_id=3492)}),
]