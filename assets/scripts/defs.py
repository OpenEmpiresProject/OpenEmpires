from dataclasses import dataclass
from typing import Dict, List

# Start of basic definitions. These definitions are known to the game. It is possible to compose these 
# to define new entity types. But not possible to change these.

class Rect:
    x: int
    y: int
    w: int
    h: int
    def __init__(self, **kwargs): self.__dict__.update(kwargs)


class Point:
    x: int
    y: int
    def __init__(self, **kwargs): self.__dict__.update(kwargs)


class Graphic:
    drs_file: str = "graphics.drs"
    slp_id: int
    clip_rect: Rect
    anchor: Point
    def __init__(self, **kwargs): self.__dict__.update(kwargs)


class CompositeGraphic:
    parts: List[Graphic]
    anchor: Point
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
    display_name: str
    line_of_sight: int
    moving_speed: int
    animations: List[Animation]
    icon: Icon
    housing_need: int


class Shortcut:
    name: str
    shortcut: str # A single character shortcut. Relative to the selection.
    def __init__(self, **kwargs): self.__dict__.update(kwargs)


class Builder:
    build_speed: int
    buildables:  List[Shortcut] # List of buildings can create from this


class Gatherer:
    gather_speed: int
    resource_capacity: int


class NaturalResource:
    name: str
    display_name: str
    resource_amount: int
    graphics: Dict[str, List[Graphic]] # Graphics by theme
    icon: Icon

    def __init__(self, **kwargs): self.__dict__.update(kwargs)


class Tree(NaturalResource):
    shadow: NaturalResource
    stump: NaturalResource

    def __init__(self, **kwargs): self.__dict__.update(kwargs)


class Building:
    name: str
    display_name: str
    line_of_sight: int
    size: str
    graphics: Dict[str, CompositeGraphic] # Graphics by theme
    icon: Icon


class ResourceDropOff(Building):
    accepted_resources: List[str]
    def __init__(self, **kwargs): self.__dict__.update(kwargs)


class UnitFactory:
    producible_units: List[Shortcut] # List of units can create from this factory
    max_queue_size: int
    def __init__(self, **kwargs): self.__dict__.update(kwargs)


class Housing:
    housing_capacity: int
    def __init__(self, **kwargs): self.__dict__.update(kwargs)


class ConstructionSite:
    name: str = "construction_site"
    size: str
    graphics: Dict[str, List[Graphic]] # Graphics by theme
    progress_frame_map: Dict[int, int]

    def __init__(self, **kwargs): self.__dict__.update(kwargs)


class TileSet:
    graphics: Dict[str, List[Graphic]] # Graphics by theme

    def __init__(self, **kwargs): self.__dict__.update(kwargs)


class UIElement:
    name: str
    graphics: Dict[str, List[Graphic]] # Graphics by theme
    def __init__(self, **kwargs): self.__dict__.update(kwargs)


# End of basic definitions.

# Start of composite entity definitions. These can't have new attributes defined, but only overrides.

class TownCenter(ResourceDropOff, UnitFactory, Housing):
    def __init__(self, **kwargs): self.__dict__.update(kwargs)

    
class House(Building, Housing):
    def __init__(self, **kwargs): self.__dict__.update(kwargs)


class Villager(Unit, Builder, Gatherer):
    def __init__(self, **kwargs): self.__dict__.update(kwargs)


class MilitaryBuilding(Building, UnitFactory):
    def __init__(self, **kwargs): self.__dict__.update(kwargs)


# End of composite entity definitions

all_entity_names: List[str] = [
    "villager",
    "gold",
    "stone",
    "wood",
    "mill",
    "wood_camp",
    "mine_camp",
    "town_center",
    "house",
    "construction_site",
    "barracks"
]


all_units: List[Unit] = [
    Villager(
        name="villager",
        display_name="Villager",
        line_of_sight=256*5,
        moving_speed=256,
        build_speed=40,
        gather_speed=10,
        resource_capacity=100,
        housing_need=1,
        buildables=[Shortcut(name="mill", shortcut="m"),
                    Shortcut(name="wood_camp", shortcut="l"),
                    Shortcut(name="mine_camp", shortcut="n"),
                    Shortcut(name="town_center", shortcut="c"),
                    Shortcut(name="barracks", shortcut="b"),
                    Shortcut(name="house", shortcut="h")],
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
        display_name="Gold Mine",
        resource_amount=1000,
        graphics={"default":Graphic(slp_id=4479)},
        icon=Icon(drs_file="interfac.drs", slp_id=50731, index=3),
    ),
    NaturalResource(
        name="stone",
        display_name="Stone Mine",
        resource_amount=1000,
        graphics={"default":Graphic(slp_id=1034)},
        icon=Icon(drs_file="interfac.drs", slp_id=50731, index=1),
    ),
    Tree(
        name="wood",
        display_name="Tree",
        resource_amount=100,
        graphics={"oak":Graphic(slp_id=4652)},
        stump=NaturalResource(name="stump", graphics={"oak":Graphic(slp_id=1252)}),
        shadow=NaturalResource(name="shadow", graphics={"oak":Graphic(slp_id=2296)}),
        icon=Icon(drs_file="interfac.drs", slp_id=50731, index=0),
    ),
]

