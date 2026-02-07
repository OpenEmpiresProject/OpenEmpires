from typing import Dict, List, Optional, Union
from enum import IntEnum, Enum
from graphic_defs import *

# Core entity definitions. These definitions are known to the game. It is possible to compose these
# to define new entity types. But not possible to change these.
class _Constructible:
    def __init__(self, **kwargs):
        self.__dict__.update(kwargs)


class Model:
    name: str


class UnitType(IntEnum):
    VILLAGER = 1
    INFANTRY = 2
    ARCHER = 3
    CAVALRY = 4
    SHIP = 5
    SIEGE = 6
    

class LineOfSightShape(str, Enum):
    CIRCLE = "circle"
    ROUNDED_SQUARE = "rounded_square"
    

class Selectable:
    display_name: str
    graphics: Graphic # Use for Icon at the moment


class Vision:
    line_of_sight: int
    line_of_sight_shape: LineOfSightShape = LineOfSightShape.CIRCLE
    active_tracking: bool = False


class Animated:
    animations: List[Animation]


class Health:
    health: int


class Attack:
    attack: Dict[int, int] # Per class
    attack_multiplier: Dict[int, float] # Per class


class Armor:
    armor: Dict[int, int] # Per class
    damage_resistance: float = 0.0 # Between 0 and 1


class Unit(Vision, Model, Selectable, Animated, Health, Attack, Armor):
    moving_speed: int
    housing_need: int
    unit_type: UnitType


class Builder:
    build_speed: int
    buildables:  List[Shortcut] # List of buildings can create from this


class Gatherer:
    gather_speed: int
    resource_capacity: int


class NaturalResource(_Constructible, Model, Selectable):
    resource_amount: int
    graphics: Graphic


class NaturalResourceAdditionalPart(_Constructible):
    name: str
    graphics: Graphic


class Tree(NaturalResource, _Constructible):
    shadow: NaturalResourceAdditionalPart


class Building(Vision, Model, Selectable, Health):
    line_of_sight_shape: LineOfSightShape = LineOfSightShape.ROUNDED_SQUARE
    size: str
    construction_site: str  # Construction site name
    graphics: Graphic
    connected_constructions_allowed: Optional[bool] # Allows to constructing series of same building such as walls
    default_orientation: Optional[str]
    progress_frame_map: Dict[int, int] = {33:0, 66:1, 99:2, 100:0}



class CompoSiteGraphicBuilding(Building):
    graphics: Graphic


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


class TileSet(_Constructible, Model):
    graphics: Graphic


class UIElement(_Constructible, Model):
    graphics: Graphic


def get_all_core_defs():
    return {Rect.__name__: Rect,
            Point.__name__: Point,
            Graphic.__name__: Graphic,
            CompositeGraphic.__name__: CompositeGraphic,
            SingleGraphic.__name__: SingleGraphic,
            GraphicVariant.__name__: GraphicVariant,
            Animation.__name__: Animation,
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
            TileSet.__name__: TileSet,
            UIElement.__name__: UIElement}