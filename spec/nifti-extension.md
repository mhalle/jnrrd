# JNRRD NIfTI Extension Specification

Version: 1.0.0

## 1. Introduction

The JNRRD NIfTI Extension defines a standardized approach for incorporating NIfTI (Neuroimaging Informatics Technology Initiative) metadata and capabilities into JNRRD files. This extension enables JNRRD to maintain compatibility with neuroimaging workflows while benefiting from JNRRD's JSON-based structure and binary data handling.

NIfTI is a widely adopted format in neuroimaging that provides standardized fields for describing brain imaging data, including spatial transformations, intent codes, and other neuroimaging-specific metadata. This extension bridges the gap between JNRRD and the neuroimaging community.

## 2. Extension Declaration

To use this extension, a JNRRD file must include the following extension declaration:

```json
{"extensions": {"nifti": "https://jnrrd.org/extensions/nifti/v1.0.0"}}
```

## 3. Core Concepts

### 3.1 Coordinate Systems

NIfTI defines specific coordinate systems and transformations that are crucial for neuroimaging data:

- **RAS**: Right-Anterior-Superior coordinate system (also called "scanner-anatomical")
- **Qform**: Quaternion-based transformation defining the orientation of the volume in scanner space
- **Sform**: General affine transformation matrix for more complex alignments

### 3.2 Data Intent

NIfTI uses intent codes to specify how the data should be interpreted:

- **Intent Code**: Numeric identifier for data type (e.g., time series, correlation, p-value)
- **Intent Name**: Text description of the data intent
- **Intent Parameters**: Additional parameters specific to certain intent codes

### 3.3 Units and Scaling

NIfTI provides mechanisms for specifying physical units and scaling:

- **XYZ Units**: Units for spatial dimensions
- **Time Units**: Units for temporal dimensions
- **Scaling**: Slope and intercept for data value scaling

## 4. NIfTI Extension Fields

### 4.1 Basic Information

```json
{"nifti:version": 1}
```

| Field | Type | Description |
|-------|------|-------------|
| `version` | integer | NIfTI version: 1 for NIfTI-1, 2 for NIfTI-2 |

### 4.2 Coordinate Transformations

```json
{"nifti:qform_code": 2}
{"nifti:sform_code": 4}
{"nifti:qform_parameters": {
  "quaternion": [0.0, 0.0, 0.0, 1.0],
  "offset": [0.0, 0.0, 0.0]
}}
{"nifti:sform_matrix": [
  [1.0, 0.0, 0.0, -90.0],
  [0.0, 1.0, 0.0, -126.0],
  [0.0, 0.0, 1.0, -72.0],
  [0.0, 0.0, 0.0, 1.0]
]}
```

| Field | Type | Description |
|-------|------|-------------|
| `qform_code` | integer | Method used to generate quaternion transform (0=unknown, 1=scanner, 2=aligned, 3=talairach, 4=mni) |
| `sform_code` | integer | Method used to generate affine transform (0=unknown, 1=scanner, 2=aligned, 3=talairach, 4=mni) |
| `qform_parameters` | object | Quaternion-based transform parameters |
| `sform_matrix` | array | 4×4 affine transformation matrix |

The `qform_parameters` object contains:
- `quaternion`: [a, b, c, d] quaternion parameters
- `offset`: [x, y, z] offset parameters in mm

The `sform_matrix` is a 4×4 matrix in row-major order that maps voxel indices (i,j,k,1) to coordinates in the specified coordinate system.

### 4.3 Data Intent

```json
{"nifti:intent_code": 3}
{"nifti:intent_name": "Correlation"}
{"nifti:intent_p1": 0.05}
{"nifti:intent_p2": 0.01}
{"nifti:intent_p3": 0.0}
```

| Field | Type | Description |
|-------|------|-------------|
| `intent_code` | integer | Code identifying how the data should be interpreted (e.g., 2=correlation, 3=t-test, etc.) |
| `intent_name` | string | Name describing the intent |
| `intent_p1` | number | 1st intent-specific parameter |
| `intent_p2` | number | 2nd intent-specific parameter |
| `intent_p3` | number | 3rd intent-specific parameter |

