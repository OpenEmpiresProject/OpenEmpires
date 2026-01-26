from typing import Dict, List, Optional, Union
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


class SingleGraphic(_Constructible):
    drs_file: str = "graphics.drs"
    slp_id: int
    clip_rect: Optional[Rect]
    anchor: Optional[Point]
    flip: Optional[bool] = False
    frame_index: Optional[int]


class CompositeGraphic(_Constructible):
    parts: List[SingleGraphic]
    anchor: Point
    flip: Optional[bool] = False


class GraphicVariant(_Constructible):
    graphic: Union[SingleGraphic, CompositeGraphic]
    variation_filter: Dict[str, str]


class GraphicLayer(IntEnum):
    NONE = -1
    GROUND = 0
    ON_GROUND = 1
    ENTITIES = 2
    SKY = 3
    UI = 4


class Graphic(_Constructible):
    layer: GraphicLayer
    variants: List[GraphicVariant]


class Animation(_Constructible):
    name: str
    frame_count: int = 15
    speed: int = 10
    drs_file: str = "graphics.drs"
    slp_id: int
    repeatable: bool = True
