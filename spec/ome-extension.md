# JNRRD OME Extension Specification

Version: 1.0.0

## 1. Introduction

The JNRRD OME Extension defines a standardized approach for incorporating the Open Microscopy Environment (OME) data model into JNRRD files. This extension enables JNRRD to store and exchange biological imaging data with rich, structured metadata while maintaining compatibility with the simplicity and efficiency of the JNRRD format.

The OME data model is a well-established standard for biological microscopy data, providing a comprehensive framework for describing imaging experiments, instruments, and biological samples. By implementing the OME model as a JNRRD extension, this specification creates a bridge between medical imaging and biological microscopy workflows.

## 2. Extension Declaration

To use this extension, a JNRRD file must include the following extension declaration:

```json
{"extensions": {"ome": "https://jnrrd.org/extensions/ome/v1.0.0"}}
```

## 3. Core Concepts

### 3.1 Dimension Interpretation

In the OME model, multi-dimensional image data is typically organized with specific dimension axes:
- X, Y: Spatial dimensions
- Z: Depth (for 3D volumes)
- C: Channels (e.g., fluorescence wavelengths)
- T: Time points

JNRRD's ordered dimensions are mapped to these concepts via the `ome:dimensions` field.

### 3.2 Image and Instrument Organization

The OME data model organizes metadata hierarchically:
- Instrument: The equipment used to acquire images
- Image: The dataset itself, including acquisition parameters
- Experiment: Information about the experimental context
- Sample: Information about the biological specimen

### 3.3 Units and Quantities

Physical measurements in the OME extension include both values and units, stored as objects:

```json
{"value": 0.5, "unit": "µm"}
```

### 3.4 Hierarchical Field Notation

The OME extension fully supports JNRRD's hierarchical field notation, allowing two equivalent ways to express the same metadata:

#### Nested Object Notation

```json
{"ome:instrument": {
  "microscope": {
    "type": "confocal",
    "manufacturer": "Zeiss"
  }
}}
```

#### Flattened Hierarchy Notation

```json
{"ome:instrument.microscope.type": "confocal"}
{"ome:instrument.microscope.manufacturer": "Zeiss"}
```

These representations are functionally equivalent, and both can be used interchangeably based on the needs of the application or preference of the user.

#### Array Indexing in Flattened Hierarchies

For arrays, numeric indices are used in the path to access specific elements:

```json
# Nested form
{"ome:channels": [
  {"id": "Channel:0", "name": "DAPI", "color": "#0000FF"},
  {"id": "Channel:1", "name": "GFP", "color": "#00FF00"}
]}

# Flattened equivalent
{"ome:channels[0].id": "Channel:0"}
{"ome:channels[0].name": "DAPI"}
{"ome:channels[0].color": "#0000FF"}
{"ome:channels[1].id": "Channel:1"}
{"ome:channels[1].name": "GFP"}
{"ome:channels[1].color": "#00FF00"}
```

This approach allows efficient handling of large arrays by updating individual elements without rewriting the entire array:

```json
# Update just one property of one channel
{"ome:channels[1].color": "#22FF22"}
```

It also enables progressive building of arrays by adding elements one at a time, which is particularly useful for channels, ROIs, and other array-based OME concepts.

## 4. OME Extension Fields

### 4.1 Dimensions and Coordinate Mapping

The `ome:dimensions` field defines how JNRRD dimensions map to OME concepts and provides physical calibration information:

```json
{"ome:dimensions": {
  "order": "XYZCT",
  "physical_sizes": {
    "X": {"value": 0.1, "unit": "µm"},
    "Y": {"value": 0.1, "unit": "µm"},
    "Z": {"value": 0.5, "unit": "µm"},
    "T": {"value": 10.0, "unit": "min"}
  }
}}
```

| Field | Type | Description |
|-------|------|-------------|
| `order` | string | Mapping between JNRRD dimensions and OME concepts (X,Y,Z,C,T) |
| `physical_sizes` | object | Physical calibration information for each dimension |

