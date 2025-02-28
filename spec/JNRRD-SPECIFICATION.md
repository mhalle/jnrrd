# JNRRD: JSON-based Nearly Raw Raster Data Format Specification
Version: 1.0.0

## 1. Introduction

JNRRD (JSON-based Nearly Raw Raster Data) is a modern evolution of the NRRD file format, designed to combine the structural advantages of JSON with the efficiency of binary raster data storage. This specification defines the JNRRD format, its capabilities, and implementation guidelines.

JNRRD aims to preserve the key concepts of NRRD (Nearly Raw Raster Data) while leveraging JSON's flexibility, extensibility, and widespread adoption. Like NRRD, JNRRD provides a framework for storing scientific and medical imaging data with comprehensive metadata, but with improved interoperability and extensibility.

### 1.1 Design Goals

- Maintain NRRD's powerful capabilities for describing n-dimensional arrays
- Leverage JSON's standardized format and universal support
- Provide robust metadata capabilities with a clear extension mechanism
- Support all scientific data types and orientations
- Ensure backward compatibility with NRRD concepts
- Improve readability and reduce ambiguity through standardized naming

## 2. Basic Structure and Syntax

### 2.1 Format Identification

JNRRD files are identified by a required JSON object at the beginning of the file:

```json
{"jnrrd": "0004"}
```

This serves the same purpose as the NRRD magic line. The version number indicates the features supported by the format:

- `0004`: Initial version with core NRRD features expressed through JSON

### 2.2 File Structure Overview

A JNRRD file consists of two distinct sections:
1. A header containing metadata as line-delimited JSON objects
2. A binary data section containing the raw array data

### 2.3 Header Structure and Syntax

The header consists of a sequence of JSON objects, with exactly one complete object per line. Each line represents a metadata field with a single key-value pair:

```
{"jnrrd": "0004"}
{"type": "float"}
{"dimension": 3}
{"sizes": [256, 256, 100]}
...
```

> **Note:** Throughout this specification, many JSON examples are shown pretty-printed with indentation and line breaks for readability. In an actual JNRRD file, each JSON object must be on a single line without internal newlines, even for complex nested structures.

The header ends when:
- A line does not contain a valid JSON object, or
- A blank line is encountered (recommended for readability)

### 2.4 JSON Encoding Requirements

- Each line must contain exactly one valid JSON object with a single key-value pair
- The JSON encoding must be UTF-8
- Line termination may be either LF (`\n`) or CRLF (`\r\n`)
- JSON values must follow the standard JSON specification (RFC 8259)
- String values containing newlines must escape them as `\n` following JSON standards
- Field names must follow the canonical naming convention defined in this specification
- Field names are case-sensitive

### 2.5 Handling Newlines and Whitespace

In standard JSON, newlines within strings are escaped as `\n`. This is crucial in JNRRD since each line must represent exactly one complete JSON object.

For example, a multi-line description would be encoded as:
```json
{"description": "Line 1\nLine 2\nLine 3"}
```

Whitespace at the beginning or end of a line is ignored, but whitespace within the JSON object must follow JSON standards.

## 3. Core Header Fields

### 3.1 Required Fields

The following fields are required in every JNRRD file:

| Field | Type | Description |
|-------|------|-------------|
| `jnrrd` | string | Format version identifier (e.g., "0004") |
| `type` | string | Data type (see Section 5.4) |
| `dimension` | number | Number of dimensions in the dataset |
| `sizes` | array | Array of dimension sizes [size₀, size₁, ..., sizeₙ₋₁] |
| `encoding` | string | Data encoding method (e.g., "raw", "gzip") |

### 3.2 Common Optional Fields

These fields are commonly used but not required:

| Field | Type | Description |
|-------|------|-------------|
| `endian` | string | Byte order ("little" or "big") |
| `space` | string | Coordinate system specification |
| `space_dimension` | number | Dimension of the space |
| `space_directions` | array | Array of direction vectors |
| `space_origin` | array | Origin point coordinates |
| `spacings` | array | Array of spacing values along each dimension |
| `centers` | array | Array of centering types ("cell" or "node") |
| `kinds` | array | Array of axis kinds |
| `labels` | array | Array of axis labels |
| `units` | array | Array of measurement units |
| `content` | string | Description of the data content |
| `measurement_frame` | array | Orientation frame for vector/tensor quantities |

## 4. Extension Mechanism

### 4.1 Extension Overview

The JNRRD extension mechanism provides a standardized way to incorporate domain-specific metadata without modifying the core specification. Extensions allow different domains (medical imaging, scientific computing, microscopy, etc.) to define their own metadata conventions while maintaining compatibility with standard JNRRD readers.

Extensions in JNRRD are:
- **Namespaced**: Each extension has a unique prefix to avoid field name collisions
- **Self-documenting**: Extensions include a URI that points to their specification
- **Validated**: Extensions can use JSON Schema for validation
- **Optional**: Readers can ignore unknown extensions
- **Discoverable**: The `extensions` field lists all extensions used

