from game_model_defs import *

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
    "construction_site_small",
    "construction_site_medium",
    "construction_site_large",
    "construction_site_huge",
    "construction_site_gate",
    "barracks",
    "militia",
    "palisade",
    "stone_wall",
    "stone_gate",
    "default_tileset",
    "resource_panel",
    "control_panel",
    "progress_bar",
    "cursor",
    "generic_widget"
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
        attack_rate=2,
        attack={ArmorClass.MELEE: 10},
        attack_multiplier={},
        armor={ArmorClass.MELEE: 5},
        health=100,
        buildables=[Shortcut(name="mill", shortcut="m"),
                    Shortcut(name="wood_camp", shortcut="l"),
                    Shortcut(name="mine_camp", shortcut="n"),
                    Shortcut(name="town_center", shortcut="c"),
                    Shortcut(name="barracks", shortcut="b"),
                    Shortcut(name="palisade", shortcut="p"),
                    Shortcut(name="stone_wall", shortcut="o"),
                    Shortcut(name="stone_gate", shortcut="i"),
                    Shortcut(name="house", shortcut="h")],
        graphics=Graphic(
            layer=GraphicLayer.ENTITIES,
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(drs_file="interfac.drs", slp_id=50730, frame_index=16),
                    variation_filter={GraphicVariantType.ICON:"true"})]),
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
        damage_resistance=0.1,
        attack_rate=2,
        attack={ArmorClass.MELEE: 10, ArmorClass.PIERCE: 5},
        attack_multiplier={},
        armor={ArmorClass.MELEE: 20},
        health=100,
        graphics=Graphic(
            layer=GraphicLayer.ENTITIES,
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(drs_file="interfac.drs", slp_id=50730, frame_index=8),
                    variation_filter={GraphicVariantType.ICON:"true"})]),
        animations=[
            Animation(name="idle", frame_count=6, speed=15, drs_file="graphics.drs", slp_id=993),
            Animation(name="move", frame_count=12, speed=15, drs_file="graphics.drs", slp_id=997),
            Animation(name="attack", frame_count=10, speed=15, drs_file="graphics.drs", slp_id=987),
        ]
    ),
]

all_natural_resources: List[NaturalResource] = [
    NaturalResource(
        name="gold",
        display_name="Gold Mine",
        resource_amount=1000,
        graphics=Graphic(
            layer=GraphicLayer.ENTITIES,
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=4479),
                    variation_filter={GraphicVariantType.THEME:"default"}),
                GraphicVariant(
                    graphic=SingleGraphic(drs_file="interfac.drs", slp_id=50731, frame_index=3),
                    variation_filter={GraphicVariantType.ICON:"true"})]),
    ),
    NaturalResource(
        name="stone",
        display_name="Stone Mine",
        resource_amount=1000,
        graphics=Graphic(
            layer=GraphicLayer.ENTITIES,
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=1034),
                    variation_filter={GraphicVariantType.THEME:"default"}),
                GraphicVariant(
                    graphic=SingleGraphic(drs_file="interfac.drs", slp_id=50731, frame_index=1),
                    variation_filter={GraphicVariantType.ICON:"true"})]),
    ),
    Tree(
        name="wood",
        display_name="Tree",
        resource_amount=100,
        graphics=Graphic(
            layer=GraphicLayer.ENTITIES,
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=4652),
                    variation_filter={GraphicVariantType.THEME:"oak"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=1252),
                    variation_filter={GraphicVariantType.THEME:"oak", GraphicVariantType.STATE:"stump"}),
                GraphicVariant(
                    graphic=SingleGraphic(drs_file="interfac.drs", slp_id=50731, frame_index=0),
                    variation_filter={GraphicVariantType.ICON:"true"})
                ]),
        shadow=NaturalResourceAdditionalPart(
            name="shadow", 
            graphics=Graphic(
                    layer=GraphicLayer.ON_GROUND,
                    variants=[
                        GraphicVariant(
                            graphic=SingleGraphic(slp_id=2296),
                            variation_filter={GraphicVariantType.THEME:"oak"})])),
    ),
]