The `order` string defines how JNRRD dimensions map to OME dimensions. For example, if `order` is "XYZCT" and JNRRD dimensions are [1024, 1024, 50, 3, 10], then:
- JNRRD dimension 0 (size 1024) = X dimension
- JNRRD dimension 1 (size 1024) = Y dimension
- JNRRD dimension 2 (size 50) = Z dimension
- JNRRD dimension 3 (size 3) = C (channel) dimension
- JNRRD dimension 4 (size 10) = T (time) dimension

### 4.2 Channels

The `ome:channels` field describes each channel in the dataset:

```json
{"ome:channels": [
  {
    "id": "Channel:0",
    "name": "DAPI",
    "fluorophore": "DAPI",
    "excitation_wavelength": {"value": 405, "unit": "nm"},
    "emission_wavelength": {"value": 461, "unit": "nm"},
    "color": "#0000FF",
    "light_source_ref": "LightSource:0",
    "detector_ref": "Detector:0"
  },
  {
    "id": "Channel:1",
    "name": "GFP",
    "fluorophore": "eGFP",
    "excitation_wavelength": {"value": 488, "unit": "nm"},
    "emission_wavelength": {"value": 509, "unit": "nm"},
    "color": "#00FF00"
  }
]}
```

| Field | Type | Description |
|-------|------|-------------|
| `id` | string | Unique identifier for the channel |
| `name` | string | Human-readable name |
| `fluorophore` | string | Fluorophore or stain name |
| `excitation_wavelength` | object | Wavelength for excitation |
| `emission_wavelength` | object | Wavelength for emission detection |
| `color` | string | Suggested display color (HTML hex code) |
| `light_source_ref` | string | Reference to a light source |
| `detector_ref` | string | Reference to a detector |
| `acquisition_mode` | string | Mode of acquisition for this channel |
| `emission_filter` | string | Description of emission filter |
| `excitation_filter` | string | Description of excitation filter |
| `illumination_type` | string | Type of illumination |
| `contrast_method` | string | Method used for contrast |
| `pinhole_size` | object | Size of confocal pinhole |

### 4.3 Instrument Information

The `ome:instrument` field describes the imaging system used:

```json
{"ome:instrument": {
  "id": "Instrument:0",
  "name": "Zeiss LSM 880",
  "microscope": {
    "type": "confocal",
    "manufacturer": "Zeiss",
    "model": "LSM 880",
    "serial_number": "1234567",
    "lot_number": "ABC123"
  },
  "detectors": [
    {
      "id": "Detector:0",
      "type": "PMT",
      "manufacturer": "Zeiss",
      "model": "BiG.2",
      "serial_number": "DET8901234",
      "amplification_gain": 1.0,
      "gain": 800,
      "offset": 0,
      "voltage": 450.0,
      "zoom": 1.0
    }
  ],
  "objectives": [
    {
      "id": "Objective:0",
      "manufacturer": "Zeiss",
      "model": "Plan-Apochromat",
      "serial_number": "OBJ1234567",
      "nominal_magnification": 63.0,
      "calibrated_magnification": 63.2,
      "numerical_aperture": 1.4,
      "immersion": "Oil",
      "correction": "PlanApo",
      "working_distance": {"value": 0.19, "unit": "mm"},
      "iris": false
    }
  ],
  "light_sources": [
    {
      "id": "LightSource:0",
      "type": "laser",
      "manufacturer": "Zeiss",
      "model": "Diode 405",
      "serial_number": "LS1234567",
      "power": {"value": 50.0, "unit": "mW"},
      "wavelength": {"value": 405.0, "unit": "nm"}
    },
    {
      "id": "LightSource:1",
      "type": "laser",
      "manufacturer": "Zeiss",
      "model": "Argon",
      "serial_number": "LS7654321",
      "power": {"value": 100.0, "unit": "mW"},
      "wavelength": {"value": 488.0, "unit": "nm"}
    }
  ],
  "filters": [
    {
      "id": "Filter:0",
      "manufacturer": "Zeiss",
      "model": "BP 420-480",
      "type": "BandPass",
      "transmittance_range": {
        "cut_in": {"value": 420, "unit": "nm"},
        "cut_out": {"value": 480, "unit": "nm"},
        "transmittance": 0.90
      }
    }
  ],
  "dichroics": [
    {
      "id": "Dichroic:0",
      "manufacturer": "Zeiss",
      "model": "MBS 458/514",
      "type": "MultiPass"
    }
  ]
}}
```