### 4.2 Declaring Extensions

Extensions are declared in the `extensions` field, which maps namespace prefixes to URIs:

```json
{"extensions": {
  "metadata": "https://jnrrd.org/extensions/metadata/v1.0.0"
}}
```

The URI should point to documentation about the extension, preferably a machine-readable schema.

### 4.3 Using Extensions

Extension fields use a prefix notation with the namespace:

```json
{"metadata:name": "T1-weighted Brain MRI Dataset"}
{"metadata:license": "https://creativecommons.org/licenses/by/4.0/"}
```

Extension field values can be any valid JSON value (string, number, boolean, object, or array).

### 4.4 Hierarchical Extension Fields

Extensions can represent hierarchical data in two equivalent ways:

1. **Nested objects** (compact representation):
```json
{"metadata:creator": {"type": "Organization", "name": "Medical Research Lab"}}
```

2. **Flattened hierarchy** using JSONPath dot notation:
```json
{"metadata:creator.type": "Organization"}
{"metadata:creator.name": "Medical Research Lab"}
```

These two representations are semantically equivalent. In the flattened hierarchy approach:
- The colon (`:`) separates the extension namespace from the root object
- The dot (`.`) is used to navigate through object hierarchies
- Numeric indices in brackets (`[n]`) are used to access array elements

This approach has multiple benefits:
- Keeps individual JSON lines shorter and more readable
- Allows partial updates to hierarchical data
- Enables direct access to deeply nested values
- Follows standard JSONPath conventions

There is no limit to the depth of the hierarchy that can be represented:
```json
{"metadata:level1.level2.level3.level4": "deeply_nested_value"}
```

### 4.5 Metadata Extension Example

The metadata extension provides a simple example of using extensions:

```json
{"extensions": {"metadata": "https://jnrrd.org/extensions/metadata/v1.0.0"}}
{"metadata:name": "T1-weighted Brain MRI Dataset"}
{"metadata:description": "3D T1-weighted MRI brain scan of healthy adult subject"}
{"metadata:license": "https://creativecommons.org/licenses/by/4.0/"}
{"metadata:creator": {
  "type": "Organization",
  "name": "Medical Research Lab",
  "url": "https://example.org/lab"
}}
{"metadata:dateCreated": "2021-08-23"}
{"metadata:contentUrl": "https://example.org/datasets/brain0001.jnrrd"}
{"metadata:keywords": ["neuroimaging", "T1-weighted", "MRI", "brain"]}
```

The JSON Schema at `https://jnrrd.org/extensions/metadata/v1.0.0` would define properties like `name`, `description`, `license`, etc., without the `metadata:` prefix that appears in the JNRRD file. When validating, the validation process strips the prefix from field names before comparing them to the schema.

### 4.6 Priority Rules for Hierarchical Fields

When a field appears in both formats, the flattened hierarchy (more specific path) takes precedence:
```json
{"metadata:creator": {"type": "Organization", "name": "Original Lab"}}
{"metadata:creator.name": "Updated Lab Name"}
```

In this example, the effective value would be:
```json
{"metadata:creator": {"type": "Organization", "name": "Updated Lab Name"}}
```

Array elements can also be accessed using numeric indices in paths with bracket notation:

```json
{"metadata:authors": [{"name": "Author1"}, {"name": "Author2"}]}
{"metadata:authors[1].name": "Updated Author2"}
{"metadata:authors[2].name": "Author3"}  // Appends a new element
```

The effective value would be:
```json
{"metadata:authors": [
  {"name": "Author1"},
  {"name": "Updated Author2"},
  {"name": "Author3"}
]}
```

### 4.7 Mixed Hierarchical Path Formats

The two hierarchical representations can be mixed within the same set of fields, allowing flexibility in how data is organized. This is particularly useful for:

1. **Setting groups of related values at once**:
```json
{"metadata:institution.address": {"city": "Boston", "state": "MA", "country": "USA"}}
```

2. **Modifying or extending specific nested values**:
```json
{"metadata:institution.address.city": "Cambridge"}
```

When both approaches are used, the more specific path always takes precedence over the less specific one.

### 4.8 Parser Implementation

A reference implementation for parsing hierarchical extension fields in JNRRD headers is available in [jnrrd_header_parser.md](./jnrrd_header_parser.md). This implementation:

1. **Handles dot notation** for object properties (e.g., `metadata:creator.name`)
2. **Handles bracket notation** for array indices (e.g., `nifti:quaternion[0]`)
3. **Creates nested structures** automatically based on path components
4. **Handles field overrides** where more specific paths take precedence
5. **Separates extensions** by namespace prefixes
6. **Merges objects** with complementary paths

The parser provides a consistent way to convert from the flat, line-based JNRRD header format to a fully hierarchical data structure that can be used by applications.

### 4.9 Writer Responsibility

The writer of a JNRRD file is responsible for:

