# NRRD Extension Mechanism Specification
Version: 1.0.0

## 1. Introduction

This specification defines an extension mechanism for the NRRD (Nearly Raw Raster Data) format that adds the flexibility of JSON-structured metadata while maintaining complete backward compatibility with existing NRRD parsers. 

The NRRD extension mechanism introduces new keys in NRRD files that are prefixed by a short name representing a NRRD extension. An extension is a group of related fields defined by a self-contained standard that further describes information about the image. For example, a DICOM extension might include patient information, while a metadata extension might include attribution and licensing details.

These extension-specific NRRD keys and their values are translated into standard JSON by NRRD extension parsers. The NRRD keys represent either JSON keys or JSONPath key hierarchies, while NRRD key values must be valid JSON. Because NRRD is line-oriented, the JSON value of any key must fit on one line. For complex data structures, this limitation might impact legibility. That's why a hierarchical key syntax can be used interchangeably.

For example, instead of:
```
meta:creator:={"type":"Organization","name":"Medical Research Lab","url":"https://example.org/lab"}
```

You can use:
```
meta:creator.type:="Organization"
meta:creator.name:="Medical Research Lab"
meta:creator.url:="https://example.org/lab"
```

The design leverages the key-value pair syntax (introduced in NRRD0002) but adds conventions for structured, hierarchical metadata organized by extension namespaces. This approach is inspired by the JSON-based NRRD (JNRRD) format but implemented in a way that requires no changes to the core NRRD format.

### 1.1 Design Goals

- Maintain complete backward compatibility with existing NRRD parsers
- Enable structured, hierarchical metadata organization
- Support domain-specific extensions (medical imaging, microscopy, etc.)
- Allow validation of extension data through JSON Schema
- Provide a migration path to JNRRD without breaking compatibility

## 2. Extension Mechanism Overview

### 2.1 Extension Declaration

Extensions are declared using a special key-value pair with the key `extensions`. The value is a JSON object mapping namespace prefixes to URIs:

```
extensions:={"meta":"https://jnrrd.org/extensions/metadata/v1.0.0","nifti":"https://jnrrd.org/extensions/nifti/v1.0.0"}
```

This declaration defines two extensions:
1. `meta` - Pointing to the URI `https://jnrrd.org/extensions/metadata/v1.0.0`
2. `nifti` - Pointing to the URI `https://jnrrd.org/extensions/nifti/v1.0.0`

The URIs should point to documentation about the extension, preferably a machine-readable schema in JSON Schema format.

The extensions key can also be expressed using the hierarchical key syntax for individual extensions:

```
extensions.meta:="https://jnrrd.org/extensions/metadata/v1.0.0"
extensions.nifti:="https://jnrrd.org/extensions/nifti/v1.0.0"
```

This approach may be more maintainable when working with many extensions, as it's easier to add, remove, or comment out individual extension declarations.

### 2.2 Extension Keys

Extension data is stored using key-value pairs with keys following the pattern:

```
[prefix]:[path]:=[value]
```

Where:
- `[prefix]` is a namespace prefix declared in the `extensions` object
- `[path]` is the path within the extension namespace
- `[value]` is a JSON-compatible value (string, number, boolean, object, or array)

Examples:
```
meta:name:="T1-weighted Brain MRI Dataset"
meta:license:="https://creativecommons.org/licenses/by/4.0/"
```

### 2.3 CURIE Parallel

The extension key format is parallel to Compact URI Expressions (CURIEs), where:
- The prefix acts as the CURIE prefix
- The path acts as the CURIE reference

This parallel to CURIEs helps establish a clear semantics for the extension key format and ties it to established web standards for identifier compaction.

## 3. Hierarchical Data Representation

Extension data can be represented in two equivalent ways:

### 3.1 Nested Objects

Complex data can be represented as nested JSON objects:

```
meta:creator:={"type":"Organization","name":"Medical Research Lab","url":"https://example.org/lab"}
```

### 3.2 Flattened Hierarchy

The same data can be represented using dot notation:

```
meta:creator.type:="Organization"
meta:creator.name:="Medical Research Lab"
meta:creator.url:="https://example.org/lab"
```

