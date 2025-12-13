from core_defs import *


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
    "barracks",
    "militia",
    "palisade",
    "stone_wall",
    "stone_gate",
]


all_units: List[Unit] = [
    Villager(
        name="villager",
        display_name="Villager",
        unit_type = UnitType.VILLAGER,
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
                    Shortcut(name="palisade", shortcut="p"),
                    Shortcut(name="stone_wall", shortcut="o"),
                    Shortcut(name="stone_gate", shortcut="i"),
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
    ),
    MilitaryUnit(
        name="militia",
        display_name="Milita",
        unit_type = UnitType.INFANTRY,
        line_of_sight=256*5,
        moving_speed=256,
        housing_need=1,
        icon=Icon(drs_file="interfac.drs", slp_id=50730, index=8),
        animations=[
            Animation(name="idle", frame_count=6, speed=15, drs_file="graphics.drs", slp_id=993),
            Animation(name="move", frame_count=12, speed=15, drs_file="graphics.drs", slp_id=997),
        ]
    ),
]

all_natural_resources: List[NaturalResource] = [
    NaturalResource(
        name="gold",
        display_name="Gold Mine",
        resource_amount=1000,
        graphics=Graphic(
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=4479),
                    variation_filter={GraphicVariantType.THEME:"default"})]),
        icon=Icon(drs_file="interfac.drs", slp_id=50731, index=3),
    ),
    NaturalResource(
        name="stone",
        display_name="Stone Mine",
        resource_amount=1000,
        graphics=Graphic(
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=1034),
                    variation_filter={GraphicVariantType.THEME:"default"})]),
        icon=Icon(drs_file="interfac.drs", slp_id=50731, index=1),
    ),
    Tree(
        name="wood",
        display_name="Tree",
        resource_amount=100,
        graphics=Graphic(
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=4652),
                    variation_filter={GraphicVariantType.THEME:"default"})]),
        stump=NaturalResourceAdditionalPart(
            name="stump", 
            graphics=Graphic(
                variants=[
                    GraphicVariant(
                        graphic=SingleGraphic(slp_id=1252),
                        variation_filter={GraphicVariantType.THEME:"oak"})])),
        shadow=NaturalResourceAdditionalPart(
            name="shadow", 
            graphics=Graphic(
                    variants=[
                        GraphicVariant(
                            graphic=SingleGraphic(slp_id=2296),
                            variation_filter={GraphicVariantType.THEME:"oak"})])),
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
        graphics=Graphic(
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=3483),
                    variation_filter={GraphicVariantType.THEME:"default"})]),
        icon=Icon(drs_file="interfac.drs", slp_id=50705, index=21)
    ),
    ResourceDropOff(
        name="wood_camp", 
        display_name="Lumber Camp",
        line_of_sight=256*5,
        size="medium",
        accepted_resources=["wood"], 
        graphics=Graphic(
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=3505),
                    variation_filter={GraphicVariantType.THEME:"default"})]),
        icon=Icon(drs_file="interfac.drs", slp_id=50705, index=40)
    ),
    ResourceDropOff(
        name="mine_camp", 
        display_name="Mining Camp",
        line_of_sight=256*5,
        size="medium",
        accepted_resources=["gold", "stone"],
        graphics=Graphic(
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=3492),
                    variation_filter={GraphicVariantType.THEME:"default"})]),
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
        garrisonable_unit_types = [UnitType.VILLAGER],
        garrison_capacity = 10,
        graphics=Graphic(
            variants=[
                GraphicVariant(
                    graphic=CompositeGraphic(
                        anchor=Point(x=187, y=195),
                        parts=[
                            SingleGraphic(slp_id=900), 
                            SingleGraphic(slp_id=3601, anchor=Point(x=153,y=-20)), 
                            SingleGraphic(slp_id=3605, anchor=Point(x=187,y=0)), 
                            SingleGraphic(slp_id=4617, anchor=Point(x=-46,y=-20)), 
                            SingleGraphic(slp_id=4621, anchor=Point(x=-161,y=0)), 
                            SingleGraphic(slp_id=3597, anchor=Point(x=185,y=47)), 
                            SingleGraphic(slp_id=4613, anchor=Point(x=-46,y=48))]),
                    variation_filter={GraphicVariantType.THEME:"default"})]),
        icon=Icon(drs_file="interfac.drs", slp_id=50705, index=28)
    ),
    House(
        housing_capacity=5,
        name="house",
        display_name="House",
        line_of_sight=256*2,
        size="medium",
        graphics=Graphic(
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=2223),
                    variation_filter={GraphicVariantType.THEME:"default"})]),
        icon=Icon(drs_file="interfac.drs", slp_id=50705, index=34)
    ),
    MilitaryBuilding(
        name="barracks",
        display_name="Barracks",
        line_of_sight=256*2,
        size="large",
        unit_creation_speed=40,
        max_queue_size=10,
        graphics=Graphic(
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=130),
                    variation_filter={GraphicVariantType.THEME:"default"})]),
        icon=Icon(drs_file="interfac.drs", slp_id=50705, index=3),
        producible_units=[Shortcut(name="militia", shortcut="m")]
    ),
    Wall(
        name="palisade",
        display_name="Palisade",
        line_of_sight=256*2,
        size="small",
        graphics=Graphic(
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=1828, frame_index=0),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"right_angled"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=1828, frame_index=1),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"left_angled"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=1828, frame_index=2),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"corner"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=1828, frame_index=3),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"horizontal"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=1828, frame_index=4),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"vertical"}),
        ]),
        icon=Icon(drs_file="interfac.drs", slp_id=50705, index=30),
        connected_constructions_allowed=True,
        default_orientation="corner"
    ),
    Wall(
        name="stone_wall",
        display_name="Stone Wall",
        line_of_sight=256*2,
        size="small",
        graphics=Graphic(
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=2110),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"right_angled"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=2110),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"left_angled"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=2110),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"corner"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=2110),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"horizontal"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=2110),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"vertical"}),
        ]),
        icon=Icon(drs_file="interfac.drs", slp_id=50705, index=29),
        connected_constructions_allowed=True,
        default_orientation="corner"
    ),
    Gate(
        name="stone_gate",
        display_name="Stone Gate",
        line_of_sight=256*2,
        active_tracking = True,
        line_of_sight_shape = LineOfSightShape.CIRCLE,
        size="gate",
        graphics=Graphic(
            variants=[
                GraphicVariant(
                    graphic=CompositeGraphic(
                        anchor=Point(x=125, y=190),
                        parts=[
                            SingleGraphic(slp_id=2391, anchor=Point(x=-15,y=175)),
                            SingleGraphic(slp_id=1926, anchor=Point(x=60,y=130)), 
                            SingleGraphic(slp_id=2391, anchor=Point(x=130,y=103)),
                        ]
                    ),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"right_angled",
                                      GraphicVariantType.STATE:"closed"}),
                GraphicVariant(
                    graphic=CompositeGraphic(
                        anchor=Point(x=125, y=190),
                        parts=[
                            SingleGraphic(slp_id=2391, anchor=Point(x=-15,y=175)),
                            SingleGraphic(slp_id=1926, anchor=Point(x=60,y=130)), 
                            SingleGraphic(slp_id=2391, anchor=Point(x=130,y=103)),
                        ],
                        flip=True
                    ),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"left_angled",
                                      GraphicVariantType.STATE:"closed"}),
                GraphicVariant(
                    graphic=CompositeGraphic(
                        anchor=Point(x=195, y=150),
                        parts=[
                            SingleGraphic(slp_id=3999, anchor=Point(x=100,y=120)), 
                            SingleGraphic(slp_id=2391, anchor=Point(x=195,y=150)),
                            SingleGraphic(slp_id=2391, anchor=Point(x=-90,y=150)),
                        ],
                    ),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"horizontal",
                                      GraphicVariantType.STATE:"closed"}),
                GraphicVariant(
                    graphic=CompositeGraphic(
                        anchor=Point(x=51, y=178+48),
                        parts=[
                            SingleGraphic(slp_id=2391, anchor=Point(x=51,y=178+48)),
                            SingleGraphic(slp_id=4087, anchor=Point(x=10,y=150)), 
                            SingleGraphic(slp_id=2391, anchor=Point(x=51,y=178-96)),
                        ],
                    ),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"vertical",
                                      GraphicVariantType.STATE:"closed"}),
        ]),
        icon=Icon(drs_file="interfac.drs", slp_id=50705, index=29),
        default_orientation="right_angled"
    ),
]