| Field | Type | Description |
|-------|------|-------------|
| `id` | string | Unique identifier for the instrument |
| `name` | string | Human-readable name |
| `microscope` | object | Description of the microscope |
| `detectors` | array | List of detector devices |
| `objectives` | array | List of microscope objectives |
| `light_sources` | array | List of illumination sources |
| `filters` | array | List of optical filters |
| `dichroics` | array | List of dichroic mirrors |

### 4.4 Experiment Information

The `ome:experiment` field captures metadata about the experiment:

```json
{"ome:experiment": {
  "id": "Experiment:0",
  "type": "fluorescence microscopy",
  "description": "DAPI staining of HeLa cells",
  "acquisition_date": "2023-07-15T14:30:00Z",
  "experimenter": {
    "id": "Experimenter:0",
    "first_name": "Jane",
    "last_name": "Doe",
    "email": "jane.doe@example.org",
    "institution": "University of Science",
    "orcid": "0000-0001-2345-6789"
  },
  "project": {
    "id": "Project:0",
    "name": "Cell cycle analysis",
    "description": "Analysis of nuclear morphology during cell cycle"
  },
  "funding": [
    {
      "agency": "National Science Foundation",
      "id": "NSF-12345",
      "reference": "Smith et al. 2023"
    }
  ]
}}
```

| Field | Type | Description |
|-------|------|-------------|
| `id` | string | Unique identifier for the experiment |
| `type` | string | Type of experiment |
| `description` | string | Textual description |
| `acquisition_date` | string | ISO 8601 timestamp |
| `experimenter` | object | Information about the person who performed the experiment |
| `project` | object | Information about the research project |
| `funding` | array | List of funding sources |

### 4.5 Biological Sample Information

The `ome:sample` field describes the biological specimen:

```json
{"ome:sample": {
  "id": "Sample:0",
  "name": "HeLa cells",
  "type": "cell",
  "description": "Human cervical cancer cell line",
  "preparation_date": "2023-07-14",
  "preparation_type": "cell culture",
  "organism": {
    "taxonomy_id": 9606,
    "name": "Homo sapiens",
    "strain": "HeLa",
    "genotype": "wild-type"
  },
  "tissue": {
    "name": "cervix",
    "type": "epithelial"
  },
  "treatments": [
    {
      "id": "Treatment:0",
      "type": "chemical",
      "description": "PFA fixation",
      "concentration": {"value": 4.0, "unit": "percent"},
      "duration": {"value": 15, "unit": "min"}
    }
  ],
  "temperature": {"value": 37.0, "unit": "celsius"},
  "embedding_medium": "ProLong Gold",
  "culture_medium": "DMEM + 10% FBS"
}}
```

| Field | Type | Description |
|-------|------|-------------|
| `id` | string | Unique identifier for the sample |
| `name` | string | Human-readable name |
| `type` | string | Type of specimen |
| `description` | string | Textual description |
| `preparation_date` | string | ISO 8601 date |
| `preparation_type` | string | How the sample was prepared |
| `organism` | object | Information about the organism |
| `tissue` | object | Information about the tissue |
| `treatments` | array | List of treatments applied to the sample |
| `temperature` | object | Temperature during imaging |
| `embedding_medium` | string | Medium used for embedding |
| `culture_medium` | string | Medium used for cell culture |

### 4.6 ROIs and Annotations

The `ome:rois` field describes regions of interest within the image:

```json
{"ome:rois": [
  {
    "id": "ROI:0",
    "name": "Cell nucleus",
    "description": "Nuclear region of cell 1",
    "shapes": [
      {
        "type": "rectangle",
        "x": 100.5,
        "y": 150.2,
        "width": 50.0,
        "height": 50.0,
        "z": 5,
        "c": 0,
        "t": 0,
        "stroke_color": "#FF0000",
        "stroke_width": 1.0,
        "fill_color": "rgba(255,0,0,0.2)"
      }
    ]
  },
  {
    "id": "ROI:1",
    "name": "Cell membrane",
    "shapes": [
      {
        "type": "polygon",
        "points": [[125.0, 150.0], [175.0, 150.0], [175.0, 200.0], [125.0, 200.0]],
        "z": 5,
        "c": 1,
        "t": 0
      }
    ]
  },
  {
    "id": "ROI:2",
    "name": "Spot",
    "shapes": [
      {
        "type": "point",
        "x": 200.5,
        "y": 300.2,
        "z": 10,
        "c": 2,
        "t": 0
      }
    ]
  },
  {
    "id": "ROI:3",
    "name": "Path",
    "shapes": [
      {
        "type": "polyline",
        "points": [[250.0, 350.0], [300.0, 400.0], [350.0, 350.0]],
        "z": 15,
        "c": 0,
        "t": 0
      }
    ]
  },
  {
    "id": "ROI:4",
    "name": "Circle",
    "shapes": [
      {
        "type": "ellipse",
        "x": 400.0,
        "y": 400.0,
        "radiusX": 50.0,
        "radiusY": 50.0,
        "z": 20,
        "c": 1,
        "t": 0
      }
    ]
  }
]}
```

| Field | Type | Description |
|-------|------|-------------|
| `id` | string | Unique identifier for the ROI |
| `name` | string | Human-readable name |
| `description` | string | Textual description |
| `shapes` | array | List of shapes that make up the ROI |

Each shape can be one of several types:
- `point`: Single point
- `rectangle`: Rectangular region
- `ellipse`: Elliptical region
- `polygon`: Closed polygon
- `polyline`: Open polyline
- `label`: Text annotation

Shapes include coordinates and can be associated with specific Z, C, and T indices.

### 4.7 Imaging Environment

The `ome:imaging_environment` field captures conditions during imaging:

```json
{"ome:imaging_environment": {
  "map": {
    "temperature": {"value": 37.0, "unit": "celsius"},
    "air_pressure": {"value": 101.3, "unit": "kPa"},
    "humidity": {"value": 60.0, "unit": "percent"},
    "co2_percent": {"value": 5.0, "unit": "percent"},
    "medium": "PBS",
    "chamber": "glass-bottom dish"
  }
}}
```

| Field | Type | Description |
|-------|------|-------------|
| `map` | object | Key-value pairs of environmental parameters |

### 4.8 Planes Information

The `ome:planes` field provides per-image-plane acquisition parameters:

```json
{"ome:planes": [
  {
    "the_z": 0,
    "the_c": 0,
    "the_t": 0,
    "delta_t": {"value": 0.0, "unit": "s"},
    "exposure_time": {"value": 150.0, "unit": "ms"},
    "position_x": {"value": 20.0, "unit": "µm"},
    "position_y": {"value": 30.0, "unit": "µm"},
    "position_z": {"value": 0.0, "unit": "µm"}
  },
  {
    "the_z": 0,
    "the_c": 1,
    "the_t": 0,
    "delta_t": {"value": 0.5, "unit": "s"},
    "exposure_time": {"value": 100.0, "unit": "ms"},
    "position_x": {"value": 20.0, "unit": "µm"},
    "position_y": {"value": 30.0, "unit": "µm"},
    "position_z": {"value": 0.0, "unit": "µm"}
  }
]}
```

| Field | Type | Description |
|-------|------|-------------|
| `the_z` | integer | Z-index for this plane |
| `the_c` | integer | Channel index for this plane |
| `the_t` | integer | Time index for this plane |
| `delta_t` | object | Time offset from experiment start |
| `exposure_time` | object | Exposure duration |
| `position_x` | object | X stage position |
| `position_y` | object | Y stage position |
| `position_z` | object | Z stage position |
| `hash_sha1` | string | SHA-1 hash of plane data |

### 4.9 Image Acquisition Settings

```json
{"ome:image_acquisition": {
  "id": "Image:0",
  "name": "Cell Sample 1",
  "description": "HeLa cells stained with DAPI and Phalloidin",
  "acquisition_date": "2023-07-15T14:30:00Z",
  "pixel_type": "uint16",
  "significant_bits": 12,
  "dimension_order": "XYZCT",
  "binning": "1x1",
  "stage_label": "Position1",
  "physical_size_x": {"value": 0.1, "unit": "µm"},
  "physical_size_y": {"value": 0.1, "unit": "µm"},
  "physical_size_z": {"value": 0.5, "unit": "µm"},
  "pixels_type": "uint16"
}}
```