Common intent codes include:
- 0: Unknown/unspecified
- 2: Correlation coefficient
- 3: T-statistic
- 4: F-statistic
- 5: Z-score
- 6: Chi-squared distribution
- 1001: Estimate
- 1002: Label
- 1003: Neuroname
- 1004: General matrix
- 1005: Symmetric matrix
- 1006: Displacement vector
- 1007: Vector
- 1008: Pointset
- 1009: Triangle
- 1010: Quaternion
- 1011: Dimensionless

### 4.4 Units and Scaling

```json
{"nifti:units": {
  "xyz": 2,
  "time": 8
}}
{"nifti:scaling": {
  "slope": 1.0,
  "intercept": 0.0
}}
```

| Field | Type | Description |
|-------|------|-------------|
| `units.xyz` | integer | Spatial units code (1=meters, 2=millimeters, 3=micrometers) |
| `units.time` | integer | Temporal units code (8=seconds, 16=milliseconds, 24=microseconds, 32=hertz, 40=ppm, 48=rad/s) |
| `scaling.slope` | number | Scaling slope (multiply stored values by this) |
| `scaling.intercept` | number | Scaling intercept (add this after multiplying by slope) |

### 4.5 Slice Timing Information

```json
{"nifti:slice": {
  "code": 1,
  "start": 0,
  "end": 35,
  "duration": 0.05
}}
```

| Field | Type | Description |
|-------|------|-------------|
| `slice.code` | integer | Slice timing pattern (0=unknown, 1=sequential_increasing, 2=sequential_decreasing, 3=alternating_increasing_1, 4=alternating_decreasing_1, 5=alternating_increasing_2, 6=alternating_decreasing_2) |
| `slice.start` | integer | Index of first acquired slice |
| `slice.end` | integer | Index of last acquired slice |
| `slice.duration` | number | Time for 1 slice in temporal units |

### 4.6 Display and Calibration

```json
{"nifti:calibration": {
  "min": 0.0,
  "max": 255.0
}}
```

| Field | Type | Description |
|-------|------|-------------|
| `calibration.min` | number | Display range minimum value |
| `calibration.max` | number | Display range maximum value |

### 4.7 Description and Metadata

```json
{"nifti:description": "resting state fMRI data with physiological monitoring"}
{"nifti:aux_file": "subject_info.txt"}
{"nifti:database_name": "OpenfMRI"}
{"nifti:generated_by": "SPM12"}
{"nifti:session_error": 0.0}
{"nifti:regular": 1}
```

| Field | Type | Description |
|-------|------|-------------|
| `description` | string | Text description of the data (max 80 chars in NIfTI-1) |
| `aux_file` | string | Auxiliary filename (max 24 chars in NIfTI-1) |
| `database_name` | string | Name of database that contains this image |
| `generated_by` | string | Name of program that generated this file |
| `session_error` | number | Estimate of session error |
| `regular` | integer | 1 if all slices have same time pattern |

### 4.8 NIfTI-2 Specific Extensions

For NIfTI-2 compatibility, additional fields are available:

```json
{"nifti:esize": 0}
{"nifti:datatype": 16}
{"nifti:extension_sequence": [
  {
    "code": 4,
    "content": "eyJzb21ldGhpbmciOiJ2YWx1ZSJ9"
  }
]}
```

| Field | Type | Description |
|-------|------|-------------|
| `esize` | integer | Number of bytes of extended header data |
| `datatype` | integer | NIfTI datatype code (e.g., 2=uint8, 4=int16, 16=float32) |
| `extension_sequence` | array | Array of extension objects with code and base64-encoded content |

### 4.9 Integration with JNRRD Space

NIfTI coordinate systems can be explicitly linked to JNRRD space fields:

```json
{"nifti:space_mapping": {
  "jnrrd_space": "right_anterior_superior",
  "nifti_orientation": "RAS"
}}
```

