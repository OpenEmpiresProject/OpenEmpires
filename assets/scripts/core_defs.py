from typing import Dict, List, Optional
from enum import IntEnum

# Core entity definitions. These definitions are known to the game. It is possible to compose these
# to define new entity types. But not possible to change these.

class _Constructible:
    def __init__(self, **kwargs):
        self.__dict__.update(kwargs)


class Rect(_Constructible):
    x: int
    y: int
    w: int
    h: int


class Point(_Constructible):
    x: int
    y: int


class Graphic(_Constructible):
    drs_file: str = "graphics.drs"
    slp_id: int
    clip_rect: Optional[Rect]
    anchor: Optional[Point]


class CompositeGraphic(_Constructible):
    parts: List[Graphic]
    anchor: Point


class Animation(_Constructible):
    name: str
    frame_count: int = 15
    speed: int = 10
    drs_file: str = "graphics.drs"
    slp_id: int
    repeatable: bool = True


class Icon(Graphic):
    index: int


class UnitType(IntEnum):
    VILLAGER = 1
    INFANTRY = 2
    ARCHER = 3
    CAVALRY = 4
    SHIP = 5
    SIEGE = 6


class Unit:
    name: str
    display_name: str
    line_of_sight: int
    moving_speed: int
    animations: List[Animation]
    icon: Icon
    housing_need: int
    unit_type: UnitType


class Shortcut(_Constructible):
    name: str
    shortcut: str # A single character shortcut. Relative to the selection.


class Builder:
    build_speed: int
    buildables:  List[Shortcut] # List of buildings can create from this


class Gatherer:
    gather_speed: int
    resource_capacity: int


class NaturalResource(_Constructible):
    name: str
    display_name: str
    resource_amount: int
    graphics: Dict[str, List[Graphic]] # Graphics by theme
    icon: Icon


class NaturalResourceAdditionalPart(_Constructible):
    name: str
    graphics: Dict[str, List[Graphic]] # Graphics by theme


class Tree(NaturalResource, _Constructible):
    shadow: NaturalResourceAdditionalPart
    stump: NaturalResourceAdditionalPart


class Building:
    name: str
    display_name: str
    line_of_sight: int
    size: str
    graphics: Dict[str, List[Graphic]] # Graphics by theme
    icon: Icon
    orientations: Optional[Dict[str, int]] # orientation name to frame id mapping
    connected_constructions_allowed: Optional[bool] # Allows to constructing series of same building such as walls

class CompoSiteGraphicBuilding(Building):
    graphics: Dict[str, CompositeGraphic] # Graphics by theme


class ResourceDropOff(Building, _Constructible):
    accepted_resources: List[str]


class UnitFactory:
    producible_units: List[Shortcut] # List of units can create from this factory
    max_queue_size: int
    unit_creation_speed: int


class Housing:
    housing_capacity: int


class Garrison:
    garrisonable_unit_types: List[UnitType]
    garrison_capacity: int


class ConstructionSite(_Constructible):
    name: str = "construction_site"
    size: str
    graphics: Dict[str, List[Graphic]] # Graphics by theme
    progress_frame_map: Dict[int, int]


class TileSet(_Constructible):
    name: str
    graphics: Dict[str, List[Graphic]] # Graphics by theme


class UIElement(_Constructible):
    name: str
    graphics: Dict[str, List[Graphic]] # Graphics by theme


def get_all_core_defs():
    return {Rect.__name__: Rect,
            Point.__name__: Point,
            Graphic.__name__: Graphic,
            CompositeGraphic.__name__: CompositeGraphic,
            Animation.__name__: Animation,
            Icon.__name__: Icon,
            Unit.__name__: Unit,
            Shortcut.__name__: Shortcut,
            Builder.__name__: Builder,
            Gatherer.__name__: Gatherer,
            NaturalResource.__name__: NaturalResource,
            Tree.__name__: Tree,
            NaturalResourceAdditionalPart.__name__: NaturalResourceAdditionalPart,
            Building.__name__: Building,
            ResourceDropOff.__name__: ResourceDropOff,
            UnitFactory.__name__: UnitFactory,
            Housing.__name__: Housing,
            ConstructionSite.__name__: ConstructionSite,
            TileSet.__name__: TileSet,
            UIElement.__name__: UIElement}