These two representations are semantically equivalent. In the flattened hierarchy approach:
- The colon (`:`) separates the extension namespace from the root object
- The dot (`.`) is used to navigate through object hierarchies
- Numeric indices in brackets (`[n]`) are used to access array elements

This approach offers several benefits:
- Keeps individual key-value lines shorter and more readable
- Allows partial updates to hierarchical data
- Enables direct access to deeply nested values
- Follows standard JSONPath conventions

### 3.3 Array Representation

Arrays can be represented in two ways:

1. **Direct JSON array representation**:
```
meta:keywords:=["neuroimaging","T1-weighted","MRI","brain"]
```

2. **Indexed representation**:
```
meta:authors[0].name:="Author1"
meta:authors[1].name:="Author2"
```

### 3.4 Priority Rules for Hierarchical Fields

When a field appears in both formats, the flattened hierarchy (more specific path) takes precedence:

```
meta:creator:={"type":"Organization","name":"Original Lab"}
meta:creator.name:="Updated Lab Name"
```

The effective value would be:
```json
{
  "type": "Organization",
  "name": "Updated Lab Name"
}
```

Array elements can also be accessed and modified using numeric indices:

```
meta:authors:=[{"name":"Author1"},{"name":"Author2"}]
meta:authors[1].name:="Updated Author2"
meta:authors[2].name:="Author3"
```

The effective value would be:
```json
[
  {"name":"Author1"},
  {"name":"Updated Author2"},
  {"name":"Author3"}
]
```

## 4. Extension Data Consolidation

### 4.1 Namespace Stripping

When consolidating extension data, parsers should strip the extension prefix from field names before combining them. For example, the fields:

```
meta:name:="T1-weighted Brain MRI Dataset"
meta:description:="3D T1-weighted MRI brain scan of healthy adult subject"
```

Should be consolidated into an object:

```json
{
  "name": "T1-weighted Brain MRI Dataset",
  "description": "3D T1-weighted MRI brain scan of healthy adult subject"
}
```

This consolidated object is associated with the extension prefix "meta".

### 4.2 Hierarchical Reconstruction

Parsers should reconstruct hierarchical structures based on dot notation and array indices:

```
meta:creator.type:="Organization"
meta:creator.name:="Medical Research Lab"
meta:authors[0].name:="John Smith"
meta:authors[1].name:="Jane Doe"
```

Should be consolidated into:

```json
{
  "creator": {
    "type": "Organization",
    "name": "Medical Research Lab"
  },
  "authors": [
    {"name": "John Smith"},
    {"name": "Jane Doe"}
  ]
}
```

### 4.3 Final Structure

The final structure after parsing all extensions should be a dictionary with the following structure:

```json
{
  "extensions": {
    "meta": "https://jnrrd.org/extensions/metadata/v1.0.0",
    "nifti": "https://jnrrd.org/extensions/nifti/v1.0.0"
  },
  "extension_data": {
    "meta": {
      // metadata extension fields
    },
    "nifti": {
      // nifti extension fields
    }
  }
}
```

## 5. JSON Schema Validation

### 5.1 Extension Validation with JSON Schema

Extensions can and should use standard JSON Schema for validation. When a parser encounters extension fields, it can collect all fields for a given extension, strip the extension prefix, and validate the resulting object against the schema provided at the extension's URL.

### 5.2 Schema Publication

Extension authors should provide a JSON Schema at the extension URL. This schema defines the structure, required fields, and constraints for all fields in that extension. The schema should:

1. Use JSON Schema draft-07 or later
2. Include descriptions for all properties
3. Specify required fields
4. Define property types, formats, and constraints
5. Include examples where helpful
6. Define properties without extension prefixes (the prefix will be stripped before validation)

### 5.3 Validation Process

Parsers implement validation by:

1. Reading the extension declarations from the `extensions` field
2. Fetching the JSON Schema from each extension's URL
3. For each extension:
   - Collecting all fields with that extension's prefix
   - Removing the prefix from field names
   - Creating a consolidated object with all fields for that extension
   - Validating this consolidated object against the schema

## 6. Implementation Guidelines

### 6.1 Reading NRRD Files with Extensions