| Field | Type | Description |
|-------|------|-------------|
| `id` | string | Unique identifier for the image |
| `name` | string | Human-readable name |
| `description` | string | Textual description |
| `acquisition_date` | string | ISO 8601 timestamp |
| `dimension_order` | string | Order of dimensions (XYZCT) |
| `binning` | string | Pixel binning used in acquisition |
| `significant_bits` | integer | Number of significant bits in pixel data |
| `pixels_type` | string | Pixel data type |

## 5. Integration with JNRRD Core Features

### 5.1 Space Mapping

The OME extension's coordinate system and the JNRRD `space` field can be explicitly linked:

```json
{"ome:space_mapping": {
  "jnrrd_space": "right_anterior_superior",
  "ome_mapping": {
    "right": "X",
    "anterior": "Y",
    "superior": "Z"
  }
}}
```

This field defines how the JNRRD coordinate system maps to the OME dimensions.

### 5.2 Tiling and Multi-Resolution Integration

When used with the JNRRD Tiling Extension, the OME extension provides guidance for biological-specific tiling strategies:

```json
{"ome:tiling_strategy": {
  "preferred_chunk_sizes": {
    "XY": [256, 256],
    "Z": 16,
    "C": 1,
    "T": 1
  },
  "access_pattern": "XY_slices"
}}
```

This field suggests optimal tiling parameters for microscopy data.

## 6. Examples

### 6.1 Minimal Fluorescence Microscopy Example

```
{"jnrrd": "0004"}
{"type": "uint16"}
{"dimension": 5}
{"sizes": [1024, 1024, 50, 3, 1]}
{"endian": "little"}
{"encoding": "raw"}
{"extensions": {"ome": "https://jnrrd.org/extensions/ome/v1.0.0"}}
{"ome:dimensions": {
  "order": "XYZCT",
  "physical_sizes": {
    "X": {"value": 0.1, "unit": "µm"},
    "Y": {"value": 0.1, "unit": "µm"},
    "Z": {"value": 0.5, "unit": "µm"}
  }
}}
{"ome:channels": [
  {"id": "Channel:0", "name": "DAPI", "fluorophore": "DAPI", "color": "#0000FF"},
  {"id": "Channel:1", "name": "GFP", "fluorophore": "eGFP", "color": "#00FF00"},
  {"id": "Channel:2", "name": "mCherry", "fluorophore": "mCherry", "color": "#FF0000"}
]}
{"ome:sample": {"id": "Sample:0", "name": "HeLa cells"}}

[BINARY DATA FOLLOWS]
```

### 6.1.1 Same Example Using Flattened Hierarchy

```
{"jnrrd": "0004"}
{"type": "uint16"}
{"dimension": 5}
{"sizes": [1024, 1024, 50, 3, 1]}
{"endian": "little"}
{"encoding": "raw"}
{"extensions": {"ome": "https://jnrrd.org/extensions/ome/v1.0.0"}}
{"ome:dimensions.order": "XYZCT"}
{"ome:dimensions.physical_sizes.X.value": 0.1}
{"ome:dimensions.physical_sizes.X.unit": "µm"}
{"ome:dimensions.physical_sizes.Y.value": 0.1}
{"ome:dimensions.physical_sizes.Y.unit": "µm"}
{"ome:dimensions.physical_sizes.Z.value": 0.5}
{"ome:dimensions.physical_sizes.Z.unit": "µm"}
{"ome:channels[0].id": "Channel:0"}
{"ome:channels[0].name": "DAPI"}
{"ome:channels[0].fluorophore": "DAPI"}
{"ome:channels[0].color": "#0000FF"}
{"ome:channels[1].id": "Channel:1"}
{"ome:channels[1].name": "GFP"}
{"ome:channels[1].fluorophore": "eGFP"}
{"ome:channels[1].color": "#00FF00"}
{"ome:channels[2].id": "Channel:2"}
{"ome:channels[2].name": "mCherry"}
{"ome:channels[2].fluorophore": "mCherry"}
{"ome:channels[2].color": "#FF0000"}
{"ome:sample.id": "Sample:0"}
{"ome:sample.name": "HeLa cells"}

[BINARY DATA FOLLOWS]
```