1. **Maintaining hierarchical consistency**: When updating a field that appears in both flattened and nested formats, the writer should ensure that all related fields are updated appropriately.

2. **Proper extension declarations**: All extension prefixes used in the file should be declared in the `extensions` field before they are used.

3. **Schema compliance**: Writers should ensure that after extension prefixes are stripped, the resulting data structure would comply with the schema referenced in the extension declaration.

4. **Prefix consistency**: Each extension should use a consistent prefix throughout the file, as declared in the `extensions` field.

## 5. Data Representation

### 5.1 Data Types

JNRRD supports the following data types using standardized, numeric naming conventions:

#### 5.1.1 Integer Types

| Type String | Description | Bytes per Element |
|-------------|-------------|-------------------|
| `int8` | 8-bit signed integer | 1 |
| `uint8` | 8-bit unsigned integer | 1 |
| `int16` | 16-bit signed integer | 2 |
| `uint16` | 16-bit unsigned integer | 2 |
| `int32` | 32-bit signed integer | 4 |
| `uint32` | 32-bit unsigned integer | 4 |
| `int64` | 64-bit signed integer | 8 |
| `uint64` | 64-bit unsigned integer | 8 |

#### 5.1.2 Floating Point Types

| Type String | Description | Bytes per Element | Notes |
|-------------|-------------|-------------------|-------|
| `float16` | 16-bit IEEE 754 half-precision | 2 | For AI/ML applications |
| `bfloat16` | 16-bit Brain Floating Point | 2 | For AI/ML applications |
| `float32` | 32-bit IEEE 754 single-precision | 4 | |
| `float64` | 64-bit IEEE 754 double-precision | 8 | |

#### 5.1.3 Special Types

| Type String | Description | Bytes per Element | Notes |
|-------------|-------------|-------------------|-------|
| `block` | Opaque memory block | Variable | Requires `block_size` field |
| `complex64` | Complex number (2× 32-bit float) | 8 | real and imaginary components |
| `complex128` | Complex number (2× 64-bit float) | 16 | real and imaginary components |

> **Note on NRRD Compatibility**: When converting from NRRD to JNRRD, implementations should map C-style type names (`signed_char`, `unsigned_char`, `short`, `int`, `float`, `double`, etc.) to their numeric equivalents (`int8`, `uint8`, `int16`, `int32`, `float32`, `float64`, etc.). This standardization improves compatibility with modern programming languages and scientific computing libraries.

### 5.2 Encodings

JNRRD supports the following encoding methods:

| Encoding | Description |
|----------|-------------|
| `raw` | Uncompressed binary data |
| `ascii` | ASCII text representation of values |
| `hex` | Hexadecimal representation of binary data |
| `gzip` | Gzip-compressed binary data |
| `bzip2` | Bzip2-compressed binary data |
| `zstd` | Zstandard-compressed binary data (new in JNRRD) |
| `lz4` | LZ4-compressed binary data (new in JNRRD) |

### 5.3 Special Field: Detached Data

For detached headers (where data is stored separately), use the `data_file` field:

```json
{"data_file": "image_data.raw"}
```

Multiple data files can be specified using an array:
```json
{"data_files": ["slice001.raw", "slice002.raw", "slice003.raw"]}
```

Or using a pattern:
```json
{"data_file_pattern": {"format": "slice%03d.raw", "min": 1, "max": 100, "step": 1}}
```

## 6. Advanced Extension Features

JNRRD includes several standardized extensions that are available for specialized use cases. Complete specifications for each extension are maintained in separate documents.

### 6.1 NIfTI Extension

The [NIfTI extension](./nifti-extension.md) provides fields for neuroimaging data compatibility with the NIfTI format:

```json
{"extensions": {"nifti": "https://jnrrd.org/extensions/nifti/v1.0.0"}}
{"nifti:intent_code": 3}
{"nifti:intent_name": "Correlation"}
{"nifti:qform_code": 1}
{"nifti:sform_code": 4}
```

This can be represented with both nested objects and flattened hierarchy:

Nested object approach:
```json
{"nifti:qform_quaternion": {"b": 0.0, "c": 0.0, "d": 0.0, "a": 1.0, "qx": 0.0, "qy": 0.0, "qz": 0.0, "dx": 0.0, "dy": 0.0, "dz": 0.0}}
```

Flattened hierarchy approach:
```json
{"nifti:qform_quaternion.a": 1.0}
{"nifti:qform_quaternion.b": 0.0}
{"nifti:qform_quaternion.c": 0.0}
```

In the NIfTI extension's JSON Schema, these properties are defined without their prefixes. During validation, the `nifti:` prefix is stripped from field names, so `nifti:intent_code` is validated against the schema's `intent_code` property. See the [NIfTI extension specification](./nifti-extension.md) for full details.

### 6.2 DICOM Extension

The [DICOM extension](./dicom-extension.md) includes fields for compatibility with medical imaging workflows:

```json
{"extensions": {"dicom": "https://jnrrd.org/extensions/dicom/v1.0.0"}}
{"dicom:patient": {
  "id": "12345",
  "name": "ANONYMOUS",
  "birth_date": "19800101",
  "sex": "F"
}}
{"dicom:study": {
  "instance_uid": "1.2.840.113619.2.334.3.2831183778.864.1629754903.547",
  "date": "20210823",
  "time": "152143",
  "description": "BRAIN MRI"
}}
```

When validating, these fields are matched against a schema that defines properties without the `dicom:` prefix. The full DICOM extension specification, including all supported fields and validation rules, is available in the [DICOM extension specification](./dicom-extension.md).

### 6.3 Additional Extensions

JNRRD supports several other specialized extensions:

- [FITS extension](./fits-extension.md) - For astronomical data compatibility
- [OME extension](./ome-extension.md) - For microscopy metadata compatibility
- [Tiling extension](./tiling-extension.md) - For multi-resolution tiled access to large datasets

### 6.4 Creating Custom Extensions

To create a custom extension:

1. Choose a namespace prefix that identifies your organization or domain
2. Create documentation that describes all fields and their meaning
3. Host the documentation at a stable URI, including a JSON Schema for validation
4. Use the namespace prefix for all extension fields in JNRRD files
5. Declare the extension in the `extensions` field before using any extension fields
6. **Important**: Your extension's JSON Schema should define properties without prefixes, as the validation process will strip these prefixes before validation

Example of a custom microscopy extension in a JNRRD file:

```json
{"extensions": {"microscopy": "https://example.org/jnrrd-extensions/microscopy/v1"}}
{"microscopy:instrument": "Zeiss LSM 880"}
{"microscopy:objective": "63x/1.4 Oil Plan-Apochromat"}
{"microscopy:fluorophores": ["DAPI", "Alexa Fluor 488", "Alexa Fluor 647"]}
```

In this example, the schema at `https://example.org/jnrrd-extensions/microscopy/v1` would define properties like `instrument`, `objective`, and `fluorophores` without the `microscopy:` prefix that appears in the JNRRD file.

## 7. Validation using JSON Schema

### 7.1 Extension Validation with JSON Schema

JNRRD extensions can use standard JSON Schema for validation. When a parser encounters extension fields, it can collect all fields for a given extension, strip the extension prefix, and validate the resulting object against the schema provided at the extension's URL.

**Important**: Extension schemas define fields without the extension prefix. For example, the field `metadata:name` in a JNRRD file would be matched against the `name` property in the schema, as the `metadata:` prefix is stripped during validation.

### 7.2 Schema Publication

Extension authors should provide a JSON Schema at the extension URL. This schema defines the structure, required fields, and constraints for all fields in that extension. The schema should:

1. Use JSON Schema draft-07 or later
2. Include descriptions for all properties
3. Specify required fields
4. Define property types, formats, and constraints
5. Include examples where helpful
6. **Define properties without extension prefixes** (the prefix will be stripped before validation)

For example, a schema hosted at `https://jnrrd.org/extensions/metadata/v1.0.0` would define properties like `name`, `description`, and `license` (without the `metadata:` prefix), even though in JNRRD files these fields would appear as `metadata:name`, `metadata:description`, and `metadata:license`.

The documentation for extension schemas should clarify that prefixes are not part of the property names in the schema, but will be present in the actual JNRRD files. It can be helpful to include a note in property descriptions that shows how the property appears in JNRRD files (e.g., "Name of the dataset (stored as 'metadata:name' in JNRRD files)").

### 7.3 Validation Process

Parsers implement validation by:

1. Reading the extension declarations from the `extensions` field
2. Fetching the JSON Schema from each extension's URL
3. For each extension:
   - Collecting all fields with that extension's prefix (e.g., all fields starting with `metadata:`)
   - Removing the prefix from field names (e.g., `metadata:name` becomes just `name`)
   - Creating a consolidated object with all fields for that extension with their prefixes removed
   - Validating this consolidated object against the schema

This approach ensures that extension schemas can be written without knowledge of which prefix will be used in a particular JNRRD file, as the prefix is arbitrary and defined in the file's `extensions` declaration.

### 7.4 Validation Implementation Example

Python example for extension validation:

```python
import json
import requests
import jsonschema

def validate_extension(extension_name, extension_url, header):
    # Fetch schema from the extension URL
    response = requests.get(extension_url)
    if response.status_code != 200:
        raise ValueError(f"Failed to fetch schema for {extension_name}")
    
    schema_data = response.json()
    # The actual JSON Schema is in the "schema" field
    schema = schema_data.get("schema")
    
    # Extract all fields for this extension
    prefix = f"{extension_name}:"
    extension_fields = {}
    
    # Example: If extension_name is "metadata", we're looking for fields like "metadata:name"
    for key, value in header.items():
        if key.startswith(prefix):
            # Remove prefix - "metadata:name" becomes "name"
            field_name = key[len(prefix):]
            
            # Handle hierarchical keys
            # Convert dot notation paths with array indices to use bracket notation
            # For example, convert "items.0.name" to "items[0].name"
            import re
            field_name = re.sub(r'\.(\d+)\.', r'[\1].', field_name)
            # Handle the case where the array index is at the end of the path
            field_name = re.sub(r'\.(\d+)$', r'[\1]', field_name)
            
            # Store the value with the prefix-free field name
            extension_fields[field_name] = value
    
    if not extension_fields:
        return True  # No fields to validate
    
    # For demonstration, show what we're validating
    print(f"Validating {extension_name} fields: {json.dumps(extension_fields, indent=2)}")
    print(f"Against schema: {json.dumps(schema, indent=2)}")
    
    # Validate the prefix-free fields against the schema
    jsonschema.validate(instance=extension_fields, schema=schema)
    return True$', r'[\1]', field_name)
            
            # Store the value with the prefix-free field name
            extension_fields[field_name] = value
    
    if not extension_fields:
        return True  # No fields to validate
    
    # For demonstration, show what we're validating
    print(f"Validating {extension_name} fields: {json.dumps(extension_fields, indent=2)}")
    print(f"Against schema: {json.dumps(schema, indent=2)}")
    
    # Validate the prefix-free fields against the schema
    jsonschema.validate(instance=extension_fields, schema=schema)
    return True
```

## 8. Space and Orientation Information

### 8.1 Space Definition

The space in which the data exists is defined by either:
```json
{"space": "right_anterior_superior"}
```
or 
```json
{"space_dimension": 3}
```

Predefined spaces include:
- `right_anterior_superior` or `RAS` (3D)
- `left_anterior_superior` or `LAS` (3D)
- `left_posterior_superior` or `LPS` (3D)
- `right_anterior_superior_time` or `RAST` (4D)
- `left_anterior_superior_time` or `LAST` (4D)
- `left_posterior_superior_time` or `LPST` (4D)
- `scanner_xyz` (3D)
- `scanner_xyz_time` (4D)
- `3D_right_handed` (3D)
- `3D_left_handed` (3D)
- `3D_right_handed_time` (4D)
- `3D_left_handed_time` (4D)

### 8.2 Space Origin

The origin of the data in the defined space:
```json
{"space_origin": [0.0, 0.0, 0.0]}
```

### 8.3 Space Directions

The direction vectors for each axis:
```json
{"space_directions": [[1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0]]}
```

For non-spatial axes, use `null`:
```json
{"space_directions": [null, [1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0]]}
```

### 8.4 Measurement Frame

For vector/tensor quantities:
```json
{"measurement_frame": [[1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0]]}
```

## 9. Axis Information

### 9.1 Sizes

Required for all axes:
```json
{"sizes": [256, 256, 100]}
```

### 9.2 Spacings

Optional, defines sample spacing along each axis:
```json
{"spacings": [0.5, 0.5, 1.2]}
```

Use `null` for non-spatial axes.

### 9.3 Thicknesses

Optional, defines the sample thickness along each axis:
```json
{"thicknesses": [0.5, 0.5, 1.5]}
```

### 9.4 Axis Mins and Maxs

Optional, defines the world-space bounds of each axis:
```json
{"axis_mins": [0.0, 0.0, 0.0]}
{"axis_maxs": [128.0, 128.0, 120.0]}
```

### 9.5 Centers

Optional, defines if data is cell-centered or node-centered:
```json
{"centers": ["cell", "cell", "node"]}
```

Valid values: `"cell"`, `"node"`, `null`

### 9.6 Kinds

Optional, defines the kind of information along each axis:
```json
{"kinds": ["space", "space", "space"]}
```

Common values include:
- `domain`, `space`, `time` (independent variables)
- `vector`, `scalar`, `complex`, `rgb_color`, `rgba_color`
- `2d_matrix`, `3d_matrix`, `quaternion`, etc.

### 9.7 Labels

Optional, provides descriptive labels for each axis:
```json
{"labels": ["X", "Y", "Z"]}
```

### 9.8 Units

Optional, defines the units of measurement:
```json
{"units": ["mm", "mm", "mm"]}
```

## 10. Data Section

### 10.1 Data Format

The binary data immediately follows the header. The exact format depends on the `encoding`, `type`, and `endian` fields.

### 10.2 Line Skip and Byte Skip

For skipping data in detached data files:
```json
{"line_skip": 5}
{"byte_skip": 1024}
```

### 10.3 Remote Data Access

Unlike traditional NRRD which interprets file references as local filesystem paths, JNRRD implementations may interpret filenames in the `data_file` and `data_files` fields as URIs/URLs, enabling remote network access to file resources. This allows for distributed workflows where metadata and binary data can be stored and accessed from different network locations.

```json
{"data_file": "https://example.org/datasets/brain0001.raw"}
{"data_file": "s3://medical-imaging-bucket/patient123/ct-scan.raw"}
```

When using remote data access:

1. Implementations should support common protocols such as HTTP, HTTPS, FTP, and cloud storage URIs
2. Authentication may be required and should be handled according to the respective protocol
3. Caching strategies should be considered for efficient access to remote data
4. Error handling should include appropriate network-related failure modes

This feature enables JNRRD to work seamlessly in distributed computing environments, cloud-based workflows, and federated data repositories.

## 11. Specialized Extensions

### 11.1 Segmentation Extension

The JNRRD segmentation extension provides a more elegant way to represent segmentation data than the traditional NRRD approach which used awkward sequentially numbered keys (like `Segment0_Name`, `Segment1_Color`, etc.).

#### 11.1.1 Extension Declaration 

```json
{"extensions": {"seg": "https://jnrrd.org/extensions/segmentation/v1.0.0"}}
```

#### 11.1.2 Key Concepts

The segmentation extension introduces a structured approach to storing segmentation data:

1. **Segments Collection**: All segment information is stored in a single JSON array
2. **Terminology Support**: Standard medical terminology codes are natively supported
3. **Hierarchical Organization**: Related properties are logically grouped
4. **Clean Metadata**: No need for sequentially numbered keys

#### 11.1.3 Segmentation Fields

> **Note:** In the examples below, JSON is shown pretty-printed with indentation and newlines for clarity. In an actual JNRRD file, each JSON object must be on a single line without internal newlines, as specified in Section 2.2. For example, a complex structure like `seg:segments` would be encoded as a single, potentially long line in the file.

The primary field in the segmentation extension is:

```json
{"seg:segments": [
  {
    "id": "segment_1",
    "label_value": 1,
    "name": "ribs",
    "color": [0.992, 0.909, 0.619],
    "layer": 0,
    "extent": [0, 124, 0, 127, 0, 33],
    "terminology": {
      "context_name": "Segmentation category and type - 3D Slicer General Anatomy list",
      "category": {
        "coding_scheme": "SCT", 
        "code_value": "123037004", 
        "code_meaning": "Anatomical Structure"
      },
      "type": {
        "coding_scheme": "SCT", 
        "code_value": "113197003", 
        "code_meaning": "Rib"
      }
    },
    "status": "completed",
    "metadata": {
      "custom_property": "value"
    }
  },
  {
    "id": "segment_2",
    "label_value": 2,
    "name": "cervical_vertebrae",
    "color": [1.0, 1.0, 0.811],
    "layer": 0,
    "extent": [0, 124, 0, 127, 0, 33],
    "terminology": {
      "context_name": "Segmentation category and type - 3D Slicer General Anatomy list",
      "category": {
        "coding_scheme": "SCT", 
        "code_value": "123037004", 
        "code_meaning": "Anatomical Structure"
      },
      "type": {
        "coding_scheme": "SCT", 
        "code_value": "122494005", 
        "code_meaning": "Cervical spine"
      }
    },
    "status": "inprogress"
  }
]}
```

Additional segmentation-specific fields:

```json
{"seg:master_representation": "Binary labelmap"}
{"seg:contained_representations": ["Binary labelmap", "Closed surface"]}
{"seg:reference_image_extent_offset": [0, 0, 0]}
```

#### 11.1.4 Validation

When validating the segmentation extension, as with all extensions, the `seg:` prefix is stripped from the field names before validation. For example, the field `seg:segments` in the JNRRD file would be validated against the `segments` property in the schema hosted at the extension URI.

The schema referenced by `https://jnrrd.org/extensions/segmentation/v1.0.0` would define properties like `segments`, `master_representation`, and `contained_representations` without the `seg:` prefix that appears in the actual JNRRD file.

This segmentation approach provides benefits over the original NRRD design:

1. **Clean structure**: Logical hierarchical organization
2. **Better readability**: Related information is grouped together
3. **Standards compliance**: Native support for medical terminology
4. **Extensibility**: Easy to add new segment properties
5. **Array-based**: Proper use of JSON arrays eliminates numbered keys
6. **Improved validation**: Structure is easier to validate with JSON Schema

## 12. Implementation Guidelines

### 12.1 Reading JNRRD Files

1. Read the file line by line until a non-JSON line or blank line is encountered
2. Parse each line as a JSON object and extract the key-value pair
3. Store the file position at the beginning of binary data
4. Read and process the binary data according to the header information

### 12.2 Writing JNRRD Files

1. Write required metadata fields as JSON objects, one per line
2. Write optional metadata fields as needed
3. Add a blank line (recommended)
4. Write the binary data immediately after

### 12.3 Handling Special Values

For floating-point data:
- Use `null` for "unknown" values (equivalent to NaN in NRRD)
- Use standard JSON representation for infinity: `{"value": Infinity}` or `{"value": -Infinity}`

### 12.4 Conversion from NRRD

When converting from NRRD to JNRRD:
1. Replace spaces and hyphens in field names with underscores
2. Use canonical field names (see table below)
3. Convert field values to appropriate JSON types (arrays, objects, strings, numbers, etc.)
4. Use proper JSON string escaping for text fields