1. Parse the NRRD header normally, processing all standard NRRD fields
2. If an `extensions` key is found, parse its value as a JSON object mapping prefixes to URIs
3. For each line containing a key-value pair with a colon in the key:
   - Extract the prefix (part before the first colon)
   - Check if this prefix is defined in the extensions
   - If so, parse this as an extension field
   - Otherwise, treat it as a normal key-value pair
4. For each extension, consolidate its fields by stripping the prefix and reconstructing hierarchical structures

### 6.2 Writing NRRD Files with Extensions

1. Write all required NRRD fields first
2. If extensions are used, write the `extensions` key-value pair
3. For each extension field:
   - Prefix the field name with the extension namespace
   - Encode complex values as JSON
   - Choose between nested objects and flattened hierarchy based on complexity and readability

### 6.3 Handling Non-Extension Key-Value Pairs

Parsers should skip consolidation for key-value pairs that do not have extension prefixes. These are treated as standard NRRD key-value pairs.

If a key-value pair has a pattern that looks like an extension key (contains a colon) but the prefix is not declared in the `extensions` object, it should be treated as a regular key-value pair, not an extension field.

### 6.4 Whitespace Handling

Whitespace significance follows standard JSON parsing rules:

1. **Outside JSON values**: Any whitespace before or after the `:=` operator is not significant. Parsers should extract the JSON value by trimming whitespace from both sides.

2. **Within JSON values**: Whitespace handling follows the JSON specification (RFC 8259):
   - Whitespace outside string literals is not significant
   - Whitespace within string literals is significant and must be preserved
   - Whitespace between array/object elements is not significant

For example, these fields are all equivalent:
```
meta:name:="T1-weighted MRI"
meta:name:=  "T1-weighted MRI"  
meta:name:=       "T1-weighted MRI"
```

Similarly, these are equivalent according to JSON parsing rules:
```
meta:keywords:=["MRI", "T1", "neuroimaging"]
meta:keywords:=  [  "MRI"  ,  "T1"  ,  "neuroimaging"  ]  
```

But this is different because whitespace within the string literals is preserved:
```
meta:keywords:=["MRI", "T1  ", "neuroimaging"]
```

In this example, the extra spaces in "T1  " are part of the string value and are significant.

### 6.5 Backward Compatibility Considerations

Since this extension mechanism uses the existing key-value pair syntax of NRRD, older parsers will still be able to read NRRD files with extensions, but will not interpret the extensions correctly. They will simply see extension fields as additional key-value pairs.

## 7. Examples

### 7.1 Minimal Example

```
NRRD0004
# minimal example with extensions
type: float
dimension: 3
sizes: 256 256 120
space: right-anterior-superior
encoding: raw
extensions:={"meta":"https://jnrrd.org/extensions/metadata/v1.0.0"}
meta:name:="T1-weighted Brain MRI Dataset"
meta:creator:="Medical Research Lab"
meta:dateCreated:="2021-08-23"
```

### 7.2 Complex Example with Multiple Extensions

```
NRRD0004
# complex example with multiple extensions
type: float
dimension: 3
sizes: 256 256 120
space: right-anterior-superior
spacings: 0.5 0.5 0.7
encoding: raw
extensions:={"meta":"https://jnrrd.org/extensions/metadata/v1.0.0","dicom":"https://jnrrd.org/extensions/dicom/v1.0.0"}
meta:name:="T1-weighted Brain MRI Dataset"
meta:description:="3D T1-weighted MRI brain scan of healthy adult subject"
meta:license:="https://creativecommons.org/licenses/by/4.0/"
meta:creator:={"@type":"Organization","name":"Medical Research Lab","url":"https://example.org/lab"}
meta:dateCreated:="2021-08-23"
meta:keywords:=["neuroimaging","T1-weighted","MRI","brain"]
dicom:patient.id:="12345"
dicom:patient.name:="ANONYMOUS"
dicom:patient.birth_date:="19800101"
dicom:study.instance_uid:="1.2.840.113619.2.334.3.2831183778.864.1629754903.547"
dicom:study.date:="20210823"
dicom:study.time:="152143"
dicom:study.description:="BRAIN MRI"
```

### 7.3 Segmentation Example