### 6.2 Complete Multi-Channel Time Series Example

```
{"jnrrd": "0004"}
{"type": "uint16"}
{"dimension": 5}
{"sizes": [2048, 2048, 30, 4, 20]}
{"endian": "little"}
{"encoding": "zstd"}
{"space": "right_anterior_superior"}
{"extensions": {"ome": "https://jnrrd.org/extensions/ome/v1.0.0", "tile": "https://jnrrd.org/extensions/tile/v1.0.0"}}
{"ome:dimensions": {
  "order": "XYZCT",
  "physical_sizes": {
    "X": {"value": 0.065, "unit": "µm"},
    "Y": {"value": 0.065, "unit": "µm"},
    "Z": {"value": 0.2, "unit": "µm"},
    "T": {"value": 10.0, "unit": "min"}
  }
}}
{"ome:channels": [
  {"id": "Channel:0", "name": "DAPI", "fluorophore": "DAPI", "color": "#0000FF"},
  {"id": "Channel:1", "name": "GFP", "fluorophore": "eGFP", "color": "#00FF00"},
  {"id": "Channel:2", "name": "mCherry", "fluorophore": "mCherry", "color": "#FF0000"},
  {"id": "Channel:3", "name": "Far-Red", "fluorophore": "Cy5", "color": "#FF00FF"}
]}
{"ome:instrument": {
  "id": "Instrument:0",
  "name": "Leica SP8",
  "microscope": {"type": "confocal", "manufacturer": "Leica"}
}}
{"ome:sample": {
  "id": "Sample:0",
  "name": "HeLa cells",
  "organism": {"name": "Homo sapiens"}
}}
{"ome:experiment": {
  "id": "Experiment:0",
  "type": "FRAP",
  "description": "Fluorescence Recovery After Photobleaching"
}}
{"tile:enabled": true}
{"tile:dimensions": [0, 1, 2]}
{"tile:sizes": [256, 256, 30]}
{"tile:storage": "internal"}
{"ome:tiling_strategy": {
  "preferred_chunk_sizes": {
    "XY": [256, 256],
    "Z": 30,
    "C": 1,
    "T": 1
  }
}}

[BINARY DATA FOLLOWS]
```

## 7. OME-ZARR Compatibility

The OME extension for JNRRD is designed to maintain compatibility with OME-ZARR where possible:

1. **Metadata Structure**: The organizational structure mirrors OME-ZARR's approach
2. **Field Names**: Field names align with OME-ZARR conventions
3. **Value Types**: Units and quantities use the same representation

This allows tools to convert between JNRRD+OME and OME-ZARR with minimal information loss.

## 8. Hierarchical Paths and Implementation

### 8.1 Parsing Hierarchical Fields

Implementations should follow these rules when processing OME extension fields:

1. **Construct Object Tree**: Build a hierarchical object model from all fields
2. **Process Flattened Paths**: When encountering a path like `ome:channels[0].name`, parse it as:
   ```
   extension = "ome"
   path = ["channels", 0, "name"]
   ```
3. **Path Splitting**: Split on the first colon (`:`) to separate extension namespace, then split on dots (`.`) for path components
4. **Array Recognition**: Numeric path components in square brackets (e.g., `[0]`) indicate array indices
5. **Precedence**: More specific paths (deeper in hierarchy) override less specific ones
6. **Array Creation**: If a path like `ome:channels[5].name` is encountered without intermediate indices, create missing array elements as needed

This aligns with standard JSONPath conventions, where dots are used to access object properties and bracket notation with numbers accesses array elements.

### 8.2 Limitations and Design Considerations

1. **Conflicting Representations**: 
   - When both nested and flattened representations exist, implementations must handle merge conflicts appropriately
   - Always consider the most specific (deepest) path as authoritative

2. **Dimensions**: 
   - OME extensions for JNRRD maps dimensions through the `order` field
   - Users must ensure consistency between JNRRD dimensions and OME interpretation