This field defines how the JNRRD coordinate system maps to NIfTI coordinates.

## 5. Coordinate Systems Handling

NIfTI provides two ways to specify the orientation of the volume in space:

1. **Quaternion (qform)**: A rotation based on quaternion parameters plus an offset
2. **Affine matrix (sform)**: A full 4×4 affine transformation matrix

JNRRD implementations should use these rules for interpretation:

1. If `sform_code` > 0, use the sform transformation
2. Else if `qform_code` > 0, use the qform transformation
3. Otherwise, fall back to JNRRD's native space information

The codes indicate the coordinate system referenced:
- 0: Unknown
- 1: Scanner coordinates
- 2: Aligned to some other scan
- 3: Talairach-Tournoux Atlas
- 4: MNI 152 Atlas

## 5. Data Consistency Requirements

When using the NIfTI extension with JNRRD, the file writer must ensure consistency between the core JNRRD fields and their corresponding NIfTI extension fields. Specifically:

1. **Dimensions and Sizes**: The JNRRD `dimension` and `sizes` fields must match the dimensionality implied by the NIfTI fields.

2. **Coordinate Systems**: The JNRRD `space`, `space_directions`, and `space_origin` fields must be consistent with the NIfTI transformation matrices (`qform_parameters` and `sform_matrix`).

3. **Data Type**: The JNRRD `type` field should correspond to the appropriate NIfTI data type.

4. **Scaling**: Any scaling applied to the data must be consistently represented in both JNRRD and NIfTI metadata.

It is the file writer's responsibility to maintain this consistency. Readers may use either the JNRRD core fields or the NIfTI extension fields for interpretation, but should expect them to represent the same underlying data structure.

## 6. Examples

### 6.1 Minimal Functional MRI Example

```
{"jnrrd": "0004"}
{"type": "float32"}
{"dimension": 4}
{"sizes": [64, 64, 32, 180]}
{"endian": "little"}
{"encoding": "gzip"}
{"space": "right_anterior_superior"}
{"space_directions": [
  [3.0, 0.0, 0.0],
  [0.0, 3.0, 0.0],
  [0.0, 0.0, 3.0],
  [0.0, 0.0, 0.0, 2.0]  // Fourth dimension is time, 2.0s per volume
]}
{"space_origin": [-90.0, -126.0, -72.0]}
{"extensions": {"nifti": "https://jnrrd.org/extensions/nifti/v1.0.0"}}
{"nifti:version": 1}
{"nifti:intent_code": 2002}  // Time series
{"nifti:intent_name": "Time Series"}
{"nifti:sform_code": 4}  // MNI space
{"nifti:sform_matrix": [
  [3.0, 0.0, 0.0, -90.0],
  [0.0, 3.0, 0.0, -126.0],
  [0.0, 0.0, 3.0, -72.0],
  [0.0, 0.0, 0.0, 1.0]
]}
{"nifti:units": {"xyz": 2, "time": 8}}  // mm and seconds
{"nifti:slice": {"code": 1, "duration": 2.0}}
{"nifti:description": "resting state fMRI"}

[BINARY DATA FOLLOWS]
```

### 6.2 Complete Structural MRI Example with Transformation

