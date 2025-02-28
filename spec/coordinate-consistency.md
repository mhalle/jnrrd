# JNRRD Coordinate System Consistency Guide

Version: 1.0.0

## 1. Introduction

This guide addresses how to create JNRRD files that maintain coordinate system consistency between core NRRD spatial information and multiple extensions (DICOM, NIfTI, and transforms). The goal is to ensure that spatial information is represented consistently across all these representations, avoiding conflicts or ambiguities.

## 2. Coordinate Systems Overview

Different medical and scientific imaging formats use different coordinate system conventions:

| Format | Primary Coordinate System | Axis Order | Origin Convention |
|--------|--------------------------|------------|-------------------|
| NRRD   | Right-Anterior-Superior (RAS) or Left-Posterior-Superior (LPS) | [x,y,z] | First voxel center |
| DICOM  | Left-Posterior-Superior (LPS) | [x,y,z] | Upper-left corner of first pixel |
| NIfTI  | Right-Anterior-Superior (RAS) | [x,y,z] | Center of the volume |
| ITK/VTK| Right-Anterior-Superior (RAS) | [x,y,z] | Origin specified by transform |

## 3. Core JNRRD Spatial Fields

JNRRD defines three primary spatial fields:

```json
{"space": "right-anterior-superior"}  // or "left-posterior-superior"
{"space_directions": [
  [x1, y1, z1],  // direction 1
  [x2, y2, z2],  // direction 2
  [x3, y3, z3]   // direction 3
]}
{"space_origin": [ox, oy, oz]}
```

These fields define:
1. The coordinate system convention
2. The direction and scaling of each axis
3. The origin point in physical space

## 4. Extension-Specific Spatial Information

### 4.1 DICOM Extension

```json
{"dicom:image": {
  "image_orientation_patient": [Xx, Xy, Xz, Yx, Yy, Yz],
  "image_position_patient": [Sx, Sy, Sz],
  "pixel_spacing": [Δj, Δi]
}}
```

### 4.2 NIfTI Extension

```json
{"nifti:qform_code": 1}
{"nifti:sform_code": 1}
{"nifti:qform_matrix": [
  [m11, m12, m13, m14],
  [m21, m22, m23, m24],
  [m31, m32, m33, m34],
  [0, 0, 0, 1]
]}
{"nifti:sform_matrix": [
  [m11, m12, m13, m14],
  [m21, m22, m23, m24],
  [m31, m32, m33, m34],
  [0, 0, 0, 1]
]}
```

### 4.3 Transforms Extension

```json
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
    "type": "affine",
    "input": "array",
    "output": "physical",
    "affine": [
      [a11, a12, a13, a14],
      [a21, a22, a23, a24],
      [a31, a32, a33, a34],
      [0, 0, 0, 1]
    ]
  }
]}
```

## 5. Maintaining Consistency

To maintain consistency across all representations, follow these principles:

1. **Choose a Primary Representation**: Define one representation (typically core NRRD fields) as the primary source of truth.
2. **Derive Extension Values**: Calculate extension-specific values from the primary representation.
3. **Validate Consistency**: Ensure that all representations define the same spatial transformation.

## 6. Consistency Guidelines

### 6.1 NRRD and DICOM Consistency

When `space` is "left-posterior-superior" and the orientation is axis-aligned:

```json
// Core NRRD fields
{"space": "left-posterior-superior"}
{"space_directions": [
  [dx, 0, 0],
  [0, dy, 0],
  [0, 0, dz]
]}
{"space_origin": [ox, oy, oz]}

// Equivalent DICOM fields
{"dicom:image": {
  "image_orientation_patient": [1, 0, 0, 0, 1, 0],
  "image_position_patient": [ox, oy, oz],
  "pixel_spacing": [dy, dx]  // Note the reversal: DICOM [row_spacing, col_spacing]
}}
```

For non-axis-aligned orientations:

```json
// Core NRRD fields
{"space": "left-posterior-superior"}
{"space_directions": [
  [r1x, r1y, r1z],
  [r2x, r2y, r2z],
  [r3x, r3y, r3z]
]}
{"space_origin": [ox, oy, oz]}

// Equivalent DICOM fields
{"dicom:image": {
  "image_orientation_patient": [
    r1x/|r1|, r1y/|r1|, r1z/|r1|,
    r2x/|r2|, r2y/|r2|, r2z/|r2|
  ],
  "image_position_patient": [ox, oy, oz],
  "pixel_spacing": [|r2|, |r1|]  // Magnitudes of row and column vectors
}}
```

Where |r1|, |r2| are the magnitudes of the direction vectors.

### 6.2 NRRD and NIfTI Consistency

When `space` is "right-anterior-superior":

```json
// Core NRRD fields
{"space": "right-anterior-superior"}
{"space_directions": [
  [r1x, r1y, r1z],
  [r2x, r2y, r2z],
  [r3x, r3y, r3z]
]}
{"space_origin": [ox, oy, oz]}

// Equivalent NIfTI fields
{"nifti:sform_code": 1}
{"nifti:sform_matrix": [
  [r1x, r2x, r3x, ox],
  [r1y, r2y, r3y, oy],
  [r1z, r2z, r3z, oz],
  [0, 0, 0, 1]
]}
```

When `space` is "left-posterior-superior", a conversion is needed:

```json
// NIfTI fields derived from LPS NRRD fields
{"nifti:sform_code": 1}
{"nifti:sform_matrix": [
  [-r1x, -r2x, -r3x, -ox],
  [-r1y, -r2y, -r3y, -oy],
  [r1z, r2z, r3z, oz],
  [0, 0, 0, 1]
]}
```

