from game_model_defs import *

all_models = [
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
    )
]

all_entity_names = ["gold"]

def validate_all():
    return True
