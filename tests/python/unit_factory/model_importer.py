from game_model_defs import *

all_models = [
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
    )
]

all_entity_names = ["barracks"]
def validate_all():
    return True