```
{"jnrrd": "0004"}
{"type": "int16"}
{"dimension": 3}
{"sizes": [256, 256, 160]}
{"endian": "little"}
{"encoding": "raw"}
{"space": "right_anterior_superior"}
{"space_directions": [
  [1.0, 0.0, 0.0],
  [0.0, 1.0, 0.0],
  [0.0, 0.0, 1.0]
]}
{"space_origin": [-90.0, -126.0, -72.0]}
{"extensions": {"nifti": "https://jnrrd.org/extensions/nifti/v1.0.0"}}
{"nifti:version": 1}
{"nifti:intent_code": 0}  // None
{"nifti:qform_code": 1}  // Scanner coordinates
{"nifti:sform_code": 4}  // MNI space
{"nifti:qform_parameters": {
  "quaternion": [0.0, 0.0, 0.0, 1.0],  // Identity rotation
  "offset": [-90.0, -126.0, -72.0]     // Matches space_origin
}}
{"nifti:sform_matrix": [
  [1.0, 0.0, 0.0, -90.0],   // Matches space_directions[0] + space_origin[0]
  [0.0, 1.0, 0.0, -126.0],  // Matches space_directions[1] + space_origin[1]
  [0.0, 0.0, 1.0, -72.0],   // Matches space_directions[2] + space_origin[2]
  [0.0, 0.0, 0.0, 1.0]
]}
{"nifti:units": {"xyz": 2}}  // mm
{"nifti:scaling": {"slope": 1.0, "intercept": 0.0}}  // No scaling applied
{"nifti:calibration": {"min": 0.0, "max": 4095.0}}
{"nifti:description": "T1-weighted structural MRI"}
{"nifti:aux_file": "subject_demographics.json"}
{"nifti:generated_by": "dcm2niix v1.0.20190902"}

[BINARY DATA FOLLOWS]
```

### 6.3 Same Example Using Flattened Hierarchy Notation

```
{"jnrrd": "0004"}
{"type": "int16"}
{"dimension": 3}
{"sizes": [256, 256, 160]}
{"endian": "little"}
{"encoding": "raw"}
{"space": "right_anterior_superior"}
{"space_directions": [
  [1.0, 0.0, 0.0],
  [0.0, 1.0, 0.0],
  [0.0, 0.0, 1.0]
]}
{"space_origin": [-90.0, -126.0, -72.0]}
{"extensions": {"nifti": "https://jnrrd.org/extensions/nifti/v1.0.0"}}
{"nifti:version": 1}
{"nifti:intent_code": 0}
{"nifti:qform_code": 1}
{"nifti:sform_code": 4}
{"nifti:qform_parameters.quaternion[0]": 0.0}
{"nifti:qform_parameters.quaternion[1]": 0.0}
{"nifti:qform_parameters.quaternion[2]": 0.0}
{"nifti:qform_parameters.quaternion[3]": 1.0}
{"nifti:qform_parameters.offset[0]": -90.0}
{"nifti:qform_parameters.offset[1]": -126.0}
{"nifti:qform_parameters.offset[2]": -72.0}
{"nifti:sform_matrix[0][0]": 1.0}
{"nifti:sform_matrix[0][1]": 0.0}
{"nifti:sform_matrix[0][2]": 0.0}
{"nifti:sform_matrix[0][3]": -90.0}
{"nifti:sform_matrix[1][0]": 0.0}
{"nifti:sform_matrix[1][1]": 1.0}
{"nifti:sform_matrix[1][2]": 0.0}
{"nifti:sform_matrix[1][3]": -126.0}
{"nifti:sform_matrix[2][0]": 0.0}
{"nifti:sform_matrix[2][1]": 0.0}
{"nifti:sform_matrix[2][2]": 1.0}
{"nifti:sform_matrix[2][3]": -72.0}
{"nifti:sform_matrix[3][0]": 0.0}
{"nifti:sform_matrix[3][1]": 0.0}
{"nifti:sform_matrix[3][2]": 0.0}
{"nifti:sform_matrix[3][3]": 1.0}
{"nifti:units.xyz": 2}
{"nifti:scaling.slope": 1.0}
{"nifti:scaling.intercept": 0.0}
{"nifti:calibration.min": 0.0}
{"nifti:calibration.max": 4095.0}
{"nifti:description": "T1-weighted structural MRI"}
{"nifti:aux_file": "subject_demographics.json"}
{"nifti:generated_by": "dcm2niix v1.0.20190902"}

[BINARY DATA FOLLOWS]
```

## 7. Converting Between NIfTI and JNRRD

### 7.1 Converting NIfTI to JNRRD+NIfTI Extension

When converting from a NIfTI file to JNRRD with the NIfTI extension:

1. Extract header information from the NIfTI file
2. Map NIfTI header fields to corresponding JNRRD NIfTI extension fields
3. Set appropriate JNRRD core fields (type, dimension, sizes, space, space_directions)
4. Convert coordinate transforms (qform, sform) to JNRRD space representation
5. Extract the binary data and store with appropriate encoding

### 7.2 Converting JNRRD+NIfTI Extension to NIfTI

When converting from JNRRD with NIfTI extension to a NIfTI file:

1. Create a new NIfTI header
2. Map JNRRD NIfTI extension fields to NIfTI header fields
3. Calculate appropriate dimensional information
4. Set transformation matrices from JNRRD space information and NIfTI extension transforms
5. Copy the binary data, applying any needed byte order transformations

## 8. Implementation Notes

1. **Quaternion vs. Affine**: The quaternion (qform) representation is more restricted but numerically stable, while the affine (sform) representation allows any linear transformation
2. **Byte Order**: NIfTI is typically stored in the byte order of the machine that wrote it, so endianness must be preserved
3. **Extensions**: NIfTI extensions (usually stored after the header) can be represented in the `extension_sequence` field
4. **Scaling**: The scaling factors (slope and intercept) should be applied to stored values to get real-world values
5. **Time Series**: For 4D data (time series), the 4th dimension in `space_directions` should be [0,0,0,dt] where dt is the time step

## 9. Compatibility with JNRRD Extensions

The NIfTI extension can be used together with other JNRRD extensions:

- **JNRRD Tiling Extension**: Enables chunked access to large neuroimaging datasets
- **DICOM Extension**: Can provide additional medical context alongside NIfTI metadata
- **OME Extension**: Useful for multi-modal neuroimaging with microscopy components

## 10. JSON Schema