```
NRRD0004
# segmentation example
type: uint8
dimension: 3
sizes: 256 256 120
space: right-anterior-superior
spacings: 0.5 0.5 0.7
encoding: raw
extensions:={"seg":"https://jnrrd.org/extensions/segmentation/v1.0.0"}
seg:master_representation:="Binary labelmap"
seg:segments[0].id:="segment_1"
seg:segments[0].label_value:=1
seg:segments[0].name:="ribs"
seg:segments[0].color:=[0.992,0.909,0.619]
seg:segments[0].terminology.category.coding_scheme:="SCT"
seg:segments[0].terminology.category.code_value:="123037004"
seg:segments[0].terminology.category.code_meaning:="Anatomical Structure"
seg:segments[1].id:="segment_2"
seg:segments[1].label_value:=2
seg:segments[1].name:="cervical_vertebrae"
seg:segments[1].color:=[1.0,1.0,0.811]
```

## 8. Implementation Guidelines

Below is pseudocode in Python-like syntax for parsing NRRD files with extensions:

```python
def parse_nrrd_extensions(header):
    # Initialize result structure
    result = {
        "extensions": {},
        "extension_data": {}
    }
    
    # Find all extension declarations
    extensions = {}
    
    # Look for main extensions declaration
    if "extensions" in header:
        try:
            # Parse the main extensions object
            extensions = parse_json(header["extensions"])
        except JSONParseError:
            # Skip if extensions declaration is invalid
            pass
    
    # Look for hierarchical extension declarations (extensions.prefix)
    for key, value in header.items():
        if key.startswith("extensions."):
            # Extract prefix from key (e.g., "extensions.meta" -> "meta")
            prefix = key[len("extensions."):]
            
            # Add to extensions dictionary
            extensions[prefix] = value
    
    # If no extensions were found, return empty result
    if not extensions:
        return result
        
    # Store parsed extensions
    result["extensions"] = extensions
    
    # Group fields by extension prefix
    extension_fields = {}
    for key, value in header.items():
        if ":" in key:
            prefix, path = split_at_first_colon(key)
            if prefix in extensions:
                # Store fields under their extension prefix
                if prefix not in extension_fields:
                    extension_fields[prefix] = {}
                extension_fields[prefix][path] = value
    
    # Process each extension's fields
    for prefix, fields in extension_fields.items():
        result["extension_data"][prefix] = reconstruct_hierarchy(fields)
    
    return result

def reconstruct_hierarchy(fields):
    result = {}
    
    for path, value in fields.items():
        # Try parsing value as JSON if appropriate
        try:
            if looks_like_json(value):
                value = parse_json(value)
        except:
            # Keep original value if parsing fails
            pass
        
        # Split path into components (handling dots and array indices)
        parts = parse_path(path)
        
        # Build nested structure following the path
        current = result
        for i, part in enumerate(parts[:-1]):
            # Create intermediate objects/arrays as needed
            if part not in current:
                # Determine if we need an array or object
                if is_array_index(parts[i+1]):
                    current[part] = []
                else:
                    current[part] = {}
            current = current[part]
        
        # Set value at final location
        current[parts[-1]] = value
    
    return result
```

This pseudocode outlines the key steps for:
1. Parsing the extensions declaration
2. Identifying and extracting extension fields
3. Reconstructing hierarchical structures from path-based keys
4. Handling JSON value parsing

## 9. Conclusion

This extension mechanism provides a way to add rich, structured metadata to NRRD files while maintaining full backward compatibility with existing NRRD parsers. It leverages the existing key-value pair syntax of NRRD but adds conventions for extension namespaces and hierarchical data representation.

By following the approach defined in this specification, NRRD files can incorporate domain-specific metadata in a standardized way, enabling better interoperability with various metadata ecosystems while preserving the simplicity and portability of the NRRD format.

## 10. References

1. Original NRRD Format Specification
2. JSON RFC 8259 (https://tools.ietf.org/html/rfc8259)
3. JSON Schema (https://json-schema.org/)
4. JNRRD Specification (https://github.com/jnrrd/spec)
5. CURIE Syntax 1.0 (https://www.w3.org/TR/curie/)

## 11. License

This specification is made available under the CC0 1.0 Universal (CC0 1.0) Public Domain Dedication.