from game_model_defs import *

all_models = [
    Wall(
        name="palisade",
        display_name="Palisade",
        line_of_sight=256*2,
        size="small",
        construction_site="construction_site_small",
        graphics=Graphic(
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
                    variation_filter={GraphicVariantType.ICON:"true"})
        ]),
        connected_constructions_allowed=True,
        default_orientation="corner"
    )
]

all_entity_names = ["palisade"]
def validate_all():
    return True
