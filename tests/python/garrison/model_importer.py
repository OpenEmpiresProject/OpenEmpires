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
    )
]

all_entity_names = ["town_center"]
def validate_all():
    return True