The following JSON Schema can be used to validate the NIfTI extension fields:

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "JNRRD NIfTI Extension Schema",
  "description": "Schema for validating JNRRD NIfTI extension fields",
  "type": "object",
  "properties": {
    "version": {
      "type": "integer",
      "description": "NIfTI version (1 or 2) (stored as 'nifti:version' in JNRRD files)",
      "enum": [1, 2]
    },
    "intent_code": {
      "type": "integer",
      "description": "Code for data intent (stored as 'nifti:intent_code' in JNRRD files)"
    },
    "intent_name": {
      "type": "string",
      "description": "Name for data intent (stored as 'nifti:intent_name' in JNRRD files)",
      "maxLength": 16
    },
    "intent_p1": {
      "type": "number",
      "description": "First intent parameter (stored as 'nifti:intent_p1' in JNRRD files)"
    },
    "intent_p2": {
      "type": "number",
      "description": "Second intent parameter (stored as 'nifti:intent_p2' in JNRRD files)"
    },
    "intent_p3": {
      "type": "number",
      "description": "Third intent parameter (stored as 'nifti:intent_p3' in JNRRD files)"
    },
    "qform_code": {
      "type": "integer",
      "description": "Code for quaternion transform (stored as 'nifti:qform_code' in JNRRD files)",
      "minimum": 0,
      "maximum": 4
    },
    "sform_code": {
      "type": "integer",
      "description": "Code for affine transform (stored as 'nifti:sform_code' in JNRRD files)",
      "minimum": 0,
      "maximum": 4
    },
    "qform_parameters": {
      "type": "object",
      "description": "Quaternion transform parameters (stored as 'nifti:qform_parameters' in JNRRD files)",
      "properties": {
        "quaternion": {
          "type": "array",
          "description": "Quaternion parameters [a,b,c,d]",
          "items": {
            "type": "number"
          },
          "minItems": 4,
          "maxItems": 4
        },
        "offset": {
          "type": "array",
          "description": "Offset parameters [x,y,z]",
          "items": {
            "type": "number"
          },
          "minItems": 3,
          "maxItems": 3
        }
      },
      "required": ["quaternion", "offset"]
    },
    "sform_matrix": {
      "type": "array",
      "description": "4×4 affine transformation matrix (stored as 'nifti:sform_matrix' in JNRRD files)",
      "items": {
        "type": "array",
        "items": {
          "type": "number"
        },
        "minItems": 4,
        "maxItems": 4
      },
      "minItems": 4,
      "maxItems": 4
    },
    "units": {
      "type": "object",
      "description": "Units for spatial and temporal dimensions (stored as 'nifti:units' in JNRRD files)",
      "properties": {
        "xyz": {
          "type": "integer",
          "description": "Units for spatial dimensions",
          "enum": [0, 1, 2, 3]
        },
        "time": {
          "type": "integer",
          "description": "Units for temporal dimension",
          "enum": [0, 8, 16, 24, 32, 40, 48]
        }
      }
    },
    "scaling": {
      "type": "object",
      "description": "Scaling factors for data values (stored as 'nifti:scaling' in JNRRD files)",
      "properties": {
        "slope": {
          "type": "number",
          "description": "Scaling slope"
        },
        "intercept": {
          "type": "number",
          "description": "Scaling intercept"
        }
      },
      "required": ["slope", "intercept"]
    },
    "slice": {
      "type": "object",
      "description": "Slice timing information (stored as 'nifti:slice' in JNRRD files)",
      "properties": {
        "code": {
          "type": "integer",
          "description": "Slice timing pattern code",
          "minimum": 0,
          "maximum": 6
        },
        "start": {
          "type": "integer",
          "description": "First acquired slice"
        },
        "end": {
          "type": "integer",
          "description": "Last acquired slice"
        },
        "duration": {
          "type": "number",
          "description": "Time for one slice acquisition"
        }
      },
      "required": ["code"]
    },
    "calibration": {
      "type": "object",
      "description": "Display range calibration (stored as 'nifti:calibration' in JNRRD files)",
      "properties": {
        "min": {
          "type": "number",
          "description": "Display range minimum"
        },
        "max": {
          "type": "number",
          "description": "Display range maximum"
        }
      },
      "required": ["min", "max"]
    },
    "description": {
      "type": "string",
      "description": "Text description of the data (stored as 'nifti:description' in JNRRD files)",
      "maxLength": 80
    },
    "aux_file": {
      "type": "string",
      "description": "Auxiliary filename (stored as 'nifti:aux_file' in JNRRD files)",
      "maxLength": 24
    },
    "database_name": {
      "type": "string",
      "description": "Name of database containing this image (stored as 'nifti:database_name' in JNRRD files)"
    },
    "generated_by": {
      "type": "string",
      "description": "Program that generated this file (stored as 'nifti:generated_by' in JNRRD files)"
    },
    "session_error": {
      "type": "number",
      "description": "Estimate of session error (stored as 'nifti:session_error' in JNRRD files)"
    },
    "regular": {
      "type": "integer",
      "description": "1 if all slices have same time pattern (stored as 'nifti:regular' in JNRRD files)",
      "enum": [0, 1]
    },
    "dim_info": {
      "type": "integer",
      "description": "Encoding of the frequency, phase, and slice dimensions (stored as 'nifti:dim_info' in JNRRD files)"
    },
    "extension_sequence": {
      "type": "array",
      "description": "NIfTI extensions (stored as 'nifti:extension_sequence' in JNRRD files)",
      "items": {
        "type": "object",
        "properties": {
          "code": {
            "type": "integer",
            "description": "Extension code"
          },
          "content": {
            "type": "string",
            "description": "Base64-encoded extension content"
          }
        },
        "required": ["code", "content"]
      }
    },
    "space_mapping": {
      "type": "object",
      "description": "Mapping between JNRRD and NIfTI spaces (stored as 'nifti:space_mapping' in JNRRD files)",
      "properties": {
        "jnrrd_space": {
          "type": "string",
          "description": "JNRRD space identifier"
        },
        "nifti_orientation": {
          "type": "string",
          "description": "NIfTI orientation identifier",
          "enum": ["RAS", "LAS", "LPS", "RPS", "RAI", "LAI", "LPI", "RPI"]
        }
      },
      "required": ["jnrrd_space", "nifti_orientation"]
    }
  }
}
```