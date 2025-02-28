# JNRRD Transforms Extension Specification

Version: 1.0.0

## 1. Introduction

The JNRRD Transforms Extension provides first-class support for coordinate systems and spatial transformations in JNRRD files. This extension facilitates working with multi-modal imaging, registration, and any scenario requiring coordinate transformations.

The extension was designed based on the [NG-FF coordinate systems and transformations specification](https://github.com/ome/ngff/pull/138), adapting its concepts to JNRRD's extension mechanism.

### 1.1 Motivation

Coordinate and spatial transformations are vital for scientific imaging to enable:

1. **Reproducibility and Consistency**: Standardized transformation storage ensures consistent application across different platforms and applications.
2. **Integration with Analysis Workflows**: First-class transformation support allows seamless integration with image analysis pipelines.
3. **Efficiency and Accuracy**: Storing transformations avoids re-sampling and reduces errors in data processing.
4. **Flexibility in Analysis**: Native transformation support allows researchers to apply, modify, or reverse transformations as needed.

## 2. Extension Declaration

This extension is declared in the `extensions` field of the JNRRD header:

```json
{"extensions": {"transform": "https://jnrrd.org/extensions/transforms/v1.0.0"}}
```

### 2.1 Relationship to NRRD Conventions

This extension adapts and expands upon several concepts from the original NRRD format:

#### 2.1.1 NRRD Space Directions and JNRRD Transforms

In traditional NRRD, spatial orientation is primarily specified through the `space directions` field, which encodes axis directions and spacings. In JNRRD, the transforms extension provides more expressivity through the affine and scale transformations:

```
# Original NRRD
space directions: (0.5,0,0) (0,0.5,0) (0,0,1.2)
space origin: (0,0,0)

# Equivalent JNRRD transform
{"transform:transformations": [{
  "type": "scale",
  "input": "array",
  "output": "physical",
  "scale": [0.5, 0.5, 1.2]
}]}
```

For more complex orientation matrices in NRRD that include origin information:

```
# Original NRRD
space directions: (0.5,0,0) (0,0.5,0) (0,0,1.2)
space origin: (10.0,20.0,5.0)

# Equivalent JNRRD transform
{"transform:transformations": [{
  "type": "affine",
  "input": "array",
  "output": "physical",
  "affine": [
    [0.5, 0.0, 0.0, 10.0],
    [0.0, 0.5, 0.0, 20.0],
    [0.0, 0.0, 1.2, 5.0]
  ]
}]}
```

#### 2.1.2 NRRD Kinds and JNRRD Axis Types

NRRD uses the `kinds` field to specify the semantic meaning of each axis. This extension harmonizes with that approach:

```
# Original NRRD
dimension: 3
sizes: 256 256 128
kinds: space space space

# Equivalent JNRRD transform
{"transform:coordinateSystems": [{
  "name": "physical",
  "axes": [
    {"name": "x", "type": "space", "unit": "millimeter"},
    {"name": "y", "type": "space", "unit": "millimeter"},
    {"name": "z", "type": "space", "unit": "millimeter"}
  ]
}]}
```

NRRD kind values map to JNRRD axis types as follows:
- `domain`, `space`, `time` → `space`, `time` accordingly
- `vector`, `scalar`, `complex` → Various specialized types in JNRRD
- `list` → Could be represented using `discrete: true`
- `2d-matrix`, `3d-matrix` → Could use specialized transforms

#### 2.1.3 NRRD Measurement Frame and JNRRD Transforms

NRRD's `measurement frame` field, used for tensor/vector data, can be represented in JNRRD using appropriate transformations, providing greater expressivity and clarity.

## 3. Coordinate Systems

### 3.1 Overview

A "coordinate system" is a collection of named axes with a unique identifier. Each coordinate system:

- MUST have a unique name
- MUST specify a set of axes
- MAY provide additional metadata

### 3.2 Syntax

Coordinate systems can be defined using the JNRRD extension mechanism:

```json
{"transform:coordinateSystems": [
  {
    "name": "physical",
    "axes": [
      {
        "name": "x",
        "type": "space",
        "unit": "millimeter",
        "discrete": false
      },
      {
        "name": "y",
        "type": "space", 
        "unit": "millimeter",
        "discrete": false
      },
      {
        "name": "z",
        "type": "space",
        "unit": "millimeter", 
        "discrete": false
      }
    ]
  },
  {
    "name": "acquisition",
    "axes": [
      {
        "name": "x",
        "type": "space",
        "unit": "micrometer",
        "discrete": false
      },
      {
        "name": "y",
        "type": "space",
        "unit": "micrometer",
        "discrete": false
      },
      {
        "name": "z",
        "type": "space",
        "unit": "micrometer",
        "discrete": false
      },
      {
        "name": "t",
        "type": "time",
        "unit": "second",
        "discrete": false
      }
    ]
  }
]}
```

Alternatively, using the JNRRD hierarchical extension format:

```json
{"transform:coordinateSystems[0].name": "physical"}
{"transform:coordinateSystems[0].axes[0].name": "x"}
{"transform:coordinateSystems[0].axes[0].type": "space"}
{"transform:coordinateSystems[0].axes[0].unit": "millimeter"}
{"transform:coordinateSystems[0].axes[0].discrete": false}
```

### 3.3 Default Array Coordinate System

Every JNRRD array has an implicit default coordinate system whose parameters need not be explicitly defined:
- Its name is derived from the data field description
- Its axes have `"type":"array"`, are unitless
- The ith axis has `"name":"dim_i"`

### 3.4 Axis Properties

Each axis in a coordinate system:
- MUST have a unique `name` within that coordinate system
- SHOULD have a `type` (one of "array", "space", "time", "channel", "coordinate", "displacement")
- MAY have a `unit` according to UDUNITS-2 (e.g., "millimeter", "second")
- MAY have a `discrete` property (boolean, true if the axis represents a discrete dimension)
- MAY have a `longName` for additional description

Recommended units:
- For "space" axes: 'angstrom', 'nanometer', 'micrometer', 'millimeter', 'centimeter', 'meter', etc.
- For "time" axes: 'microsecond', 'millisecond', 'second', 'minute', 'hour', 'day', etc.

### 3.5 Coordinate Convention

For continuous coordinate systems, the pixel/voxel center is the origin of the continuous coordinate system. A pixel/voxel centered at array position `(0,0)` has continuous coordinates `(0.0, 0.0)` when the transformation is the identity. The continuous rectangle of the pixel is given by the half-open interval `[-0.5, 0.5) x [-0.5, 0.5)`.

## 4. Coordinate Transformations

### 4.1 Overview

Coordinate transformations describe mappings between two coordinate systems. Each transformation:
- MUST specify a transformation `type`
- MUST include an `input` coordinate system name
- MUST include an `output` coordinate system name
- MUST include type-specific parameters
- MAY have a unique `name`

### 4.2 Basic Syntax

```json
{"transform:transformations": [
  {
    "name": "array_to_physical",
    "type": "scale",
    "input": "array",
    "output": "physical",
    "scale": [0.5, 0.5, 1.2]
  }
]}
```

Alternatively, using the JNRRD hierarchical extension format:

```json
{"transform:transformations[0].name": "array_to_physical"}
{"transform:transformations[0].type": "scale"}
{"transform:transformations[0].input": "array"}
{"transform:transformations[0].output": "physical"}
{"transform:transformations[0].scale": [0.5, 0.5, 1.2]}
```

### 4.3 Transformation Types

#### 4.3.1 identity

Maps input coordinates to output coordinates without modification. Input and output must have the same dimensionality.

```json
{
  "type": "identity",
  "input": "array",
  "output": "physical"
}
```

#### 4.3.2 mapAxis

Describes axis permutations as a mapping of axis names. For every output axis, specifies which input axis provides its value.

```json
{
  "type": "mapAxis",
  "input": "array",
  "output": "physical",
  "mapAxis": {"x": "dim_0", "y": "dim_1", "z": "dim_2"}
}
```

#### 4.3.3 translation

Adds a constant offset to each coordinate. The `translation` array length must match the dimensionality of both input and output.

```json
{
  "type": "translation",
  "input": "array",
  "output": "physical",
  "translation": [10.0, 20.0, 5.0]
}
```

#### 4.3.4 scale

Multiplies each coordinate by a constant factor. The `scale` array length must match the dimensionality of both input and output.

```json
{
  "type": "scale",
  "input": "array", 
  "output": "physical",
  "scale": [0.5, 0.5, 1.2]
}
```

#### 4.3.5 affine

General affine transformation using a matrix. For N-dimensional input to M-dimensional output, represented as an M×(N+1) matrix.

```json
{
  "type": "affine",
  "input": "array",
  "output": "physical",
  "affine": [
    [0.5, 0.0, 0.0, 10.0],
    [0.0, 0.5, 0.0, 20.0],
    [0.0, 0.0, 1.2, 5.0]
  ]
}
```

#### 4.3.6 rotation

Rotation transformation stored as an N×N matrix for N-dimensional spaces. Must have determinant equal to one, with orthonormal rows and columns.

```json
{
  "type": "rotation",
  "input": "array",
  "output": "physical",
  "rotation": [
    [0.866, -0.5, 0.0],
    [0.5, 0.866, 0.0],
    [0.0, 0.0, 1.0]
  ]
}
```

#### 4.3.7 sequence

A sequence of transformations applied in order. The output of one transformation becomes the input to the next.

```json
{
  "type": "sequence",
  "input": "array",
  "output": "physical",
  "transformations": [
    {
      "type": "scale",
      "scale": [0.5, 0.5, 1.2]
    },
    {
      "type": "translation",
      "translation": [10.0, 20.0, 5.0]
    }
  ]
}
```

#### 4.3.8 displacements and coordinates

Non-linear transformations using a displacement field or coordinate field. Stored in separate JNRRD data files referenced by path.

```json
{
  "type": "displacements",
  "input": "array",
  "output": "physical",
  "path": "displacements.jnrrd",
  "interpolation": "linear"
}
```

```json
{
  "type": "coordinates",
  "input": "array",
  "output": "physical",
  "path": "coordinates.jnrrd",
  "interpolation": "linear"
}
```

#### 4.3.9 inverseOf

Indicates the inverse direction of another transformation.

```json
{
  "type": "inverseOf",
  "input": "physical",
  "output": "array",
  "transformation": {
    "type": "displacements",
    "path": "displacements.jnrrd"
  }
}
```

#### 4.3.10 bijection

Explicitly defines an invertible transformation by providing both forward and inverse transformations.

```json
{
  "type": "bijection",
  "input": "array",
  "output": "physical",
  "forward": {
    "type": "displacements",
    "path": "forward_displacements.jnrrd"
  },
  "inverse": {
    "type": "displacements",
    "path": "inverse_displacements.jnrrd"
  }
}
```

#### 4.3.11 byDimension

Builds a high-dimensional transformation using lower-dimensional transformations on subsets of dimensions.

```json
{
  "type": "byDimension",
  "input": "array",
  "output": "physical",
  "transformations": [
    {
      "type": "affine",
      "input": ["dim_0", "dim_1"],
      "output": ["x", "y"],
      "affine": [[0.5, 0.0, 10.0], [0.0, 0.5, 20.0]]
    },
    {
      "type": "scale",
      "input": ["dim_2"],
      "output": ["z"],
      "scale": [1.2]
    }
  ]
}
```

### 4.4 Matrix Transformations

Matrices (affine and rotation) are applied to column vectors representing points in the input coordinate system. Matrices are stored as 2D arrays where:
- The outer array represents rows
- The inner arrays represent columns
- For example, `[[1,2,3], [4,5,6]]` has 2 rows and 3 columns

### 4.5 Invertibility

Transformations may be invertible or non-invertible:
- `identity`, `translation`, `scale` (with non-zero values), `rotation` are always invertible
- `affine` may be invertible when input and output dimensions are equal
- `sequence` is invertible if all component transformations are invertible
- `displacements` and `coordinates` are generally not invertible but can be approximated
- `inverseOf` and `bijection` explicitly handle inverse transformations

## 5. Implementation Guidelines

### 5.1 Coordination with Other Extensions

When using both the transforms extension and other extensions (e.g., NIfTI or DICOM extensions), use the appropriate coordinate system for each domain. For example:

```json
{"extensions": {
  "transform": "https://jnrrd.org/extensions/transforms/v1.0.0",
  "nifti": "https://jnrrd.org/extensions/nifti/v1.0.0"
}}
{"transform:coordinateSystems": [{
  "name": "RAS",
  "axes": [
    {"name": "x", "type": "space", "unit": "millimeter"},
    {"name": "y", "type": "space", "unit": "millimeter"},
    {"name": "z", "type": "space", "unit": "millimeter"}
  ]
}]}
{"nifti:qform_code": 1}
{"transform:transformations": [{
  "name": "array_to_RAS",
  "type": "affine",
  "input": "array",
  "output": "RAS",
  "affine": [[...]]
}]}
```

### 5.2 Storing Transformation Data

#### 5.2.1 Displacement and Coordinate Field Format

For `displacements` and `coordinates` transformations that reference external JNRRD files, the referenced files should follow the standard JNRRD format with specific structure:

- For an N-dimensional input coordinate system, the array MUST have N+1 dimensions
- The last dimension MUST have length equal to the output dimensionality (M)
- The first N dimensions correspond to the sampling grid in the input coordinate system
- For each position in the input grid, the array stores M values representing:
  - For `displacements`: Vector offsets from the current position
  - For `coordinates`: Absolute coordinates in the output space

A displacement field JNRRD file example:

```json
{"jnrrd": "0004"}
{"type": "float32"}
{"dimension": 4}  // 3D spatial grid + 1D for displacement vectors
{"sizes": [128, 128, 64, 3]}  // Last dimension (3) is for displacement vectors in 3D
{"endian": "little"}
{"encoding": "raw"}
{"space": "right_anterior_superior"}
{"extensions": {"transform": "https://jnrrd.org/extensions/transforms/v1.0.0"}}
{"transform:coordinateSystem": {
  "name": "displacementField",
  "axes": [
    {"name": "x", "type": "space", "unit": "millimeter"},
    {"name": "y", "type": "space", "unit": "millimeter"},
    {"name": "z", "type": "space", "unit": "millimeter"},
    {"name": "vector", "type": "displacement"}
  ]
}}
{"transform:sampling": {
  "x": 1.0,
  "y": 1.0,
  "z": 1.5
}}
{"transform:domain": {
  "min": [0.0, 0.0, 0.0],
  "max": [127.0, 127.0, 95.0]
}}
```

In this example:
- The displacement field is a 4D array (128×128×64×3)
- The first three dimensions define the input coordinate grid 
- The fourth dimension contains 3D displacement vectors
- Each point (x,y,z) in the displacement field maps to a point in the output space at (x+dx, y+dy, z+dz)

#### 5.2.2 Interpolation Between Grid Points

When applying displacements or coordinate transformations to points that don't exactly fall on grid points, interpolation is necessary:

- The `interpolation` parameter in the transformation definition specifies how to interpolate
- Common values include:
  - `"linear"`: (default) Linear interpolation between grid points
  - `"nearest"`: Nearest neighbor interpolation, useful for label maps
  - `"cubic"`: Higher-order interpolation for smoother results

## 6. Examples

### 6.1 Simple Scale and Translation Example

```json
{"jnrrd": "0004"}
{"type": "float32"}
{"dimension": 3}
{"sizes": [512, 512, 100]}
{"endian": "little"}
{"encoding": "raw"}
{"extensions": {"transform": "https://jnrrd.org/extensions/transforms/v1.0.0"}}
{"transform:coordinateSystems": [
  {
    "name": "array",
    "axes": [
      {"name": "dim_0", "type": "array"},
      {"name": "dim_1", "type": "array"},
      {"name": "dim_2", "type": "array"}
    ]
  },
  {
    "name": "physical",
    "axes": [
      {"name": "x", "type": "space", "unit": "millimeter"},
      {"name": "y", "type": "space", "unit": "millimeter"},
      {"name": "z", "type": "space", "unit": "millimeter"}
    ]
  }
]}
{"transform:transformations": [
  {
    "name": "array_to_physical",
    "type": "sequence",
    "input": "array",
    "output": "physical",
    "transformations": [
      {
        "type": "scale",
        "scale": [0.5, 0.5, 1.2]
      },
      {
        "type": "translation",
        "translation": [0.0, 0.0, -60.0]
      }
    ]
  }
]}
```

### 6.2 Multi-Modal Registration Example

```json
{"jnrrd": "0004"}
{"type": "float32"}
{"dimension": 3}
{"sizes": [256, 256, 128]}
{"endian": "little"}
{"encoding": "raw"}
{"extensions": {"transform": "https://jnrrd.org/extensions/transforms/v1.0.0"}}
{"transform:coordinateSystems": [
  {
    "name": "MRI",
    "axes": [
      {"name": "x", "type": "space", "unit": "millimeter"},
      {"name": "y", "type": "space", "unit": "millimeter"},
      {"name": "z", "type": "space", "unit": "millimeter"}
    ]
  },
  {
    "name": "CT",
    "axes": [
      {"name": "x", "type": "space", "unit": "millimeter"},
      {"name": "y", "type": "space", "unit": "millimeter"},
      {"name": "z", "type": "space", "unit": "millimeter"}
    ]
  }
]}
{"transform:transformations": [
  {
    "name": "MRI_to_CT",
    "type": "affine",
    "input": "MRI",
    "output": "CT",
    "affine": [
      [0.9848, -0.1736, 0.0, 10.5],
      [0.1736, 0.9848, 0.0, -5.2],
      [0.0, 0.0, 1.0, 12.0]
    ]
  },
  {
    "name": "CT_to_MRI",
    "type": "inverseOf",
    "input": "CT",
    "output": "MRI",
    "transformation": {
      "type": "affine",
      "affine": [
        [0.9848, -0.1736, 0.0, 10.5],
        [0.1736, 0.9848, 0.0, -5.2],
        [0.0, 0.0, 1.0, 12.0]
      ]
    }
  }
]}
```

### 6.3 Non-Linear Registration Example with External Displacement Field

```json
{"jnrrd": "0004"}
{"type": "float32"}
{"dimension": 3}
{"sizes": [256, 256, 128]}
{"endian": "little"}
{"encoding": "raw"}
{"extensions": {"transform": "https://jnrrd.org/extensions/transforms/v1.0.0"}}
{"transform:coordinateSystems": [
  {
    "name": "subject",
    "axes": [
      {"name": "x", "type": "space", "unit": "millimeter"},
      {"name": "y", "type": "space", "unit": "millimeter"},
      {"name": "z", "type": "space", "unit": "millimeter"}
    ]
  },
  {
    "name": "template",
    "axes": [
      {"name": "x", "type": "space", "unit": "millimeter"},
      {"name": "y", "type": "space", "unit": "millimeter"},
      {"name": "z", "type": "space", "unit": "millimeter"}
    ]
  }
]}
{"transform:transformations": [
  {
    "name": "subject_to_template",
    "type": "sequence",
    "input": "subject",
    "output": "template",
    "transformations": [
      {
        "type": "affine",
        "affine": [
          [0.97, -0.1, 0.0, 5.0],
          [0.1, 0.97, 0.0, -2.5],
          [0.0, 0.0, 1.0, 7.0]
        ]
      },
      {
        "type": "displacements",
        "path": "subject_to_template_warp.jnrrd",
        "interpolation": "linear"
      }
    ]
  }
]}
```

## 7. Validation

The JSON Schema for validating this extension is available at the extension URI: `https://jnrrd.org/extensions/transforms/v1.0.0`.

## 8. Standard Transformation Schema Validation

This extension provides a JSON Schema for validating coordinate systems and transformations:

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "JNRRD Transforms Extension",
  "description": "Schema for validating coordinate systems and transformations in JNRRD files",
  "definitions": {
    "axis": {
      "type": "object",
      "required": ["name"],
      "properties": {
        "name": {"type": "string", "minLength": 1},
        "type": {
          "type": "string", 
          "enum": ["array", "space", "time", "channel", "coordinate", "displacement"]
        },
        "unit": {"type": "string"},
        "discrete": {"type": "boolean"},
        "longName": {"type": "string"}
      }
    },
    "coordinateSystem": {
      "type": "object",
      "required": ["name", "axes"],
      "properties": {
        "name": {"type": "string", "minLength": 1},
        "axes": {
          "type": "array",
          "items": {"$ref": "#/definitions/axis"},
          "minItems": 1
        }
      }
    },
    "identity": {
      "type": "object",
      "required": ["type", "input", "output"],
      "properties": {
        "type": {"enum": ["identity"]},
        "name": {"type": "string"},
        "input": {"type": "string"},
        "output": {"type": "string"}
      }
    },
    "mapAxis": {
      "type": "object",
      "required": ["type", "input", "output", "mapAxis"],
      "properties": {
        "type": {"enum": ["mapAxis"]},
        "name": {"type": "string"},
        "input": {"type": "string"},
        "output": {"type": "string"},
        "mapAxis": {
          "type": "object",
          "additionalProperties": {"type": "string"}
        }
      }
    },
    "scale": {
      "type": "object",
      "required": ["type", "input", "output", "scale"],
      "properties": {
        "type": {"enum": ["scale"]},
        "name": {"type": "string"},
        "input": {"type": "string"},
        "output": {"type": "string"},
        "scale": {
          "type": "array",
          "items": {"type": "number"},
          "minItems": 1
        }
      }
    },
    "translation": {
      "type": "object",
      "required": ["type", "input", "output", "translation"],
      "properties": {
        "type": {"enum": ["translation"]},
        "name": {"type": "string"},
        "input": {"type": "string"},
        "output": {"type": "string"},
        "translation": {
          "type": "array",
          "items": {"type": "number"},
          "minItems": 1
        }
      }
    },
    "affine": {
      "type": "object",
      "required": ["type", "input", "output", "affine"],
      "properties": {
        "type": {"enum": ["affine"]},
        "name": {"type": "string"},
        "input": {"type": "string"},
        "output": {"type": "string"},
        "affine": {
          "type": "array",
          "items": {
            "type": "array",
            "items": {"type": "number"}
          },
          "minItems": 1
        }
      }
    },
    "rotation": {
      "type": "object",
      "required": ["type", "input", "output", "rotation"],
      "properties": {
        "type": {"enum": ["rotation"]},
        "name": {"type": "string"},
        "input": {"type": "string"},
        "output": {"type": "string"},
        "rotation": {
          "type": "array",
          "items": {
            "type": "array",
            "items": {"type": "number"}
          },
          "minItems": 1
        }
      }
    },
    "displacements": {
      "type": "object",
      "required": ["type", "input", "output", "path"],
      "properties": {
        "type": {"enum": ["displacements"]},
        "name": {"type": "string"},
        "input": {"type": "string"},
        "output": {"type": "string"},
        "path": {"type": "string"},
        "interpolation": {
          "type": "string",
          "enum": ["linear", "nearest", "cubic"]
        }
      }
    },
    "coordinates": {
      "type": "object",
      "required": ["type", "input", "output", "path"],
      "properties": {
        "type": {"enum": ["coordinates"]},
        "name": {"type": "string"},
        "input": {"type": "string"},
        "output": {"type": "string"},
        "path": {"type": "string"},
        "interpolation": {
          "type": "string",
          "enum": ["linear", "nearest", "cubic"]
        }
      }
    },
    "transformation": {
      "oneOf": [
        {"$ref": "#/definitions/identity"},
        {"$ref": "#/definitions/mapAxis"},
        {"$ref": "#/definitions/scale"},
        {"$ref": "#/definitions/translation"},
        {"$ref": "#/definitions/affine"},
        {"$ref": "#/definitions/rotation"},
        {"$ref": "#/definitions/displacements"},
        {"$ref": "#/definitions/coordinates"},
        {"$ref": "#/definitions/sequence"},
        {"$ref": "#/definitions/inverseOf"},
        {"$ref": "#/definitions/bijection"},
        {"$ref": "#/definitions/byDimension"}
      ]
    },
    "sequence": {
      "type": "object",
      "required": ["type", "input", "output", "transformations"],
      "properties": {
        "type": {"enum": ["sequence"]},
        "name": {"type": "string"},
        "input": {"type": "string"},
        "output": {"type": "string"},
        "transformations": {
          "type": "array",
          "items": {"$ref": "#/definitions/transformation"},
          "minItems": 1
        }
      }
    },
    "inverseOf": {
      "type": "object",
      "required": ["type", "input", "output", "transformation"],
      "properties": {
        "type": {"enum": ["inverseOf"]},
        "name": {"type": "string"},
        "input": {"type": "string"},
        "output": {"type": "string"},
        "transformation": {"$ref": "#/definitions/transformation"}
      }
    },
    "bijection": {
      "type": "object",
      "required": ["type", "input", "output", "forward", "inverse"],
      "properties": {
        "type": {"enum": ["bijection"]},
        "name": {"type": "string"},
        "input": {"type": "string"},
        "output": {"type": "string"},
        "forward": {"$ref": "#/definitions/transformation"},
        "inverse": {"$ref": "#/definitions/transformation"}
      }
    },
    "byDimension": {
      "type": "object",
      "required": ["type", "input", "output", "transformations"],
      "properties": {
        "type": {"enum": ["byDimension"]},
        "name": {"type": "string"},
        "input": {"type": "string"},
        "output": {"type": "string"},
        "transformations": {
          "type": "array",
          "items": {
            "type": "object",
            "allOf": [
              {"$ref": "#/definitions/transformation"},
              {
                "properties": {
                  "input": {
                    "type": "array",
                    "items": {"type": "string"},
                    "minItems": 1
                  },
                  "output": {
                    "type": "array",
                    "items": {"type": "string"},
                    "minItems": 1
                  }
                },
                "required": ["input", "output"]
              }
            ]
          },
          "minItems": 1
        }
      }
    }
  },
  "type": "object",
  "properties": {
    "coordinateSystems": {
      "type": "array",
      "items": {"$ref": "#/definitions/coordinateSystem"},
      "minItems": 1
    },
    "transformations": {
      "type": "array",
      "items": {"$ref": "#/definitions/transformation"},
      "minItems": 1
    }
  }
}
```

Implementations SHOULD validate extension data against this schema to ensure compatibility.

## 9. References

1. NG-FF Coordinate Systems and Transformations: https://github.com/ome/ngff/pull/138
2. ITK Transforms: https://itk.org/ITKSoftwareGuide/html/Book2/ITKSoftwareGuide-Book2ch3.html
3. JNRRD Specification: https://jnrrd.org/spec/current
4. Original NRRD Format: https://teem.sourceforge.net/nrrd/format.html