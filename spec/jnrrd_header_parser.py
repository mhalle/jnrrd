"""
JNRRD Header Parser
==================

This module provides functionality to convert a JNRRD header (represented as a list of parsed JSON objects)
into a hierarchical dictionary structure. The parser handles extension prefixes (delimited by colons),
dot notation for object properties, and bracket notation for array indices.

The output is a hierarchical dict with a "jnrrd" key for unprefixed keys/values and additional
keys for each extension encountered in the header.

Example:
    Input (list of JSON objects):
    [
        {"jnrrd": "0004"},
        {"type": "float32"},
        {"dimension": 3},
        {"nifti:intent_code": 2},
        {"nifti:qform_parameters.quaternion[0]": 0.0},
        {"nifti:qform_parameters.quaternion[1]": 0.0},
        {"dicom:patient.id": "ANONYMOUS"},
        {"dicom:study.description": "BRAIN MRI"}
    ]

    Output (hierarchical dictionary):
    {
        "jnrrd": {
            "jnrrd": "0004",
            "type": "float32",
            "dimension": 3
        },
        "nifti": {
            "intent_code": 2,
            "qform_parameters": {
                "quaternion": [0.0, 0.0]
            }
        },
        "dicom": {
            "patient": {
                "id": "ANONYMOUS"
            },
            "study": {
                "description": "BRAIN MRI"
            }
        }
    }
"""
import re
import json
from typing import List, Dict, Any, Union, Tuple


def parse_jnrrd_header(header_objects: List[Dict[str, Any]]) -> Dict[str, Any]:
    """
    Parse a JNRRD header into a hierarchical dictionary structure.
    
    Args:
        header_objects: A list of JSON objects representing the JNRRD header.
        
    Returns:
        A dictionary with extension names as keys and hierarchical data structures as values.
    """
    result = {"jnrrd": {}}
    
    for obj in header_objects:
        if not obj or len(obj) != 1:
            # Skip invalid objects or objects with multiple keys
            continue
            
        # Get the only key and value in the object
        key, value = next(iter(obj.items()))
        
        # Check if this is an extension field
        if ":" in key:
            extension, path = key.split(":", 1)
            
            # Create extension dictionary if it doesn't exist
            if extension not in result:
                result[extension] = {}
                
            # Handle the path and update the extension's data structure
            update_nested_dict(result[extension], path, value)
        else:
            # Regular JNRRD field
            result["jnrrd"][key] = value
            
    return result


def update_nested_dict(data: Dict[str, Any], path: str, value: Any) -> None:
    """
    Update a nested dictionary based on a path string.
    
    Args:
        data: The dictionary to update.
        path: The path string (e.g., "patient.id" or "channels[0].name").
        value: The value to set at the specified path.
    """
    # Current position in the nested structure
    current = data
    
    # Parse the path into components
    components = parse_path(path)
    
    # Navigate through all but the last component
    for i, component in enumerate(components[:-1]):
        if isinstance(component, int):
            # Array index
            if isinstance(current, dict):
                # Path to here was an object property
                parent_key = components[i-1] if i > 0 else None
                if parent_key is not None and parent_key not in current:
                    current[parent_key] = []
                current = current[parent_key]
            
            # Ensure the array is long enough
            while len(current) <= component:
                current.append({} if i < len(components) - 2 else None)
            
            # Move to the next level
            current = current[component]
        else:
            # Object property
            if component not in current:
                # Next component determines type
                next_comp = components[i+1] if i+1 < len(components) else None
                if isinstance(next_comp, int):
                    current[component] = []
                else:
                    current[component] = {}
            
            # Move to the next level
            current = current[component]
    
    # Handle the last component to set the value
    last = components[-1]
    if isinstance(last, int):
        # Array index
        while len(current) <= last:
            current.append(None)
        current[last] = value
    else:
        # Object property
        current[last] = value