## 13. Field Name Mapping from NRRD to JNRRD

| NRRD Field | JNRRD Field |
|------------|-------------|
| `dimension` | `dimension` |
| `type` | `type` |
| `sizes` | `sizes` |
| `endian` | `endian` |
| `encoding` | `encoding` |
| `space` | `space` |
| `space dimension` | `space_dimension` |
| `space units` | `space_units` |
| `space origin` | `space_origin` |
| `space directions` | `space_directions` |
| `measurement frame` | `measurement_frame` |
| `content` | `content` |
| `block size` | `block_size` |
| `min` | `min` |
| `max` | `max` |
| `old min` | `old_min` |
| `old max` | `old_max` |
| `data file` | `data_file` |
| `line skip` | `line_skip` |
| `byte skip` | `byte_skip` |
| `sample units` | `sample_units` |
| `spacings` | `spacings` |
| `thicknesses` | `thicknesses` |
| `axis mins` | `axis_mins` |
| `axis maxs` | `axis_maxs` |
| `centers` | `centers` |
| `labels` | `labels` |
| `units` | `units` |
| `kinds` | `kinds` |

## 14. Core JNRRD JSON Schema

The following JSON Schema can be used to validate the core elements of JNRRD headers:

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "JNRRD Core Schema",
  "description": "Schema for validating core JNRRD header fields",
  "type": "object",
  "required": ["jnrrd", "type", "dimension", "sizes", "encoding"],
  "properties": {
    "jnrrd": {
      "type": "string",
      "description": "JNRRD format version identifier",
      "enum": ["0004"]
    },
    "type": {
      "type": "string",
      "description": "Data type of the array elements",
      "enum": [
        "int8",
        "uint8", 
        "int16",
        "uint16",
        "int32",
        "uint32",
        "int64",
        "uint64",
        "float16",
        "bfloat16",
        "float32",
        "float64",
        "complex64",
        "complex128",
        "block"
      ]
    },
    "dimension": {
      "type": "integer",
      "description": "Number of dimensions in the dataset",
      "minimum": 1,
      "maximum": 16
    },
    "sizes": {
      "type": "array",
      "description": "Size of each dimension",
      "items": {
        "type": "integer",
        "minimum": 1
      },
      "minItems": 1
    },
    "encoding": {
      "type": "string",
      "description": "Data encoding method",
      "enum": ["raw", "ascii", "txt", "text", "hex", "gzip", "gz", "bzip2", "bz2", "zstd", "lz4"]
    },
    "endian": {
      "type": "string",
      "description": "Byte order for multi-byte data types",
      "enum": ["little", "big"]
    },
    "space": {
      "type": "string",
      "description": "Coordinate system specification",
      "enum": [
        "right_anterior_superior", "RAS",
        "left_anterior_superior", "LAS",
        "left_posterior_superior", "LPS",
        "right_anterior_superior_time", "RAST",
        "left_anterior_superior_time", "LAST",
        "left_posterior_superior_time", "LPST",
        "scanner_xyz",
        "scanner_xyz_time",
        "3D_right_handed",
        "3D_left_handed",
        "3D_right_handed_time",
        "3D_left_handed_time"
      ]
    }
    // Additional schema properties omitted for brevity
  }
}
```

Usage example:

```python
import json
import jsonschema
import nrrd

# Load the JNRRD header
header = nrrd.read_header("image.jnrrd")

# Load the schema
with open("jnrrd_schema.json", "r") as f:
    schema = json.load(f)

# Validate the header against the schema
try:
    jsonschema.validate(instance=header, schema=schema)
    print("Header is valid JNRRD")
except jsonschema.exceptions.ValidationError as e:
    print(f"Header is not valid JNRRD: {e}")
```

This schema captures the core structural requirements of the JNRRD format while allowing for extension fields not defined in the core schema. The validation can be used to ensure files conform to the specification before attempting to read them.

## 15. Example JNRRD File

```
{"jnrrd": "0004"}
{"type": "float"}
{"dimension": 3}
{"sizes": [256, 256, 100]}
{"endian": "little"}
{"encoding": "raw"}
{"space": "right_anterior_superior"}
{"space_directions": [[0.5, 0.0, 0.0], [0.0, 0.5, 0.0], [0.0, 0.0, 1.2]]}
{"space_origin": [0.0, 0.0, 0.0]}
{"units": ["mm", "mm", "mm"]}
{"kinds": ["space", "space", "space"]}
{"labels": ["x", "y", "z"]}
{"content": "T1-weighted MRI"}
{"extensions": {"medical": "https://example.org/jnrrd/medical/v1"}}
{"medical:modality": "MRI"}
{"medical:sequence_type": "T1_weighted"}
{"medical:contrast_agent": false}

[BINARY DATA FOLLOWS]
```

## 16. Reference Implementations

Code for reading and writing JNRRD files in various languages:

### 16.1 Python Example Reader

```python
import json
import numpy as np

