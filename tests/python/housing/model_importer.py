from game_model_defs import *

all_models = [
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
    )
]

all_entity_names = ["house"]
def validate_all():
    return True
