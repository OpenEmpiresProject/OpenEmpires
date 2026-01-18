from typing import Dict, List, Optional, Union

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


class Graphic(_Constructible):
    variants: List[GraphicVariant]
