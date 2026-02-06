from game_model_defs import *

all_models = [
	UIElement(
        name="ui_element",
        element_name="resource_panel", 
        graphics=Graphic(
            layer=GraphicLayer.UI,
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(drs_file="interfac.drs", slp_id=51135, clip_rect=Rect(x=0, y=0, w=400, h=25)),
                    variation_filter={GraphicVariantType.THEME:"default"})]))
]

all_entity_names = ["ui_element"]

def validate_all():
    return True
