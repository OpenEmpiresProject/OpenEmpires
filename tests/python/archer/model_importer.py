from game_model_defs import *

all_models = [
	RangeUnit(
        name="archer",
        display_name="Archer",
        unit_type = UnitType.ARCHER,
        line_of_sight=256*5,
        moving_speed=256,
        housing_need=1,
        health=100,
        damage_resistance=0.1,
        primary_projectile=ProjectileProperties(
            attack={ArmorClass.MELEE: 10, ArmorClass.PIERCE: 5},
            attack_multiplier={ArmorClass.MELEE: 1.5, ArmorClass.PIERCE: 0.5},
            accuracy=1,
            reload_time=1),
        secondary_projectiles=[ProjectileProperties(
            attack={ArmorClass.MELEE: 5, ArmorClass.PIERCE: 2},
            attack_multiplier={ArmorClass.MELEE: 1.5, ArmorClass.PIERCE: 0.5},
            accuracy=0.8,
            reload_time=2)],
        projectile_entity_type="projectile",
        damage_mode=ProjectileDamageMode.ON_HIT,
        attack_range=256*4,
        armor={ArmorClass.MELEE: 20, ArmorClass.PIERCE: 25},
        icon=Icon(drs_file="interfac.drs", slp_id=50730, index=8),
        graphics=Graphic(
            layer=GraphicLayer.ENTITIES, variants=[]),
        animations=Animation(
            variants=[
                AnimationVariant(name="anim_idle", frame_count=6, speed=15, drs_file="graphics.drs", slp_id=993,variation_filter={"action":"idle"},layer=GraphicLayer.ENTITIES),
                AnimationVariant(name="anim_move", frame_count=12, slp_id=997,variation_filter={"action":"move"},layer=GraphicLayer.ENTITIES),
                AnimationVariant(name="anim_decay_corpse", frame_count=5, speed=0.1, drs_file="graphics.drs", slp_id=1389, repeatable=False,variation_filter={"action":"decay_corpse"},layer=GraphicLayer.ENTITIES),
        ])
        
    )
]

all_entity_names = ["archer", "projectile"]

def validate_all():
    return True