all_construction_sites: List[ConstructionSite] = [
    ConstructionSite(
        name="construction_site",
        size="small",
        graphics=Graphic(
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=236),
                    variation_filter={GraphicVariantType.THEME:"default"})]),
        progress_frame_map={33:0, 66:1, 99:2}),
    ConstructionSite(
        name="construction_site",
        size="medium",
        graphics=Graphic(
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=237),
                    variation_filter={GraphicVariantType.THEME:"default"})]),
        progress_frame_map={33:0, 66:1, 99:2}),
    ConstructionSite(
        name="construction_site",
        size="large",
        graphics=Graphic(
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=238),
                    variation_filter={GraphicVariantType.THEME:"default"})]),
        progress_frame_map={33:0, 66:1, 99:2}),
    ConstructionSite(
        name="construction_site",
        size="huge",
        graphics=Graphic(
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=239),
                    variation_filter={GraphicVariantType.THEME:"default"})]),
        progress_frame_map={33:0, 66:1, 99:2}),
    ConstructionSite(
        name="construction_site",
        size="gate",
        graphics=Graphic(
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=3706),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"right_angled",
                                      GraphicVariantType.STATE:"closed"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=3706, flip=True),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"left_angled",
                                      GraphicVariantType.STATE:"closed"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=4067),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"horizontal",
                                      GraphicVariantType.STATE:"closed"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=4155),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"vertical",
                                      GraphicVariantType.STATE:"closed"}),
        ]),
        progress_frame_map={33:0, 66:1, 99:2})
]

