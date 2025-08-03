from dataclasses import dataclass
from typing import Dict, List


class Rect:
    x: int
    y: int
    w: int
    h: int
    def __init__(self, **kwargs): self.__dict__.update(kwargs)


class Graphic:
    drs_file: str = "graphics.drs"
    slp_id: int
    clip_rect: Rect

    def __init__(self, **kwargs): self.__dict__.update(kwargs)


class Animation:
    name: str
    frame_count: int = 15
    speed: int = 10
    drs_file: str = "graphics.drs"
    slp_id: int
    repeatable: bool = True

    def __init__(self, **kwargs): self.__dict__.update(kwargs)


class Icon(Graphic):
    index: int
    
    def __init__(self, **kwargs): self.__dict__.update(kwargs)


class Unit:
    name: str
    line_of_sight: int
    moving_speed: int
    animations: List[Animation]
    icon: Icon


class Villager(Unit):
    build_speed: int
    gather_speed: int
    resource_capacity: int

    def __init__(self, **kwargs): self.__dict__.update(kwargs)


class NaturalResource:
    name: str
    resource_amount: int
    graphics: Dict[str, Graphic] # Graphics by theme
    icon: Icon

    def __init__(self, **kwargs): self.__dict__.update(kwargs)


class Tree(NaturalResource):
    shadow: NaturalResource
    stump: NaturalResource

    def __init__(self, **kwargs): self.__dict__.update(kwargs)


class Building:
    name: str
    line_of_sight: int
    size: str
    graphics: Dict[str, Graphic] # Graphics by theme
    icon: Icon


class ResourceDropOff:
    accepted_resources: List[str]


class SingleResourceDropOffPoint(Building, ResourceDropOff):
    def __init__(self, **kwargs): self.__dict__.update(kwargs)


class ConstructionSite:
    name: str = "construction_site"
    size: str
    graphics: Dict[str, Graphic] # Graphics by theme
    progress_frame_map: Dict[int, int]

    def __init__(self, **kwargs): self.__dict__.update(kwargs)


class TileSet:
    graphics: Dict[str, Graphic] # Graphics by theme

    def __init__(self, **kwargs): self.__dict__.update(kwargs)


class UIElement:
    name: str
    graphics: Dict[str, Graphic] # Graphics by theme
    def __init__(self, **kwargs): self.__dict__.update(kwargs)


all_units: List[Unit] = [
    Villager(
        name="villager",
        line_of_sight=256*5,
        moving_speed=256,
        build_speed=20,
        gather_speed=10,
        resource_capacity=100,
        icon=Icon(drs_file="interfac.drs", slp_id=50730, index=16),
        animations=[
            Animation(name="idle", frame_count=15, speed=15, drs_file="graphics.drs", slp_id=1388),
            Animation(name="move", frame_count=15, speed=15, drs_file="graphics.drs", slp_id=1392),
            Animation(name="chop", frame_count=15, speed=15, drs_file="graphics.drs", slp_id=1434),
            Animation(name="mine", frame_count=15, speed=15, drs_file="graphics.drs", slp_id=1880),
            Animation(name="carry_lumber", frame_count=15, speed=15, drs_file="graphics.drs", slp_id=1883),
            Animation(name="carry_stone", frame_count=15, speed=15, drs_file="graphics.drs", slp_id=1879),
            Animation(name="carry_gold", frame_count=15, speed=15, drs_file="graphics.drs", slp_id=2218),
            Animation(name="build", frame_count=15, speed=25, drs_file="graphics.drs", slp_id=1874),
        ]
    )
]

all_natural_resources: List[NaturalResource] = [
    NaturalResource(
        name="gold",
        resource_amount=1000,
        graphics={"default":Graphic(slp_id=4479)},
        icon=Icon(drs_file="interfac.drs", slp_id=50731, index=3),
    ),
    NaturalResource(
        name="stone",
        resource_amount=1000,
        graphics={"default":Graphic(slp_id=1034)},
        icon=Icon(drs_file="interfac.drs", slp_id=50731, index=1),
    ),
    Tree(
        name="wood",
        resource_amount=100,
        graphics={"oak":Graphic(slp_id=4652)},
        stump=NaturalResource(name="stump", graphics={"oak":Graphic(slp_id=1252)}),
        shadow=NaturalResource(name="shadow", graphics={"oak":Graphic(slp_id=2296)}),
        icon=Icon(drs_file="interfac.drs", slp_id=50731, index=0),
    ),
]

all_buildings: List[Building] = [
    SingleResourceDropOffPoint(
        name="mill", 
        line_of_sight=256*5,
        size="medium",
        accepted_resources=["food"], 
        graphics={"default":Graphic(slp_id=3483)}
    ),
    SingleResourceDropOffPoint(
        name="wood_camp", 
        line_of_sight=256*5,
        size="medium",
        accepted_resources=["wood"], 
        graphics={"default":Graphic(slp_id=3505)}),
    SingleResourceDropOffPoint(
        name="mine_camp", 
        line_of_sight=256*5,
        size="medium",
        accepted_resources=["gold", "stone"], 
        graphics={"default":Graphic(slp_id=3492)}),
]

all_construction_sites: List[ConstructionSite] = [
    ConstructionSite(
        size="medium", 
        graphics={"default": Graphic(slp_id=237)}, 
        progress_frame_map={33:0, 66:1, 99:2})
]

all_tilesets: List[TileSet] = [
    TileSet(graphics={"grass":Graphic(drs_file="terrain.drs", slp_id=15001)})
]

all_ui_elements: List[UIElement] = [
    UIElement(name="resource_panel", graphics={"default":Graphic(drs_file="interfac.drs", slp_id=51101, clip_rect=Rect(w=400, h=25))}),
    UIElement(name="control_panel", graphics={"default":Graphic(drs_file="interfac.drs", slp_id=51101, clip_rect=Rect(y=454, w=506, h=145))})
]