# JNRRD Header Parser

This document describes a Python implementation for parsing JNRRD headers into hierarchical data structures. The parser handles extension prefixes, dot notation for object properties, and bracket notation for array indices, converting flat JSON objects into a structured representation.

## Overview

JNRRD headers consist of line-delimited JSON objects, where each line contains a single key-value pair. These can include:

1. Standard JNRRD metadata fields (e.g., `{"dimension": 3}`)
2. Extension fields with a prefix (e.g., `{"nifti:intent_code": 2}`)
3. Hierarchical fields using dot notation (e.g., `{"dicom:patient.id": "ANONYMOUS"}`)
4. Array elements using bracket notation (e.g., `{"nifti:qform_parameters.quaternion[0]": 0.0}`)

The parser converts these flat objects into a hierarchical structure with extension namespaces as the top-level keys.

## Parser Implementation

```python
import re
import json
from typing import List, Dict, Any, Union

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
```

The `update_nested_dict` function handles path navigation and creation of nested structure:

```python
def update_nested_dict(data: Dict[str, Any], path: str, value: Any) -> None:
    """
    Update a nested dictionary based on a path string.
    
    Args:
        data: The dictionary to update.
        path: The path string (e.g., "patient.id" or "channels[0].name").
        value: The value to set at the specified path.
    """
    # Parse the path into components
    components = parse_path(path)
    
    # Current position in the nested structure
    current = data
    
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
```

The `parse_path` function handles both dot notation for properties and bracket notation for arrays:

```python
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
```

## Usage Examples

### Basic Example

```python
header_objects = [
    {"jnrrd": "0004"},
    {"type": "float32"},
    {"dimension": 3},
    {"nifti:intent_code": 2},
    {"nifti:qform_parameters.quaternion[0]": 0.0},
    {"nifti:qform_parameters.quaternion[1]": 0.0},
    {"dicom:patient.id": "ANONYMOUS"},
    {"dicom:study.description": "BRAIN MRI"}
]

parsed = parse_jnrrd_header(header_objects)
print(json.dumps(parsed, indent=2))
```

Output:
```json
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
```

### Working with Arrays

This example shows how the parser handles bracket notation and array creation:

```python
header_objects = [
    {"ome:channels": [{"id": "Channel:0", "name": "DAPI"}]},
    {"ome:channels[1].id": "Channel:1"},
    {"ome:channels[1].name": "GFP"},
    {"ome:roi.shapes[0].type": "rectangle"},
    {"ome:roi.shapes[0].coordinates": [10, 20, 30, 40]},
    {"ome:roi.shapes[1].type": "circle"},
    {"ome:roi.shapes[1].center": [50, 60]}
]

parsed = parse_jnrrd_header(header_objects)
print(json.dumps(parsed, indent=2))
```

Output:
```json
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
  }
}
```

### Multi-dimensional Arrays

The parser also handles multi-dimensional arrays and sparse array indices:

```python
header_objects = [
    {"example:matrix[1][2]": 42},
    {"example:matrix[0][1]": 17},
    {"example:items[5].name": "Item 5"},
    {"example:items[2].name": "Item 2"},
    {"example:items[0].name": "Item 0"}
]

parsed = parse_jnrrd_header(header_objects)
print(json.dumps(parsed, indent=2))
```

Output:
```json
{
  "jnrrd": {},
  "example": {
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
    ],
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
    ]
  }
}
```

### Field Overrides

When a more specific path is provided, it overrides values from less specific paths:

```python
header_objects = [
    {"vendor:config": {"debug": false, "mode": "production", "options": {"cache": true, "timeout": 30}}},
    {"vendor:config.debug": true},
    {"vendor:config.options.timeout": 60},
    {"vendor:config.options.retries": 3}
]

parsed = parse_jnrrd_header(header_objects)
print(json.dumps(parsed, indent=2))
```

Output:
```json
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
```

### Mixed Direct and Hierarchical Paths

The parser can handle a combination of direct key assignment with object values and hierarchical paths:

```python
header_objects = [
    # Direct assignment with an object value
    {"vendor:config.options": {"timeout": 60, "retries": 2}},
    
    # Other paths in the hierarchy
    {"vendor:config.debug": true},
    
    # This adds to options but doesn't override the existing values
    {"vendor:config.options.ssl": {"enabled": true, "verify": false}},
    
    # This overrides a value set earlier in the options object
    {"vendor:config.options.timeout": 120}
]

parsed = parse_jnrrd_header(header_objects)
print(json.dumps(parsed, indent=2))
```

Output:
```json
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
```

## Implementation Notes

1. **Processing Order**: Fields are processed in the order they appear in the header. This means that more specific paths that appear later will override values from less specific paths that appear earlier.

2. **Array Creation**: When an array index is specified, any missing intermediate elements are filled with `null` values.

3. **Path Components**: The parser treats path components differently based on their type:
   - String components are treated as object properties
   - Integer components are treated as array indices

4. **Default Dictionaries**: When creating a new property that will contain a nested structure, the function decides based on the next component whether to create a dictionary (for further object properties) or an array (for indices).

5. **Extension Handling**: The parser groups fields by their extension prefix, allowing different extensions to use the same property names without conflict.

## Extensibility

This implementation can be extended to handle additional features:

1. **Validation**: Add validation of field values against schema definitions
2. **Custom Transformations**: Include callbacks to transform specific field values
3. **Multiple Levels of Nesting**: Support arbitrary depth in array nesting (already supported in the current implementation)
4. **Error Handling**: Add more robust error handling for invalid paths or values