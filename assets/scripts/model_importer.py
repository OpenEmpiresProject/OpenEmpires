import inspect
from typing import Type, Any, Tuple, get_origin, get_args, Union
from pydantic import BaseModel, ValidationError, create_model, Field
from models import *


def resolve_type_recursively(type_annotation: Type) -> Type:
    """
    Recursively resolves a type annotation (List, Dict, Optional) 
    and dynamically builds Pydantic schemas for any custom component found.
    """
    origin = get_origin(type_annotation)
    args = get_args(type_annotation)
    
    # Handle all core definitions
    if inspect.isclass(type_annotation) and type_annotation.__name__ in get_all_core_defs():
        return build_dynamic_pydantic_schema(type_annotation)
    
    # If it's a built-in type (str, int, float), return it as is.
    if origin is None:
        return type_annotation

    # Handle Optional/Union[..., NoneType]
    if origin is Union:
        # Resolve the non-None part of the Union
        non_none_args = [arg for arg in args if arg is not type(None)]
        
        if len(non_none_args) == 1:
            resolved_inner_type = resolve_type_recursively(non_none_args[0])
            return Optional[resolved_inner_type] # Return Optional[T_resolved]
        else:
            resolved_inner_types = tuple(resolve_type_recursively(t) for t in non_none_args)
            return Optional[Union[resolved_inner_types]] # Return Optional[Union[...]]

    # Handle List[T]
    elif origin is list:
        resolved_inner_type = resolve_type_recursively(args[0])
        return List[resolved_inner_type]

    # Handle Dict[K, V]
    elif origin is dict:
        resolved_key_type = resolve_type_recursively(args[0])
        resolved_value_type = resolve_type_recursively(args[1])
        return Dict[resolved_key_type, resolved_value_type]
    
    return type_annotation


def build_dynamic_pydantic_schema(entity_cls: Type) -> Type[BaseModel]:
    """
    Dynamically creates a Pydantic model using the robust recursive type resolver.
    """
    fields: Dict[str, Tuple[Type, Any]] = {}

    # Collect fields from all inherited bases (in the order of inheritance)
    for base in entity_cls.__bases__:
        base_schema = build_dynamic_pydantic_schema(base)
        for field_name, model_field in base_schema.model_fields.items():
            fields[field_name] = (model_field.annotation, model_field.default)

    # Process fields defined in the current class
    try:
        annotations = inspect.get_annotations(entity_cls)
    except AttributeError:
        # Fallback for older Python versions
        annotations = getattr(entity_cls, '__annotations__', {})

    for name, type_annotation in annotations.items():
        # Resolve the type annotation completely, getting the final Pydantic type
        pydantic_type = resolve_type_recursively(type_annotation)
        
        # Use a unique sentinel object to check if the attribute exists in the class or its bases.
        SENTINEL = object()
        default_value_from_chain = getattr(entity_cls, name, SENTINEL)
        
        # Determine Field Default (based on Optional status)
        if default_value_from_chain is not SENTINEL:
            # The attribute exists in the class or its inheritance chain, use its value.
            default_value = default_value_from_chain

        # Optional without explicit default
        elif (
            (origin := get_origin(pydantic_type)) is Union
            and type(None) in get_args(pydantic_type)
        ):
            default_value = None

        # Required
        else:
            # No default found, and it's not optional, so it's required by Pydantic.
            default_value = Field(default=...)
            
        fields[name] = (pydantic_type, default_value)

    # Create the Pydantic model with obtained fields and following config.
    # from_attributes allows us to validate objects created from arbitrary classes, in this case
    # from entity core/composite definitions.
    model_config = {
        'from_attributes': True
    }

    DynamicSchema = create_model(
        f"{entity_cls.__name__}Schema",
        __base__=BaseModel,
        __config__=model_config,
        **fields
    )
    return DynamicSchema


def validate_entity(entity_instance) -> int:
    """The universal validation function called by the game core."""
    entity_cls = type(entity_instance)
    DynamicSchema = build_dynamic_pydantic_schema(entity_cls)
    
    data_to_validate = entity_instance.__dict__
    try:
        DynamicSchema.model_validate(data_to_validate)
        print(f"[✓] Validation successful for {entity_instance.name if hasattr(entity_instance, 'name') else entity_cls}")
        return 0
    
    except ValidationError as e:
        print(f"[✗] Validation failed for {entity_instance.name if hasattr(entity_instance, 'name') else entity_cls}:")
        for err in e.errors():
            parts_str = [str(part) for part in err['loc']]
            print(f"\tField: {'.'.join(parts_str)}, Error: {err['msg']}")
        return len(e.errors())


def validate_entity_list(entity_list: List) -> int:
    errors = 0
    for entity in entity_list:
        errors += validate_entity(entity)
    return errors


def validate_all() -> bool:
    print("Starting to validate all the entities against the schema")

    errors_count = 0

    errors_count += validate_entity_list(all_units)
    errors_count += validate_entity_list(all_natural_resources)
    errors_count += validate_entity_list(all_buildings)
    errors_count += validate_entity_list(all_construction_sites)
    errors_count += validate_entity_list(all_tilesets)
    errors_count += validate_entity_list(all_ui_elements)

    if errors_count:
        print(f"\n❌ Validation completed with {errors_count} error(s).")
    else:
        print(f"\n✅ Validation completed successfully without any errors.")

    return errors_count == 0
    

if __name__ == "__main__":
    validate_all()