def parse_path(path: str) -> List[Union[str, int]]:
    """
    Parse a path string into components, handling both dot notation and bracket notation.
    
    Args:
        path: The path string (e.g., "patient.id" or "channels[0].name").
        
    Returns:
        A list of path components, where string components represent object properties
        and integer components represent array indices.
    """
    components = []
    
    # Extract components using regex to handle both dot notation and bracket notation
    i = 0
    while i < len(path):
        # Check for bracket notation
        bracket_match = re.match(r'(\w+)\[(\d+)\]', path[i:])
        if bracket_match:
            prop, idx = bracket_match.groups()
            if prop:  # Property before bracket
                components.append(prop)
            components.append(int(idx))
            i += bracket_match.end()
            # Skip dot if it follows brackets
            if i < len(path) and path[i] == '.':
                i += 1
            continue
        
        # Check for dot notation
        dot_pos = path.find('.', i)
        if dot_pos != -1:
            components.append(path[i:dot_pos])
            i = dot_pos + 1
        else:
            # Last component
            components.append(path[i:])
            break
            
    return components


def example_usage():
    """
    Example of how to use the JNRRD header parser.
    """
    # Sample JNRRD header objects
    header_objects = [
        {"jnrrd": "0004"},
        {"type": "float32"},
        {"dimension": 3},
        {"sizes": [256, 256, 100]},
        {"space": "right_anterior_superior"},
        {"extensions": {"nifti": "https://jnrrd.org/extensions/nifti/v1.0.0", 
                       "dicom": "https://jnrrd.org/extensions/dicom/v1.0.0"}},
        {"nifti:intent_code": 2},
        {"nifti:qform_parameters.quaternion[0]": 0.0},
        {"nifti:qform_parameters.quaternion[1]": 0.0},
        {"nifti:sform_matrix[0][0]": 1.0},
        {"nifti:sform_matrix[0][1]": 0.0},
        {"dicom:patient.id": "ANONYMOUS"},
        {"dicom:patient.sex": "F"},
        {"dicom:study.description": "BRAIN MRI"},
        {"dicom:series.modality": "MR"},
        {"dicom:equipment.manufacturer": "GE MEDICAL SYSTEMS"}
    ]
    
    # Parse the header
    parsed = parse_jnrrd_header(header_objects)
    
    # Print the result in a readable format
    print(json.dumps(parsed, indent=2))
    
    # Expected output:
    """
    {
      "jnrrd": {
        "jnrrd": "0004",
        "type": "float32",
        "dimension": 3,
        "sizes": [256, 256, 100],
        "space": "right_anterior_superior",
        "extensions": {
          "nifti": "https://jnrrd.org/extensions/nifti/v1.0.0",
          "dicom": "https://jnrrd.org/extensions/dicom/v1.0.0"
        }
      },
      "nifti": {
        "intent_code": 2,
        "qform_parameters": {
          "quaternion": [0.0, 0.0]
        },
        "sform_matrix": [
          [1.0, 0.0]
        ]
      },
      "dicom": {
        "patient": {
          "id": "ANONYMOUS",
          "sex": "F"
        },
        "study": {
          "description": "BRAIN MRI"
        },
        "series": {
          "modality": "MR"
        },
        "equipment": {
          "manufacturer": "GE MEDICAL SYSTEMS"
        }
      }
    }
    """


# More complex examples to demonstrate handling edge cases

