from core_model_defs import *


class Constructible:
    def __init__(self, **kwargs):
        self.__dict__.update(kwargs)

# Following are entity definitions composed of one or more core entity definitions. Composition will
# determine the behaviour of the entity at runtime.
#
class TownCenter(Constructible, ResourceDropOff, UnitFactory, Housing, CompoSiteGraphicBuilding, Garrison):
    pass


class House(Constructible, Building, Housing):
    pass


class Villager(Constructible, Unit, Builder, Gatherer):
    pass


class MilitaryBuilding(Constructible, Building, UnitFactory):
    pass


class Wall(Constructible, Building):
    pass


class Gate(Constructible, Building):
    pass


class MilitaryUnit(Constructible, Unit):
    pass


class GraphicVariantType(str, Enum):
    THEME = "theme"
    CIVILIZATION = "civilization"
    ORIENTATION = "orientation"
    STATE = "state"