3. **Path Efficiency**:
   - Flattened paths allow efficient updates but can increase header line count
   - For large metadata structures, consider using a mix of nested and flattened representations

## 9. JSON Schema

The following JSON Schema can be used to validate the OME extension fields:

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "JNRRD OME Extension Schema",
  "description": "Schema for validating JNRRD OME extension fields",
  "type": "object",
  "properties": {
    "ome:dimensions": {
      "type": "object",
      "required": ["order"],
      "properties": {
        "order": {
          "type": "string",
          "pattern": "^[XYZCT]{1,5}$"
        },
        "physical_sizes": {
          "type": "object",
          "patternProperties": {
            "^[XYZCT]$": {
              "type": "object",
              "required": ["value", "unit"],
              "properties": {
                "value": {"type": "number"},
                "unit": {"type": "string"}
              }
            }
          }
        }
      }
    },
    "ome:channels": {
      "type": "array",
      "items": {
        "type": "object",
        "required": ["id"],
        "properties": {
          "id": {"type": "string"},
          "name": {"type": "string"},
          "fluorophore": {"type": "string"},
          "excitation_wavelength": {
            "type": "object",
            "properties": {
              "value": {"type": "number"},
              "unit": {"type": "string"}
            }
          },
          "emission_wavelength": {
            "type": "object",
            "properties": {
              "value": {"type": "number"},
              "unit": {"type": "string"}
            }
          },
          "color": {"type": "string"},
          "light_source_ref": {"type": "string"},
          "detector_ref": {"type": "string"},
          "acquisition_mode": {"type": "string"},
          "contrast_method": {"type": "string"}
        }
      }
    },
    "ome:instrument": {
      "type": "object",
      "properties": {
        "id": {"type": "string"},
        "name": {"type": "string"},
        "microscope": {
          "type": "object",
          "properties": {
            "type": {"type": "string"},
            "manufacturer": {"type": "string"},
            "model": {"type": "string"},
            "serial_number": {"type": "string"}
          }
        },
        "detectors": {
          "type": "array",
          "items": {
            "type": "object",
            "properties": {
              "id": {"type": "string"},
              "type": {"type": "string"},
              "manufacturer": {"type": "string"},
              "model": {"type": "string"},
              "serial_number": {"type": "string"},
              "amplification_gain": {"type": "number"},
              "gain": {"type": "number"},
              "offset": {"type": "number"},
              "voltage": {"type": "number"},
              "zoom": {"type": "number"}
            }
          }
        },
        "objectives": {
          "type": "array",
          "items": {
            "type": "object",
            "properties": {
              "id": {"type": "string"},
              "manufacturer": {"type": "string"},
              "model": {"type": "string"},
              "serial_number": {"type": "string"},
              "nominal_magnification": {"type": "number"},
              "calibrated_magnification": {"type": "number"},
              "numerical_aperture": {"type": "number"},
              "immersion": {"type": "string"},
              "correction": {"type": "string"},
              "working_distance": {
                "type": "object",
                "properties": {
                  "value": {"type": "number"},
                  "unit": {"type": "string"}
                }
              },
              "iris": {"type": "boolean"}
            }
          }
        },
        "light_sources": {
          "type": "array",
          "items": {
            "type": "object",
            "properties": {
              "id": {"type": "string"},
              "type": {"type": "string"},
              "manufacturer": {"type": "string"},
              "model": {"type": "string"},
              "serial_number": {"type": "string"},
              "power": {
                "type": "object",
                "properties": {
                  "value": {"type": "number"},
                  "unit": {"type": "string"}
                }
              },
              "wavelength": {
                "type": "object",
                "properties": {
                  "value": {"type": "number"},
                  "unit": {"type": "string"}
                }
              }
            }
          }
        },
        "filters": {
          "type": "array",
          "items": {
            "type": "object",
            "properties": {
              "id": {"type": "string"},
              "manufacturer": {"type": "string"},
              "model": {"type": "string"},
              "type": {"type": "string"},
              "transmittance_range": {
                "type": "object",
                "properties": {
                  "cut_in": {
                    "type": "object",
                    "properties": {
                      "value": {"type": "number"},
                      "unit": {"type": "string"}
                    }
                  },
                  "cut_out": {
                    "type": "object",
                    "properties": {
                      "value": {"type": "number"},
                      "unit": {"type": "string"}
                    }
                  },
                  "transmittance": {"type": "number"}
                }
              }
            }
          }
        }
      }
    },
    "ome:sample": {
      "type": "object",
      "properties": {
        "id": {"type": "string"},
        "name": {"type": "string"},
        "type": {"type": "string"},
        "description": {"type": "string"},
        "preparation_date": {"type": "string"},
        "preparation_type": {"type": "string"},
        "organism": {
          "type": "object",
          "properties": {
            "taxonomy_id": {"type": "integer"},
            "name": {"type": "string"},
            "strain": {"type": "string"},
            "genotype": {"type": "string"}
          }
        },
        "tissue": {
          "type": "object",
          "properties": {
            "name": {"type": "string"},
            "type": {"type": "string"}
          }
        },
        "treatments": {
          "type": "array",
          "items": {
            "type": "object",
            "properties": {
              "id": {"type": "string"},
              "type": {"type": "string"},
              "description": {"type": "string"},
              "concentration": {
                "type": "object",
                "properties": {
                  "value": {"type": "number"},
                  "unit": {"type": "string"}
                }
              },
              "duration": {
                "type": "object",
                "properties": {
                  "value": {"type": "number"},
                  "unit": {"type": "string"}
                }
              }
            }
          }
        }
      }
    },
    "ome:experiment": {
      "type": "object",
      "properties": {
        "id": {"type": "string"},
        "type": {"type": "string"},
        "description": {"type": "string"},
        "acquisition_date": {"type": "string"},
        "experimenter": {
          "type": "object",
          "properties": {
            "id": {"type": "string"},
            "first_name": {"type": "string"},
            "last_name": {"type": "string"},
            "email": {"type": "string"},
            "institution": {"type": "string"},
            "orcid": {"type": "string"}
          }
        }
      }
    },
    "ome:rois": {
      "type": "array",
      "items": {
        "type": "object",
        "required": ["id", "shapes"],
        "properties": {
          "id": {"type": "string"},
          "name": {"type": "string"},
          "description": {"type": "string"},
          "shapes": {
            "type": "array",
            "items": {
              "type": "object",
              "required": ["type"],
              "properties": {
                "type": {
                  "type": "string",
                  "enum": ["point", "rectangle", "ellipse", "polygon", "polyline", "label"]
                },
                "x": {"type": "number"},
                "y": {"type": "number"},
                "width": {"type": "number"},
                "height": {"type": "number"},
                "radiusX": {"type": "number"},
                "radiusY": {"type": "number"},
                "points": {
                  "type": "array",
                  "items": {
                    "type": "array",
                    "items": {"type": "number"},
                    "minItems": 2,
                    "maxItems": 2
                  }
                },
                "z": {"type": "integer"},
                "c": {"type": "integer"},
                "t": {"type": "integer"},
                "stroke_color": {"type": "string"},
                "stroke_width": {"type": "number"},
                "fill_color": {"type": "string"},
                "text": {"type": "string"},
                "font_family": {"type": "string"},
                "font_size": {"type": "number"}
              }
            }
          }
        }
      }
    },
    "ome:planes": {
      "type": "array",
      "items": {
        "type": "object",
        "required": ["the_z", "the_c", "the_t"],
        "properties": {
          "the_z": {"type": "integer"},
          "the_c": {"type": "integer"},
          "the_t": {"type": "integer"},
          "delta_t": {
            "type": "object",
            "properties": {
              "value": {"type": "number"},
              "unit": {"type": "string"}
            }
          },
          "exposure_time": {
            "type": "object",
            "properties": {
              "value": {"type": "number"},
              "unit": {"type": "string"}
            }
          },
          "position_x": {
            "type": "object",
            "properties": {
              "value": {"type": "number"},
              "unit": {"type": "string"}
            }
          },
          "position_y": {
            "type": "object",
            "properties": {
              "value": {"type": "number"},
              "unit": {"type": "string"}
            }
          },
          "position_z": {
            "type": "object",
            "properties": {
              "value": {"type": "number"},
              "unit": {"type": "string"}
            }
          }
        }
      }
    }
  }
}
```