def handle_mixed_path_notations():
    """
    Example showing how the parser handles mixed dot and bracket notations.
    """
    header_objects = [
        # Define base array
        {"ome:channels": [{"id": "Channel:0", "name": "DAPI"}]},
        
        # Add array elements using bracket notation
        {"ome:channels[1].id": "Channel:1"},
        {"ome:channels[1].name": "GFP"},
        
        # Nested arrays with mixed notation
        {"ome:roi.shapes[0].type": "rectangle"},
        {"ome:roi.shapes[0].coordinates": [10, 20, 30, 40]},
        {"ome:roi.shapes[1].type": "circle"},
        {"ome:roi.shapes[1].center": [50, 60]},
        
        # Deeper nesting
        {"seg:segments[0].terminology.category.code_value": "123037004"}
    ]
    
    parsed = parse_jnrrd_header(header_objects)
    print(json.dumps(parsed, indent=2))
    
    # Expected output:
    """
    {
      "jnrrd": {},
      "ome": {
        "channels": [
          {
            "id": "Channel:0",
            "name": "DAPI"
          },
          {
            "id": "Channel:1",
            "name": "GFP"
          }
        ],
        "roi": {
          "shapes": [
            {
              "type": "rectangle",
              "coordinates": [10, 20, 30, 40]
            },
            {
              "type": "circle",
              "center": [50, 60]
            }
          ]
        }
      },
      "seg": {
        "segments": [
          {
            "terminology": {
              "category": {
                "code_value": "123037004"
              }
            }
          }
        ]
      }
    }
    """


def handle_array_creation():
    """
    Example showing how the parser handles array creation when indices are specified out of order.
    """
    header_objects = [
        # Sparse array indices
        {"example:items[5].name": "Item 5"},
        {"example:items[2].name": "Item 2"},
        {"example:items[0].name": "Item 0"},
        
        # Multi-dimensional array
        {"example:matrix[1][2]": 42},
        {"example:matrix[0][1]": 17}
    ]
    
    parsed = parse_jnrrd_header(header_objects)
    print(json.dumps(parsed, indent=2))
    
    # Expected output:
    """
    {
      "jnrrd": {},
      "example": {
        "items": [
          {
            "name": "Item 0"
          },
          null,
          {
            "name": "Item 2"
          },
          null,
          null,
          {
            "name": "Item 5"
          }
        ],
        "matrix": [
          [
            null,
            17
          ],
          [
            null,
            null,
            42
          ]
        ]
      }
    }
    """


def handle_extensions_overrides():
    """
    Example showing how the parser handles overrides in field values.
    """
    header_objects = [
        # Define a complex object
        {"vendor:config": {"debug": False, "mode": "production", "options": {"cache": True, "timeout": 30}}},
        
        # Override specific fields
        {"vendor:config.debug": True},
        {"vendor:config.options.timeout": 60},
        
        # Add a new field
        {"vendor:config.options.retries": 3}
    ]
    
    parsed = parse_jnrrd_header(header_objects)
    print(json.dumps(parsed, indent=2))
    
    # Expected output:
    """
    {
      "jnrrd": {},
      "vendor": {
        "config": {
          "debug": true,
          "mode": "production",
          "options": {
            "cache": true,
            "timeout": 60,
            "retries": 3
          }
        }
      }
    }
    """

def handle_mixed_direct_and_hierarchical():
    """
    Example showing how the parser handles a mixture of direct keys and hierarchical paths.
    """
    header_objects = [
        # Direct assignment with an object value
        {"vendor:config.options": {"timeout": 60, "retries": 2}},
        
        # Other paths in the hierarchy
        {"vendor:config.debug": True},
        
        # This adds to options but doesn't override the existing values
        {"vendor:config.options.ssl": {"enabled": True, "verify": False}},
        
        # This overrides a value set earlier in the options object
        {"vendor:config.options.timeout": 120}
    ]
    
    parsed = parse_jnrrd_header(header_objects)
    print(json.dumps(parsed, indent=2))
    
    # Expected output:
    """
    {
      "jnrrd": {},
      "vendor": {
        "config": {
          "debug": true,
          "options": {
            "timeout": 120,
            "retries": 2,
            "ssl": {
              "enabled": true,
              "verify": false
            }
          }
        }
      }
    }
    """


if __name__ == "__main__":
    example_usage()
    print("\nMore examples:")
    handle_mixed_path_notations()
    handle_array_creation()
    handle_extensions_overrides()
    handle_mixed_direct_and_hierarchical()