all_tilesets: List[TileSet] = [
    TileSet(
        name="default_tileset",
        graphics=Graphic(
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(drs_file="terrain.drs", slp_id=15001),
                    variation_filter={GraphicVariantType.THEME:"default"})]),
    )
]

all_ui_elements: List[UIElement] = [
    UIElement(name="resource_panel", 
              graphics=Graphic(
                variants=[
                    GraphicVariant(
                        graphic=SingleGraphic(drs_file="interfac.drs", slp_id=51135, clip_rect=Rect(x=0, y=0, w=400, h=25)),
                        variation_filter={GraphicVariantType.THEME:"default"})])),
    UIElement(name="control_panel",
              graphics=Graphic(
                variants=[
                    GraphicVariant(
                        graphic=SingleGraphic(drs_file="interfac.drs", slp_id=51135, clip_rect=Rect(x=0, y=593, w=680, h=175)),
                        variation_filter={GraphicVariantType.THEME:"default"})])),
    UIElement(name="progress_bar", 
              graphics=Graphic(
                variants=[
                    GraphicVariant(
                        graphic=SingleGraphic(drs_file="interfac.drs", slp_id=50764),
                        variation_filter={GraphicVariantType.THEME:"default"})])),
    UIElement(name="cursor", 
              graphics=Graphic(
                variants=[
                    GraphicVariant(
                        graphic=SingleGraphic(drs_file="interfac.drs", slp_id=51000),
                        variation_filter={GraphicVariantType.THEME:"default"})]),)
]