all_buildings: List[Building] = [
    ResourceDropOff(
        name="mill",
        display_name="Mill",
        line_of_sight=256*5,
        size="medium",
        construction_site="construction_site_medium",
        accepted_resources=["food"], 
        health=1000,
        graphics=Graphic(
            layer=GraphicLayer.ENTITIES,
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=3483),
                    variation_filter={GraphicVariantType.THEME:"default"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=237),
                    variation_filter={GraphicVariantType.CONSTRUCTION_SITE:"true"}),
                GraphicVariant(
                    graphic=SingleGraphic(drs_file="interfac.drs", slp_id=50705, frame_index=21),
                    variation_filter={GraphicVariantType.ICON:"true"})]),
    ),
    ResourceDropOff(
        name="wood_camp", 
        display_name="Lumber Camp",
        line_of_sight=256*5,
        size="medium",
        construction_site="construction_site_medium",
        accepted_resources=["wood"], 
        health=1000,
        graphics=Graphic(
            layer=GraphicLayer.ENTITIES,
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=3505),
                    variation_filter={GraphicVariantType.THEME:"default"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=237),
                    variation_filter={GraphicVariantType.CONSTRUCTION_SITE:"true"}),
                GraphicVariant(
                    graphic=SingleGraphic(drs_file="interfac.drs", slp_id=50705, frame_index=40),
                    variation_filter={GraphicVariantType.ICON:"true"})]),
    ),
    ResourceDropOff(
        name="mine_camp", 
        display_name="Mining Camp",
        line_of_sight=256*5,
        size="medium",
        construction_site="construction_site_medium",
        accepted_resources=["gold", "stone"],
        health=1000,
        graphics=Graphic(
            layer=GraphicLayer.ENTITIES,
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=3492),
                    variation_filter={GraphicVariantType.THEME:"default"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=237),
                    variation_filter={GraphicVariantType.CONSTRUCTION_SITE:"true"}),
                GraphicVariant(
                    graphic=SingleGraphic(drs_file="interfac.drs", slp_id=50705, frame_index=39),
                    variation_filter={GraphicVariantType.ICON:"true"})]),
    ),
    TownCenter(
        name="town_center", 
        display_name="Town Center",
        line_of_sight=256*5,
        size="huge",
        construction_site="construction_site_huge",
        accepted_resources=["gold", "stone", "food"],
        unit_creation_speed=40,
        producible_units=[Shortcut(name="villager", shortcut="v")],
        max_queue_size=10,
        housing_capacity=5,
        garrisonable_unit_types = [UnitType.VILLAGER],
        garrison_capacity = 10,
        health=1000,
        graphics=Graphic(
            layer=GraphicLayer.ENTITIES,
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
                    variation_filter={GraphicVariantType.THEME:"default"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=239),
                    variation_filter={GraphicVariantType.CONSTRUCTION_SITE:"true"}),
                GraphicVariant(
                    graphic=SingleGraphic(drs_file="interfac.drs", slp_id=50705, frame_index=28),
                    variation_filter={GraphicVariantType.ICON:"true"})]),
    ),
    House(
        housing_capacity=5,
        name="house",
        display_name="House",
        line_of_sight=256*2,
        size="medium",
        construction_site="construction_site_medium",
        health=1000,
        graphics=Graphic(
            layer=GraphicLayer.ENTITIES,
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=2223),
                    variation_filter={GraphicVariantType.THEME:"default"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=237),
                    variation_filter={GraphicVariantType.CONSTRUCTION_SITE:"true"}),
                GraphicVariant(
                    graphic=SingleGraphic(drs_file="interfac.drs", slp_id=50705, frame_index=34),
                    variation_filter={GraphicVariantType.ICON:"true"})]),
    ),
    MilitaryBuilding(
        name="barracks",
        display_name="Barracks",
        line_of_sight=256*2,
        size="large",
        construction_site="construction_site_large",
        unit_creation_speed=40,
        max_queue_size=10,
        health=1000,
        graphics=Graphic(
            layer=GraphicLayer.ENTITIES,
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=130),
                    variation_filter={GraphicVariantType.THEME:"default"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=238),
                    variation_filter={GraphicVariantType.CONSTRUCTION_SITE:"true"}),
                GraphicVariant(
                    graphic=SingleGraphic(drs_file="interfac.drs", slp_id=50705, frame_index=3),
                    variation_filter={GraphicVariantType.ICON:"true"})]),
        producible_units=[Shortcut(name="militia", shortcut="m")]
    ),
    Wall(
        name="palisade",
        display_name="Palisade",
        line_of_sight=256*2,
        size="small",
        construction_site="construction_site_small",
        health=1000,
        graphics=Graphic(
            layer=GraphicLayer.ENTITIES,
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=1828, frame_index=0),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"diagonal_forward"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=1828, frame_index=1),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"diagonal_backward"}),
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
                GraphicVariant(
                    graphic=SingleGraphic(drs_file="interfac.drs", slp_id=50705, frame_index=30),
                    variation_filter={GraphicVariantType.ICON:"true"}),
                # Construction sites
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=236),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"diagonal_forward",
                                      GraphicVariantType.CONSTRUCTION_SITE:"true"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=236),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"diagonal_backward",
                                      GraphicVariantType.CONSTRUCTION_SITE:"true"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=236),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"horizontal",
                                      GraphicVariantType.CONSTRUCTION_SITE:"true"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=236),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"vertical",
                                      GraphicVariantType.CONSTRUCTION_SITE:"true"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=236),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"corner",
                                      GraphicVariantType.CONSTRUCTION_SITE:"true"})
        ]),
        connected_constructions_allowed=True,
        default_orientation="corner"
    ),
    Wall(
        name="stone_wall",
        display_name="Stone Wall",
        line_of_sight=256*2,
        size="small",
        construction_site="construction_site_small",
        health=1000,
        graphics=Graphic(
            layer=GraphicLayer.ENTITIES,
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=2110, frame_index=0),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"diagonal_forward"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=2110, frame_index=1),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"diagonal_backward"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=2110, frame_index=2),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"corner"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=2110, frame_index=3),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"horizontal"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=2110, frame_index=4),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"vertical"}),
                GraphicVariant(
                    graphic=SingleGraphic(drs_file="interfac.drs", slp_id=50705, frame_index=29),
                    variation_filter={GraphicVariantType.ICON:"true"}),
                # Construction sites
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=236),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"diagonal_forward",
                                      GraphicVariantType.CONSTRUCTION_SITE:"true"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=236, flip=True),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"diagonal_backward",
                                      GraphicVariantType.CONSTRUCTION_SITE:"true"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=236),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"horizontal",
                                      GraphicVariantType.CONSTRUCTION_SITE:"true"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=236),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"vertical",
                                      GraphicVariantType.CONSTRUCTION_SITE:"true"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=236),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"corner",
                                      GraphicVariantType.CONSTRUCTION_SITE:"true"})
        ]),
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
        construction_site="construction_site_gate",
        health=1000,
        graphics=Graphic(
            layer=GraphicLayer.ENTITIES,
            variants=[
                # Icon
                GraphicVariant(
                    graphic=SingleGraphic(drs_file="interfac.drs", slp_id=50705, frame_index=36),
                    variation_filter={GraphicVariantType.ICON:"true"}),
                # Closed Gates
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
                                      GraphicVariantType.ORIENTATION:"diagonal_forward",
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
                                      GraphicVariantType.ORIENTATION:"diagonal_backward",
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
                # Opened Gates
                GraphicVariant(
                    graphic=CompositeGraphic(
                        anchor=Point(x=125, y=190),
                        parts=[
                            SingleGraphic(slp_id=2391, anchor=Point(x=-15,y=175)),
                            SingleGraphic(slp_id=2355, anchor=Point(x=60,y=130)), 
                            SingleGraphic(slp_id=2391, anchor=Point(x=130,y=103)),
                        ]
                    ),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"diagonal_forward",
                                      GraphicVariantType.STATE:"opened"}),
                GraphicVariant(
                    graphic=CompositeGraphic(
                        anchor=Point(x=125, y=190),
                        parts=[
                            SingleGraphic(slp_id=2391, anchor=Point(x=-15,y=175)),
                            SingleGraphic(slp_id=2355, anchor=Point(x=60,y=130)), 
                            SingleGraphic(slp_id=2391, anchor=Point(x=130,y=103)),
                        ],
                        flip=True
                    ),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"diagonal_backward",
                                      GraphicVariantType.STATE:"opened"}),
                GraphicVariant(
                    graphic=CompositeGraphic(
                        anchor=Point(x=195, y=150),
                        parts=[
                            SingleGraphic(slp_id=4023, anchor=Point(x=100,y=120)), 
                            SingleGraphic(slp_id=2391, anchor=Point(x=195,y=150)),
                            SingleGraphic(slp_id=2391, anchor=Point(x=-90,y=150)),
                        ],
                    ),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"horizontal",
                                      GraphicVariantType.STATE:"opened"}),
                GraphicVariant(
                    graphic=CompositeGraphic(
                        anchor=Point(x=51, y=178+48),
                        parts=[
                            SingleGraphic(slp_id=2391, anchor=Point(x=51,y=178+48)),
                            SingleGraphic(slp_id=4111, anchor=Point(x=10,y=150)), 
                            SingleGraphic(slp_id=2391, anchor=Point(x=51,y=178-96)),
                        ],
                    ),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"vertical",
                                      GraphicVariantType.STATE:"opened"}),
                # Construction sites
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=3706),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"diagonal_forward",
                                      GraphicVariantType.CONSTRUCTION_SITE:"true"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=3706, flip=True),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"diagonal_backward",
                                      GraphicVariantType.CONSTRUCTION_SITE:"true"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=4067),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"horizontal",
                                      GraphicVariantType.CONSTRUCTION_SITE:"true"}),
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=4155),
                    variation_filter={GraphicVariantType.THEME:"default", 
                                      GraphicVariantType.ORIENTATION:"vertical",
                                      GraphicVariantType.CONSTRUCTION_SITE:"true"})
        ]),
        default_orientation="diagonal_forward"
    ),
]

