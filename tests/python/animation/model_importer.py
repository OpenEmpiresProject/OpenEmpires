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
        buildables=[Shortcut(name="mill", shortcut="m"),
                    Shortcut(name="wood_camp", shortcut="l"),
                    Shortcut(name="mine_camp", shortcut="n")],
        icon=Icon(drs_file="interfac.drs", slp_id=50730, index=16),
        animations=Animation(
            variants=[
                AnimationVariant(name="idle", frame_count=15, speed=15, drs_file="graphics.drs", slp_id=1388, variation_filter={}, layer=GraphicLayer.ENTITIES),
                AnimationVariant(name="move", frame_count=15, speed=15, drs_file="graphics.drs", slp_id=1392, variation_filter={}, layer=GraphicLayer.ENTITIES),
                AnimationVariant(name="chop", frame_count=15, speed=15, drs_file="graphics.drs", slp_id=1434, variation_filter={}, layer=GraphicLayer.ENTITIES),
                AnimationVariant(name="mine", frame_count=15, speed=15, drs_file="graphics.drs", slp_id=1880, variation_filter={}, layer=GraphicLayer.ENTITIES),
                AnimationVariant(name="carry_lumber", frame_count=15, speed=15, drs_file="graphics.drs", slp_id=1883, variation_filter={}, layer=GraphicLayer.ENTITIES),
                AnimationVariant(name="carry_stone", frame_count=15, speed=15, drs_file="graphics.drs", slp_id=1879, variation_filter={}, layer=GraphicLayer.ENTITIES),
                AnimationVariant(name="carry_gold", frame_count=15, speed=15, drs_file="graphics.drs", slp_id=2218, variation_filter={}, layer=GraphicLayer.ENTITIES),
                AnimationVariant(name="build", frame_count=15, speed=25, drs_file="graphics.drs", slp_id=1874, variation_filter={}, layer=GraphicLayer.ENTITIES),
        ])
        
    )
]

all_entity_names = ["villager"]
def validate_all():
    return True
