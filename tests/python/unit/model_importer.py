from game_model_defs import *

all_models = [
	MilitaryUnit(
        name="militia",
        display_name="Milita",
        unit_type = UnitType.INFANTRY,
        line_of_sight=256*5,
        moving_speed=256,
        housing_need=1,
        icon=Icon(drs_file="interfac.drs", slp_id=50730, index=8),
        animations=[
            Animation(name="idle", frame_count=6, speed=15, drs_file="graphics.drs", slp_id=993),
            Animation(name="move", frame_count=12, slp_id=997),
        ]
    )
]

all_entity_names = ["militia"]

def validate_all():
    return True