def read_jnrrd(filename):
    header = {}
    binary_start_pos = 0
    
    with open(filename, 'rb') as f:
        while True:
            line_start = f.tell()
            line = f.readline().strip()
            
            if not line:  # Empty line
                binary_start_pos = f.tell()
                break
            
            try:
                field = json.loads(line)
                header.update(field)
            except json.JSONDecodeError:
                binary_start_pos = line_start
                f.seek(binary_start_pos)
                break
        
        # Map types to NumPy types
        type_map = {
            # Integer types
            'int8': np.int8,
            'uint8': np.uint8,
            'int16': np.int16,
            'uint16': np.uint16,
            'int32': np.int32,
            'uint32': np.uint32,
            'int64': np.int64,
            'uint64': np.uint64,
            
            # Floating point types
            'float16': np.float16,
            'bfloat16': np.float16,  # NumPy doesn't directly support bfloat16; using float16 as fallback
            'float32': np.float32,
            'float64': np.float64,
            
            # Complex types
            'complex64': np.complex64,
            'complex128': np.complex128
        }
        
        data_type = header.get('type')
        numpy_dtype = type_map.get(data_type)
        
        if not numpy_dtype:
            raise ValueError(f"Unsupported data type: {data_type}")
        
        # Handle endianness
        endian = header.get('endian')
        if endian:
            if endian == 'little':
                numpy_dtype = numpy_dtype.newbyteorder('<')
            elif endian == 'big':
                numpy_dtype = numpy_dtype.newbyteorder('>')
        
        # Read binary data
        encoding = header.get('encoding', 'raw')
        
        if encoding == 'raw':
            binary_data = f.read()
        elif encoding == 'gzip':
            import gzip
            binary_data = gzip.decompress(f.read())
        elif encoding == 'bzip2':
            import bz2
            binary_data = bz2.decompress(f.read())
        elif encoding == 'zstd':
            import zstandard as zstd
            dctx = zstd.ZstdDecompressor()
            binary_data = dctx.decompress(f.read())
        elif encoding == 'lz4':
            import lz4.frame
            binary_data = lz4.frame.decompress(f.read())
        else:
            raise ValueError(f"Unsupported encoding: {encoding}")
        
        # Create array from binary data
        array_data = np.frombuffer(binary_data, dtype=numpy_dtype)
        
        # Reshape if dimensions are provided
        sizes = header.get('sizes', [])
        if sizes:
            array_data = array_data.reshape(sizes)
        
        return header, array_data
```

### 16.2 Python Example Writer

```python
import json
import numpy as np

def write_jnrrd(filename, header, array_data):
    with open(filename, 'wb') as f:
        # Ensure required fields
        if 'jnrrd' not in header:
            header['jnrrd'] = '0004'
        
        if 'dimension' not in header:
            header['dimension'] = len(array_data.shape)
        
        if 'sizes' not in header:
            header['sizes'] = list(array_data.shape)
        
        if 'type' not in header:
            # Map NumPy types to JNRRD types
            type_map = {
                np.dtype('int8'): 'int8',
                np.dtype('uint8'): 'uint8',
                np.dtype('int16'): 'int16',
                np.dtype('uint16'): 'uint16',
                np.dtype('int32'): 'int32',
                np.dtype('uint32'): 'uint32',
                np.dtype('int64'): 'int64',
                np.dtype('uint64'): 'uint64',
                np.dtype('float16'): 'float16',
                np.dtype('float32'): 'float32',
                np.dtype('float64'): 'float64',
                np.dtype('complex64'): 'complex64',
                np.dtype('complex128'): 'complex128'
            }
            header['type'] = type_map.get(array_data.dtype, 'float32')
        
        if 'encoding' not in header:
            header['encoding'] = 'raw'
        
        # Write each header field as a separate JSON line
        for key, value in header.items():
            line = json.dumps({key: value}).encode('utf-8') + b'\n'
            f.write(line)
        
        # Add a blank line for readability
        f.write(b'\n')
        
        # Handle encoding
        encoding = header.get('encoding', 'raw')
        binary_data = array_data.tobytes()
        
        if encoding == 'raw':
            f.write(binary_data)
        elif encoding == 'gzip':
            import gzip
            f.write(gzip.compress(binary_data))
        elif encoding == 'bzip2':
            import bz2
            f.write(bz2.compress(binary_data))
        elif encoding == 'zstd':
            import zstandard as zstd
            cctx = zstd.ZstdCompressor()
            f.write(cctx.compress(binary_data))
        elif encoding == 'lz4':
            import lz4.frame
            f.write(lz4.frame.compress(binary_data))
        else:
            raise ValueError(f"Unsupported encoding: {encoding}")
```

## 17. References

1. Original NRRD Format Specification
2. JSON RFC 8259 (https://tools.ietf.org/html/rfc8259)
3. Newline-Delimited JSON (http://ndjson.org/)

## 18. License

This specification is made available under the CC0 1.0 Universal (CC0 1.0) Public Domain Dedication.