### 6.3 NRRD and Transforms Extension Consistency

```json
// Core NRRD fields
{"space": "right-anterior-superior"}
{"space_directions": [
  [r1x, r1y, r1z],
  [r2x, r2y, r2z],
  [r3x, r3y, r3z]
]}
{"space_origin": [ox, oy, oz]}

// Equivalent transforms extension fields
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
    "name": "RAS",
    "axes": [
      {"name": "x", "type": "space", "unit": "millimeter"},
      {"name": "y", "type": "space", "unit": "millimeter"},
      {"name": "z", "type": "space", "unit": "millimeter"}
    ]
  }
]}

{"transform:transformations": [
  {
    "name": "array_to_RAS",
    "type": "affine",
    "input": "array",
    "output": "RAS",
    "affine": [
      [r1x, r2x, r3x, ox],
      [r1y, r2y, r3y, oy],
      [r1z, r2z, r3z, oz],
      [0, 0, 0, 1]
    ]
  }
]}
```

## 7. Complete Consistent Example

Here's a complete JNRRD example with consistent spatial information across all representations:

```json
{"jnrrd": "0004"}
{"type": "float32"}
{"dimension": 3}
{"sizes": [256, 256, 120]}
{"endian": "little"}
{"encoding": "raw"}

// Core NRRD fields (PRIMARY)
{"space": "left-posterior-superior"}
{"space_directions": [
  [0.9375, 0, 0],
  [0, 0.9375, 0],
  [0, 0, 1.0]
]}
{"space_origin": [-120.0, -120.0, -60.0]}

// Extensions declaration
{"extensions": {
  "dicom": "https://jnrrd.org/extensions/dicom/v1.0.0",
  "nifti": "https://jnrrd.org/extensions/nifti/v1.0.0",
  "transform": "https://jnrrd.org/extensions/transforms/v1.0.0"
}}

// DICOM extension (derived from core fields)
{"dicom:patient": {"id": "ANONYMOUS", "sex": "M"}}
{"dicom:study": {"description": "BRAIN MRI"}}
{"dicom:image": {
  "image_orientation_patient": [1.0, 0.0, 0.0, 0.0, 1.0, 0.0],
  "image_position_patient": [-120.0, -120.0, -60.0],
  "pixel_spacing": [0.9375, 0.9375]
}}

// NIfTI extension (derived from core fields)
{"nifti:intent_code": 0}
{"nifti:sform_code": 1}
{"nifti:sform_matrix": [
  [-0.9375, 0, 0, 120.0],  // Note: LPS to RAS conversion
  [0, -0.9375, 0, 120.0],
  [0, 0, 1.0, -60.0],
  [0, 0, 0, 1]
]}

// Transforms extension (derived from core fields)
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
    "name": "LPS",
    "axes": [
      {"name": "x", "type": "space", "unit": "millimeter"},
      {"name": "y", "type": "space", "unit": "millimeter"},
      {"name": "z", "type": "space", "unit": "millimeter"}
    ]
  },
  {
    "name": "RAS",
    "axes": [
      {"name": "x", "type": "space", "unit": "millimeter"},
      {"name": "y", "type": "space", "unit": "millimeter"},
      {"name": "z", "type": "space", "unit": "millimeter"}
    ]
  }
]}

{"transform:transformations": [
  {
    "name": "array_to_LPS",
    "type": "affine",
    "input": "array",
    "output": "LPS",
    "affine": [
      [0.9375, 0, 0, -120.0],
      [0, 0.9375, 0, -120.0],
      [0, 0, 1.0, -60.0],
      [0, 0, 0, 1]
    ]
  },
  {
    "name": "LPS_to_RAS",
    "type": "affine",
    "input": "LPS",
    "output": "RAS",
    "affine": [
      [-1.0, 0, 0, 0],
      [0, -1.0, 0, 0],
      [0, 0, 1.0, 0],
      [0, 0, 0, 1]
    ]
  }
]}
```

## 8. Validation Approaches

To ensure consistency across representations:

1. **Compute Physical Coordinates**: Calculate the physical coordinates of specific voxels (e.g., [0,0,0], [1,0,0], [0,1,0]) using each representation and compare.

2. **Ensure Transformations Compose**: Verify that combined transformations (e.g., array → LPS → RAS) yield the same result as direct transformations.

3. **Check Orientation Consistency**: Ensure that all representations agree on which direction is left/right, anterior/posterior, and superior/inferior.

## 9. Recommendations

1. **Always Start with Core NRRD Fields**: Define the core spatial information first, then derive extension-specific representations.

2. **Document Coordinate Systems**: Clearly document which coordinate system convention is used (`space` field).

3. **Handle Conversions Explicitly**: When using multiple coordinate systems (e.g., LPS and RAS), explicitly define the transformation between them.

4. **Verify at File Creation**: Validate coordinate system consistency when creating JNRRD files rather than relying on readers to handle inconsistencies.

## 10. References

1. NRRD Format: https://teem.sourceforge.net/nrrd/format.html
2. DICOM Standard Coordinate System: DICOM PS3.3, Section C.7.6.2.1.1
3. NIfTI Coordinate Systems: https://nifti.nimh.nih.gov/nifti-1/documentation/nifti1fields
4. JNRRD Specification: https://jnrrd.org/spec/current
5. ITK Orientation Documentation: https://itk.org/ITKSoftwareGuide/html/Book2/ITKSoftwareGuide-Book2ch3.html#x26-1170003.9