all_tilesets: List[TileSet] = [
    TileSet(
        name="default_tileset",
        graphics=Graphic(
            layer=GraphicLayer.GROUND,
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(drs_file="terrain.drs", slp_id=15001),
                    variation_filter={GraphicVariantType.THEME:"default"})]),
    )
]

all_ui_elements: List[UIElement] = [
    UIElement(name="generic_widget", 
              graphics=Graphic(
                layer=GraphicLayer.UI,
                variants=[
                    GraphicVariant(
                        # Using a random graphic here and making it invisible by setting clip_rect to 0
                        # since this is a logical UI element that doesn't have a visual representation
                        graphic=SingleGraphic(slp_id=4479, clip_rect=Rect(x=0, y=0, w=0, h=0)),
                        variation_filter={GraphicVariantType.THEME:"default"})])),
    UIElement(name="resource_panel", 
              graphics=Graphic(
                layer=GraphicLayer.UI,
                variants=[
                    GraphicVariant(
                        graphic=SingleGraphic(drs_file="interfac.drs", slp_id=51135, clip_rect=Rect(x=0, y=0, w=400, h=25)),
                        variation_filter={GraphicVariantType.THEME:"default"})])),
    UIElement(name="control_panel",
              graphics=Graphic(
                layer=GraphicLayer.UI,
                variants=[
                    GraphicVariant(
                        graphic=SingleGraphic(drs_file="interfac.drs", slp_id=51135, clip_rect=Rect(x=0, y=593, w=680, h=175)),
                        variation_filter={GraphicVariantType.THEME:"default"})])),
    UIElement(name="progress_bar", 
              graphics=Graphic(
                layer=GraphicLayer.UI,
                variants=[
                    GraphicVariant(
                        graphic=SingleGraphic(drs_file="interfac.drs", slp_id=50764),
                        variation_filter={GraphicVariantType.THEME:"default"})])),
    UIElement(name="cursor", 
              graphics=Graphic(
                layer=GraphicLayer.UI,
                variants=[
                    GraphicVariant(
                        graphic=SingleGraphic(drs_file="interfac.drs", slp_id=51000),
                        variation_filter={GraphicVariantType.THEME:"default"})]),)
]

lists = [all_units, all_natural_resources, all_buildings, all_tilesets, all_ui_elements]
all_models = [x for lst in lists for x in lst]