from game_model_defs import *

all_models = [
    ResourceDropOff(
        name="mill",
        display_name="Mill",
        line_of_sight=256*5,
        size="medium",
        construction_site="construction_site_medium",
        accepted_resources=["food"], 
        default_orientation="corner",
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
    )
]

all_entity_names = ["mill"]
def validate_all():
    return True
