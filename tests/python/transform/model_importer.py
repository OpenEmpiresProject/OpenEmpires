from game_model_defs import *

all_models = [
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
        buildables=[],
        icon=Icon(drs_file="interfac.drs", slp_id=50730, index=16),
        animations=[]
    ),
    NaturalResource(
        name="gold",
        display_name="Gold Mine",
        resource_amount=1000,
        graphics=Graphic(
            layer=GraphicLayer.ENTITIES,
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=4479),
                    variation_filter={GraphicVariantType.THEME:"default"})]),
        icon=Icon(drs_file="interfac.drs", slp_id=50731, index=3),
    ),
    ResourceDropOff(
        name="mill", 
        display_name="Mill",
        line_of_sight=256*5,
        size="medium",
        accepted_resources=["food"], 
        graphics=Graphic(
            layer=GraphicLayer.ENTITIES,
            variants=[
                GraphicVariant(
                    graphic=SingleGraphic(slp_id=3483),
                    variation_filter={GraphicVariantType.THEME:"default"})]),
        icon=Icon(drs_file="interfac.drs", slp_id=50705, index=21)
    ),
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

all_entity_names = ["villager", "gold", "mill", "default_tileset"]

def validate_all():
    return True
