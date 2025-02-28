# NRRDJSON Format Specification
Version: 1.0.0

## Overview

NRRDJSON (Newline-delimited JSON for Nearly Raw Raster Data) is a file format designed to combine the structural advantages of JSON with the binary efficiency of NRRD (Nearly Raw Raster Data). It aims to provide a more extensible and standardized approach to storing scientific and medical imaging data.

## Format Structure

A NRRDJSON file consists of two primary components:
1. A text-based header containing metadata, stored as newline-delimited JSON objects
2. A binary data section containing the raw image or volume data

### Header Format

The header consists of a sequence of JSON objects, with exactly one complete JSON object per line. Each line represents a metadata field, with one key-value pair per object. The end of the header is determined by either:
- The first line that does not contain a valid JSON object
- A blank line (optional, for readability)

Example header:
```
{"NRRD": "0004"}
{"type": "short"}
{"dimension": 3}
{"sizes": [256, 256, 100]}
{"endian": "little"}
{"encoding": "raw"}
{"space": "right_anterior_superior"}

[BINARY DATA FOLLOWS]
```

### Required Fields

The following fields are required:
- `type`: Data type (e.g., "short", "float", "double")
- `dimension`: Number of dimensions in the dataset
- `sizes`: Array of dimension sizes

### Common Fields

These fields are commonly used:
- `NRRD`: Version identifier (e.g., "0004")
- `endian`: Byte order ("little" or "big")
- `encoding`: Data encoding method (typically "raw" for binary data)
- `space`: Coordinate system specification
- `space_directions`: Array of direction vectors
- `space_origin`: Origin point coordinates
- `spacings`: Array of spacing values along each dimension
- `units`: Units of measurement for each dimension

### Binary Data

The binary data section begins immediately after the header. There is no special marker between the header and the data; the parser determines the start of binary data by finding the first line that is not a valid JSON object.

## Extensions Mechanism

NRRDJSON includes a robust extension mechanism to support custom metadata:

### Extension Declaration

Extensions are declared using the `extensions` field:

```
{"extensions": {"acme": "https://acmeimaging.example.com/formats/nrrdjson/v2.1.3", "stanford": "https://med.stanford.example.edu/imaging/formats/tissue-meta/v1.0.0"}}
```

Each extension is identified by:
- A namespace key (e.g., "acme", "stanford")
- A URL value that uniquely identifies the extension and its version

### Extension Fields

Extension fields use a prefix notation with the namespace:

```
{"acme:sequence": "T1-weighted"}
{"acme:contrast": true}
{"stanford:tissue_classification": {"method": "deeplearn-v3", "classes": 8}}
```

## Implementation Guidelines

### Reading NRRDJSON Files

1. Read the file line by line until encountering a line that is not a valid JSON object
2. Parse each line as a JSON object and add its key-value pair to the header information
3. Record the file position where the binary data begins
4. Interpret the binary data according to the header specifications

Example Python parser:
```python
import json
import numpy as np

def read_nrrdjson(filename):
    header = {}
    binary_start_pos = 0
    
    with open(filename, 'rb') as f:
        while True:
            line_start = f.tell()
            line = f.readline()
            
            if not line or not line.strip():  # Empty line or EOF
                binary_start_pos = line_start
                f.seek(binary_start_pos)
                break
            
            try:
                field = json.loads(line)
                header.update(field)
            except json.JSONDecodeError:
                binary_start_pos = line_start
                f.seek(binary_start_pos)
                break
        
        # Map NRRD types to NumPy types
        type_map = {
            'short': np.int16,
            'ushort': np.uint16,
            'int': np.int32,
            'uint': np.uint32,
            'float': np.float32,
            'double': np.float64
        }
        
        dimensions = header.get('dimension', 0)
        sizes = header.get('sizes', [])
        data_type = header.get('type', 'float')
        numpy_dtype = type_map.get(data_type, np.float32)
        
        # Read binary data
        binary_data = f.read()
        
        # Create array from binary data
        array_data = np.frombuffer(binary_data, dtype=numpy_dtype)
        
        # Reshape if dimensions are provided
        if dimensions and sizes:
            array_data = array_data.reshape(sizes)
        
        return header, array_data
```

### Writing NRRDJSON Files

1. Write each metadata field as a separate JSON object on its own line
2. Include a blank line after the header (optional, for readability)
3. Write the binary data immediately after the header

Example Python writer:
```python
def write_nrrdjson(filename, header, array_data):
    with open(filename, 'wb') as f:
        # Write each header field as a separate JSON line
        for key, value in header.items():
            if isinstance(value, dict):
                line = json.dumps({key: value}).encode('utf-8') + b'\n'
                f.write(line)
            else:
                line = json.dumps({key: value}).encode('utf-8') + b'\n'
                f.write(line)
        
        # Add a blank line for readability (optional)
        f.write(b'\n')
        
        # Write binary data
        f.write(array_data.tobytes())
```

## Handling Newlines in Values

When a value needs to contain actual newline characters, they should be escaped as `\n` within the JSON string, following standard JSON escaping rules:

```
{"description": "This is a multi-line\ndescription of an imaging dataset."}
```

## Compatibility with NRRD

NRRDJSON is designed to be conceptually compatible with NRRD. Key differences:
- NRRD uses a custom text format for its header, while NRRDJSON uses JSON
- NRRDJSON provides explicit support for extensions via the namespace mechanism
- NRRDJSON requires special handling of newlines in string values (escaped as `\n`)
- NRRDJSON does not allow field name abbreviations or multiple field name variations
- NRRDJSON enforces a single canonical name for each field
- NRRDJSON uses underscores instead of spaces or dashes in field names

## Strict Field Naming

Unlike NRRD, NRRDJSON enforces strict rules for field names to ensure consistency and eliminate ambiguity:

1. No abbreviations or alternative spellings for field names
2. No spaces in field names - use underscores instead
3. No dashes in field names - use underscores instead

Each field has exactly one canonical name that must be used.

For example:
- Use `dimension` (not `dim`)
- Use `space_directions` (not `spacedirections` or `space directions`) 
- Use `space_origin` (not `spaceorigin` or `space origin`)

## Best Practices

1. Use the canonical field names as defined in this specification
2. Register extensions with unique and stable URLs
3. Include documentation for custom extensions
4. Use appropriate data types for field values (arrays for multi-dimensional data)
5. Include units and coordinate system information for scientific data
6. Consider using a blank line separator between header and binary data for readability
7. When reading, be permissive about unknown fields to support forward compatibility

## Example Complete NRRDJSON File

```
{"NRRD": "0004"}
{"type": "short"}
{"dimension": 3}
{"sizes": [256, 256, 100]}
{"endian": "little"}
{"encoding": "raw"}
{"space": "right_anterior_superior"}
{"space_directions": [[1, 0, 0], [0, 1, 0], [0, 0, 1]]}
{"space_origin": [0, 0, 0]}
{"spacings": [0.5, 0.5, 1.2]}
{"units": ["mm", "mm", "mm"]}
{"extensions": {"acme": "https://acmeimaging.example.com/formats/nrrdjson/v2.1.3"}}
{"acme:sequence": "T1_weighted"}
{"acme:contrast": true}

[BINARY DATA FOLLOWS]
```

## Version History

- 1.0.0: Initial specification