all_buildings: List[Building] = [
    ResourceDropOff(
        name="mill", 
        display_name="Mill",
        line_of_sight=256*5,
        size="medium",
        accepted_resources=["food"], 
        graphics={"default":Graphic(slp_id=3483),},
        icon=Icon(drs_file="interfac.drs", slp_id=50705, index=21)
    ),
    ResourceDropOff(
        name="wood_camp", 
        display_name="Lumber Camp",
        line_of_sight=256*5,
        size="medium",
        accepted_resources=["wood"], 
        graphics={"default":Graphic(slp_id=3505)},
        icon=Icon(drs_file="interfac.drs", slp_id=50705, index=40)
    ),
    ResourceDropOff(
        name="mine_camp", 
        display_name="Mining Camp",
        line_of_sight=256*5,
        size="medium",
        accepted_resources=["gold", "stone"], 
        graphics={"default":Graphic(slp_id=3492)},
        icon=Icon(drs_file="interfac.drs", slp_id=50705, index=39)
    ),
    TownCenter(
        name="town_center", 
        display_name="Town Center",
        line_of_sight=256*5,
        size="huge",
        accepted_resources=["gold", "stone", "food"],
        unit_creation_speed=40,
        producible_units=[Shortcut(name="villager", shortcut="v")],
        max_queue_size=10,
        housing_capacity=5,
        graphics={"default": CompositeGraphic(
            anchor=Point(x=187, y=290),
            parts=[
                Graphic(slp_id=900), 
                Graphic(slp_id=3601, anchor=Point(x=153,y=-20)), 
                Graphic(slp_id=3605, anchor=Point(x=187,y=0)), 
                Graphic(slp_id=4617, anchor=Point(x=-46,y=-20)), 
                Graphic(slp_id=4621, anchor=Point(x=-161,y=0)), 
                Graphic(slp_id=3597, anchor=Point(x=185,y=47)), 
                Graphic(slp_id=4613, anchor=Point(x=-46,y=48))])},
        icon=Icon(drs_file="interfac.drs", slp_id=50705, index=28)
    ),
    House(
        housing_capacity=5,
        name="house", 
        display_name="House",
        line_of_sight=256*2,
        size="medium",
        graphics={"default":Graphic(slp_id=2223)},
        icon=Icon(drs_file="interfac.drs", slp_id=50705, index=34)
    ),
    MilitaryBuilding(
        name="barracks", 
        display_name="Barracks",
        line_of_sight=256*2,
        size="large",
        graphics={"default":Graphic(slp_id=130)},
        icon=Icon(drs_file="interfac.drs", slp_id=50705, index=3),
        producible_units=[]
    ),
]

all_construction_sites: List[ConstructionSite] = [
    ConstructionSite(
        size="medium", 
        graphics={"default": Graphic(slp_id=237)}, 
        progress_frame_map={33:0, 66:1, 99:2}),
    ConstructionSite(
        size="large", 
        graphics={"default": Graphic(slp_id=238)}, 
        progress_frame_map={33:0, 66:1, 99:2}),
    ConstructionSite(
        size="huge", 
        graphics={"default": Graphic(slp_id=239)}, 
        progress_frame_map={33:0, 66:1, 99:2})
]

all_tilesets: List[TileSet] = [
    TileSet(graphics={"grass":Graphic(drs_file="terrain.drs", slp_id=15001)})
]

all_ui_elements: List[UIElement] = [
    UIElement(name="resource_panel", graphics={"default":Graphic(drs_file="interfac.drs", slp_id=51135, clip_rect=Rect(w=400, h=25))}),
    UIElement(name="control_panel", graphics={"default":Graphic(drs_file="interfac.drs", slp_id=51135, clip_rect=Rect(y=593, w=680, h=175))}),
    UIElement(name="progress_bar", graphics={"default":Graphic(drs_file="interfac.drs", slp_id=50764)})
]