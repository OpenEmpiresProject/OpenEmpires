from game_model_defs import *

all_models = [
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
        icon=Icon(drs_file="interfac.drs", slp_id=50705, index=21),
        default_orientation="corner",
        connected_constructions_allowed=False
    )
]

all_entity_names = ["mill"]
def validate_all():
    return True
