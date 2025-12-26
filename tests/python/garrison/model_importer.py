from game_model_defs import *

all_models = [
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
            layer=GraphicLayer.ENTITIES,
            variants=[
                GraphicVariant(
                    graphic=CompositeGraphic(
                        parts=[],
                        anchor=Point(x=187, y=195)),
                    variation_filter={GraphicVariantType.THEME:"default"})])
    )
]

all_entity_names = ["town_center"]
def validate_all():
    return True
