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
        fire_anchors=[Point(x=65,y=55)],
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
    ),
    BuildingFire(
        name="fire",
        animations=Animation(
            variants=[
                AnimationVariant(name="anim_fire", frame_count=20, speed=20, drs_file="graphics.drs", slp_id=424, 
                          variation_filter={GraphicVariantType.STATE:"small", GraphicVariantType.VARIATION:"0", GraphicVariantType.ACTION:"fire"},layer=GraphicLayer.ENTITY_DECORATOR),
                AnimationVariant(name="anim_fire", frame_count=20, speed=20, drs_file="graphics.drs", slp_id=425, 
                          variation_filter={GraphicVariantType.STATE:"small", GraphicVariantType.VARIATION:"1", GraphicVariantType.ACTION:"fire"},layer=GraphicLayer.ENTITY_DECORATOR),
                AnimationVariant(name="anim_fire", frame_count=20, speed=20, drs_file="graphics.drs", slp_id=427, 
                          variation_filter={GraphicVariantType.STATE:"medium", GraphicVariantType.VARIATION:"0", GraphicVariantType.ACTION:"fire"},layer=GraphicLayer.ENTITY_DECORATOR),
            ]
        )
    )
]

all_entity_names = ["mill", "fire"]
def validate_all():
    return True
