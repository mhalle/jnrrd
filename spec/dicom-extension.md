# JNRRD DICOM Extension Specification

Version: 1.0.0

## 1. Introduction

The JNRRD DICOM Extension defines a standardized approach for integrating DICOM (Digital Imaging and Communications in Medicine) metadata with JNRRD files. This extension enables JNRRD files to maintain compatibility with medical imaging workflows while benefiting from JNRRD's JSON-based structure and binary data handling.

## 2. Extension Declaration

To use this extension, a JNRRD file must include the following extension declaration:

```json
{"extensions": {"dicom": "https://jnrrd.org/extensions/dicom/v1.0.0"}}
```

## 3. Core Concepts

### 3.1 DICOM Information Organization

DICOM metadata is organized hierarchically into the following categories:

1. **Patient**: Subject of the medical study
2. **Study**: Collection of series for a medical purpose 
3. **Series**: Set of images with the same modality and acquisition parameters
4. **Equipment**: Imaging device information
5. **Image**: Per-slice acquisition parameters and image details
6. **Protocol**: Acquisition protocol details

### 3.2 Anonymization

This extension is designed with privacy considerations in mind:

1. **Explicit patient identifiers** are never required
2. **Anonymized identifiers** should be used when possible
3. **Private DICOM tags** are supported but should be used cautiously

## 4. DICOM Extension Fields

### 4.1 Patient Information

```json
{"dicom:patient": {
  "id": "ANONYMOUS",
  "age": "035Y",
  "sex": "F",
  "weight": 70.5,
  "size": 165.0,
  "position": "HFS"
}}
```

| Field | DICOM Tag | VR | Description |
|-------|-----------|----|--------------------|
| `id` | (0010,0020) | LO | Patient ID (anonymized) |
| `age` | (0010,1010) | AS | Patient's age, format: nnnD/W/M/Y |
| `sex` | (0010,0040) | CS | Patient's sex: M (male), F (female), O (other) |
| `weight` | (0010,1030) | DS | Patient's weight in kg |
| `size` | (0010,1020) | DS | Patient's height in cm |
| `position` | (0018,5100) | CS | Patient position (e.g., HFS, FFP, etc.) |

### 4.2 Study Information

```json
{"dicom:study": {
  "instance_uid": "1.2.840.113619.2.334.3.2831183778.864.1629754903.547",
  "date": "20210823",
  "time": "152143",
  "description": "BRAIN MRI",
  "id": "12345",
  "accession_number": "54321",
  "referring_physician": "ANONYMOUS"
}}
```

| Field | DICOM Tag | VR | Description |
|-------|-----------|----|--------------------|
| `instance_uid` | (0020,000D) | UI | Study Instance UID |
| `date` | (0008,0020) | DA | Study date (YYYYMMDD) |
| `time` | (0008,0030) | TM | Study time (HHMMSS.FFFFFF) |
| `description` | (0008,1030) | LO | Study description |
| `id` | (0020,0010) | SH | Study ID |
| `accession_number` | (0008,0050) | SH | Accession Number |
| `referring_physician` | (0008,0090) | PN | Referring Physician's Name |

### 4.3 Series Information

```json
{"dicom:series": {
  "instance_uid": "1.2.840.113619.2.334.3.2831183778.864.1629754903.548",
  "number": 5,
  "description": "T1 AXIAL",
  "modality": "MR",
  "body_part": "BRAIN",
  "protocol_name": "T1_AXIAL",
  "date": "20210823",
  "time": "153015"
}}
```

| Field | DICOM Tag | VR | Description |
|-------|-----------|----|--------------------|
| `instance_uid` | (0020,000E) | UI | Series Instance UID |
| `number` | (0020,0011) | IS | Series Number |
| `description` | (0008,103E) | LO | Series Description |
| `modality` | (0008,0060) | CS | Modality (MR, CT, PET, etc.) |
| `body_part` | (0018,0015) | CS | Body Part Examined |
| `protocol_name` | (0018,1030) | LO | Protocol Name |
| `date` | (0008,0021) | DA | Series Date |
| `time` | (0008,0031) | TM | Series Time |

### 4.4 Equipment Information

```json
{"dicom:equipment": {
  "manufacturer": "GE MEDICAL SYSTEMS",
  "institution_name": "GENERAL HOSPITAL",
  "station_name": "MRI01",
  "manufacturer_model_name": "SIGNA EXPLORER",
  "device_serial_number": "00000000B12345",
  "software_versions": "DV26.0_R01"
}}
```

| Field | DICOM Tag | VR | Description |
|-------|-----------|----|--------------------|
| `manufacturer` | (0008,0070) | LO | Manufacturer |
| `institution_name` | (0008,0080) | LO | Institution Name |
| `station_name` | (0008,1010) | SH | Station Name |
| `manufacturer_model_name` | (0008,1090) | LO | Manufacturer's Model Name |
| `device_serial_number` | (0018,1000) | LO | Device Serial Number |
| `software_versions` | (0018,1020) | LO | Software Versions |

### 4.5 Image Information

```json
{"dicom:image": {
  "type": ["ORIGINAL", "PRIMARY", "M_SE", "M", "SE"],
  "acquisition_number": 1,
  "instance_number": 15,
  "image_orientation_patient": [1.0, 0.0, 0.0, 0.0, 1.0, 0.0],
  "image_position_patient": [-100.0, -100.0, -50.0],
  "slice_location": 23.5,
  "samples_per_pixel": 1,
  "rows": 256,
  "columns": 256,
  "pixel_spacing": [0.9375, 0.9375],
  "bits_allocated": 16,
  "bits_stored": 12,
  "high_bit": 11,
  "pixel_representation": 0,
  "window_center": 600,
  "window_width": 1600,
  "rescale_intercept": 0,
  "rescale_slope": 1.0,
  "photometric_interpretation": "MONOCHROME2"
}}
```

| Field | DICOM Tag | VR | Description |
|-------|-----------|----|--------------------|
| `type` | (0008,0008) | CS | Image Type |
| `acquisition_number` | (0020,0012) | IS | Acquisition Number |
| `instance_number` | (0020,0013) | IS | Instance Number |
| `image_orientation_patient` | (0020,0037) | DS | Image Orientation (Patient) |
| `image_position_patient` | (0020,0032) | DS | Image Position (Patient) |
| `slice_location` | (0020,1041) | DS | Slice Location |
| `samples_per_pixel` | (0028,0002) | US | Samples per Pixel |
| `rows` | (0028,0010) | US | Rows |
| `columns` | (0028,0011) | US | Columns |
| `pixel_spacing` | (0028,0030) | DS | Pixel Spacing |
| `bits_allocated` | (0028,0100) | US | Bits Allocated |
| `bits_stored` | (0028,0101) | US | Bits Stored |
| `high_bit` | (0028,0102) | US | High Bit |
| `pixel_representation` | (0028,0103) | US | Pixel Representation |
| `window_center` | (0028,1050) | DS | Window Center |
| `window_width` | (0028,1051) | DS | Window Width |
| `rescale_intercept` | (0028,1052) | DS | Rescale Intercept |
| `rescale_slope` | (0028,1053) | DS | Rescale Slope |
| `photometric_interpretation` | (0028,0004) | CS | Photometric Interpretation |

### 4.6 MR-specific Parameters

```json
{"dicom:mr": {
  "scanning_sequence": "SE",
  "sequence_variant": "SK",
  "scan_options": "SAT",
  "mr_acquisition_type": "2D",
  "repetition_time": 2500.0,
  "echo_time": 15.0,
  "echo_train_length": 1,
  "inversion_time": 0.0,
  "trigger_time": 0.0,
  "flip_angle": 90.0,
  "spacing_between_slices": 5.0,
  "number_of_averages": 1.0,
  "imaging_frequency": 63.87,
  "imaged_nucleus": "1H",
  "magnetic_field_strength": 1.5,
  "sar": 0.12,
  "db_dt": 2.5,
  "acquisition_matrix": [256, 0, 0, 256],
  "phase_encoding_direction": "ROW",
  "pixel_bandwidth": 130.0
}}
```

| Field | DICOM Tag | VR | Description |
|-------|-----------|----|--------------------|
| `scanning_sequence` | (0018,0020) | CS | Scanning Sequence |
| `sequence_variant` | (0018,0021) | CS | Sequence Variant |
| `scan_options` | (0018,0022) | CS | Scan Options |
| `mr_acquisition_type` | (0018,0023) | CS | MR Acquisition Type |
| `repetition_time` | (0018,0080) | DS | Repetition Time |
| `echo_time` | (0018,0081) | DS | Echo Time |
| `echo_train_length` | (0018,0091) | IS | Echo Train Length |
| `inversion_time` | (0018,0082) | DS | Inversion Time |
| `trigger_time` | (0018,1060) | DS | Trigger Time |
| `flip_angle` | (0018,1314) | DS | Flip Angle |
| `spacing_between_slices` | (0018,0088) | DS | Spacing Between Slices |
| `number_of_averages` | (0018,0083) | DS | Number of Averages |
| `imaging_frequency` | (0018,0084) | DS | Imaging Frequency |
| `imaged_nucleus` | (0018,0085) | SH | Imaged Nucleus |
| `magnetic_field_strength` | (0018,0087) | DS | Magnetic Field Strength |
| `sar` | (0018,1316) | DS | SAR |
| `db_dt` | (0018,1318) | DS | dB/dt |
| `acquisition_matrix` | (0018,1310) | US | Acquisition Matrix |
| `phase_encoding_direction` | (0018,1312) | CS | Phase Encoding Direction |
| `pixel_bandwidth` | (0018,0095) | DS | Pixel Bandwidth |

### 4.7 CT-specific Parameters

```json
{"dicom:ct": {
  "kvp": 120.0,
  "tube_current": 250.0,
  "exposure_time": 1250,
  "exposure": 312.5,
  "filter_type": "BODY",
  "convolution_kernel": "STANDARD",
  "focal_spot": 0.7,
  "rotation_direction": "CW",
  "exposure_modulation_type": "TCM",
  "estimated_dose_saving": 32.0,
  "ctdi_vol": 10.5,
  "ct_dose_length_product": 210.0,
  "revolution_time": 0.5,
  "single_collimation_width": 0.625,
  "total_collimation_width": 40.0,
  "table_height": 180.0,
  "gantry_detector_tilt": 0.0,
  "table_feed_per_rotation": 27.5,
  "spiral_pitch_factor": 0.9844,
  "data_collection_diameter": 500.0,
  "reconstruction_diameter": 350.0,
  "distance_source_to_detector": 950.0,
  "distance_source_to_patient": 600.0
}}
```

| Field | DICOM Tag | VR | Description |
|-------|-----------|----|--------------------|
| `kvp` | (0018,0060) | DS | KVP |
| `tube_current` | (0018,1151) | IS | X-Ray Tube Current |
| `exposure_time` | (0018,1150) | IS | Exposure Time |
| `exposure` | (0018,1152) | IS | Exposure |
| `filter_type` | (0018,1160) | SH | Filter Type |
| `convolution_kernel` | (0018,1210) | SH | Convolution Kernel |
| `focal_spot` | (0018,1190) | DS | Focal Spot |
| `rotation_direction` | (0018,1140) | CS | Rotation Direction |
| `exposure_modulation_type` | (0018,9323) | CS | Exposure Modulation Type |
| `estimated_dose_saving` | (0018,9324) | FD | Estimated Dose Saving |
| `ctdi_vol` | (0018,9345) | FD | CTDIvol |
| `ct_dose_length_product` | (0018,9302) | FD | CT Dose Length Product |
| `revolution_time` | (0018,9305) | FD | Revolution Time |
| `single_collimation_width` | (0018,9306) | FD | Single Collimation Width |
| `total_collimation_width` | (0018,9307) | FD | Total Collimation Width |
| `table_height` | (0018,1130) | DS | Table Height |
| `gantry_detector_tilt` | (0018,1120) | DS | Gantry/Detector Tilt |
| `table_feed_per_rotation` | (0018,9309) | FD | Table Feed per Rotation |
| `spiral_pitch_factor` | (0018,9311) | FD | Spiral Pitch Factor |
| `data_collection_diameter` | (0018,0090) | DS | Data Collection Diameter |
| `reconstruction_diameter` | (0018,1100) | DS | Reconstruction Diameter |
| `distance_source_to_detector` | (0018,1110) | DS | Distance Source to Detector |
| `distance_source_to_patient` | (0018,1111) | DS | Distance Source to Patient |

### 4.8 Complete DICOM Dictionary

For fields not explicitly covered in the sections above, the extension supports any standard DICOM tag using the `dicom:tag` field with DICOM tag addresses:

```json
{"dicom:tag": {
  "00080070": "GE MEDICAL SYSTEMS",
  "00200052": "1.2.840.113619.2.334.3.2831183778.864.1629754903.545"
}}
```

This allows for inclusion of any DICOM data element using its hexadecimal group and element numbers.

### 4.9 Private DICOM Tags

Private DICOM tags can be included using the `dicom:private_tag` field:

```json
{"dicom:private_tag": {
  "01910010": {
    "creator": "SIEMENS MR HEADER",
    "value": "30.5"
  }
}}
```

## 5. Relationship with JNRRD Space Information

The DICOM coordinate system information should be mapped to the corresponding JNRRD space fields:

```json
# DICOM coordinate system in JNRRD fields
{"space": "left_posterior_superior"}  # Standard DICOM patient coordinate system
{"space_directions": [
  [0.9375, 0.0, 0.0],
  [0.0, 0.9375, 0.0],
  [0.0, 0.0, 5.0]
]}
{"space_origin": [-100.0, -100.0, -50.0]}
```

The values should correspond to:
- `image_orientation_patient` and `pixel_spacing` for `space_directions`
- `image_position_patient` for `space_origin`

## 6. Examples

### 6.1 Minimal MRI Example

```
{"jnrrd": "0004"}
{"type": "int16"}
{"dimension": 3}
{"sizes": [256, 256, 24]}
{"endian": "little"}
{"encoding": "raw"}
{"space": "left_posterior_superior"}
{"space_directions": [
  [0.9375, 0.0, 0.0],
  [0.0, 0.9375, 0.0],
  [0.0, 0.0, 5.0]
]}
{"space_origin": [-100.0, -100.0, -50.0]}
{"extensions": {"dicom": "https://jnrrd.org/extensions/dicom/v1.0.0"}}
{"dicom:patient": {"id": "ANONYMOUS", "sex": "F"}}
{"dicom:study": {"description": "BRAIN MRI", "date": "20210823"}}
{"dicom:series": {"modality": "MR", "description": "T1 AXIAL"}}
{"dicom:mr": {"repetition_time": 500.0, "echo_time": 14.0, "flip_angle": 90.0}}
{"dicom:equipment": {"manufacturer": "GE MEDICAL SYSTEMS"}}

[BINARY DATA FOLLOWS]
```

### 6.2 Complete CT Example

```
{"jnrrd": "0004"}
{"type": "int16"}
{"dimension": 3}
{"sizes": [512, 512, 120]}
{"endian": "little"}
{"encoding": "gzip"}
{"space": "left_posterior_superior"}
{"space_directions": [
  [0.7, 0.0, 0.0],
  [0.0, 0.7, 0.0],
  [0.0, 0.0, 0.625]
]}
{"space_origin": [-175.0, -175.0, -100.0]}
{"extensions": {"dicom": "https://jnrrd.org/extensions/dicom/v1.0.0"}}
{"dicom:patient": {
  "id": "ANONYMOUS", 
  "age": "065Y", 
  "sex": "M", 
  "weight": 82.5, 
  "position": "HFS"
}}
{"dicom:study": {
  "instance_uid": "1.2.840.113619.2.12345.6789.12345.54321", 
  "date": "20210901", 
  "time": "092530", 
  "description": "CHEST CT", 
  "accession_number": "CT12345"
}}
{"dicom:series": {
  "modality": "CT", 
  "description": "CHEST WITH CONTRAST", 
  "protocol_name": "CHEST_ROUTINE"
}}
{"dicom:equipment": {
  "manufacturer": "GE MEDICAL SYSTEMS", 
  "manufacturer_model_name": "REVOLUTION CT", 
  "software_versions": "rev2.1"
}}
{"dicom:image": {
  "type": ["ORIGINAL", "PRIMARY"], 
  "window_center": 40, 
  "window_width": 350, 
  "rescale_intercept": -1024, 
  "rescale_slope": 1.0
}}
{"dicom:ct": {
  "kvp": 120.0, 
  "tube_current": 210.0, 
  "convolution_kernel": "STANDARD", 
  "ctdi_vol": 8.2, 
  "ct_dose_length_product": 205.0, 
  "spiral_pitch_factor": 0.98
}}

[BINARY DATA FOLLOWS]
```

## 7. DICOM Conversion and Compatibility

### 7.1 Converting DICOM to JNRRD

When converting from DICOM files to JNRRD:

1. Extract metadata from DICOM headers
2. Map to corresponding JNRRD DICOM extension fields
3. Map patient orientation to JNRRD space fields
4. Extract pixel data to JNRRD binary section
5. Apply rescale slope/intercept if appropriate

### 7.2 Converting JNRRD to DICOM

When converting from JNRRD with DICOM extension to DICOM:

1. Create DICOM headers from JNRRD DICOM extension fields
2. Map JNRRD space fields to DICOM orientation/position fields
3. Convert JNRRD binary data to DICOM pixel data
4. Create appropriate UIDs if not already present
5. Ensure required DICOM Type 1 elements are present

## 8. Best Practices

1. **Patient Privacy**: Always anonymize or de-identify patient information
2. **Required Fields**: Include at least modality, study description, and series description
3. **Coordinate Systems**: Ensure consistent mapping between DICOM and JNRRD coordinates
4. **Value Units**: Follow DICOM conventions for units (e.g., mm, ms, °)
5. **Validation**: Validate DICOM UIDs are globally unique and properly formatted
6. **Multi-Frame Compatibility**: Map 3D JNRRD to DICOM Enhanced Multi-frame objects when possible

## 9. JSON Schema

The following JSON Schema can be used to validate the DICOM extension fields:

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "JNRRD DICOM Extension Schema",
  "description": "Schema for validating JNRRD DICOM extension fields",
  "type": "object",
  "properties": {
    "dicom:patient": {
      "type": "object",
      "properties": {
        "id": { "type": "string" },
        "age": { 
          "type": "string",
          "pattern": "^\\d{3}[DWMY]$"
        },
        "sex": { 
          "type": "string",
          "enum": ["M", "F", "O"]
        },
        "weight": { "type": "number" },
        "size": { "type": "number" },
        "position": { "type": "string" }
      }
    },
    "dicom:study": {
      "type": "object",
      "properties": {
        "instance_uid": { "type": "string" },
        "date": { 
          "type": "string",
          "pattern": "^\\d{8}$"
        },
        "time": { 
          "type": "string",
          "pattern": "^\\d{6}(\\.\\d{1,6})?$"
        },
        "description": { "type": "string" },
        "id": { "type": "string" },
        "accession_number": { "type": "string" },
        "referring_physician": { "type": "string" }
      }
    },
    "dicom:series": {
      "type": "object",
      "properties": {
        "instance_uid": { "type": "string" },
        "number": { "type": "integer" },
        "description": { "type": "string" },
        "modality": { "type": "string" },
        "body_part": { "type": "string" },
        "protocol_name": { "type": "string" },
        "date": { 
          "type": "string",
          "pattern": "^\\d{8}$"
        },
        "time": { 
          "type": "string",
          "pattern": "^\\d{6}(\\.\\d{1,6})?$"
        }
      }
    },
    "dicom:equipment": {
      "type": "object",
      "properties": {
        "manufacturer": { "type": "string" },
        "institution_name": { "type": "string" },
        "station_name": { "type": "string" },
        "manufacturer_model_name": { "type": "string" },
        "device_serial_number": { "type": "string" },
        "software_versions": { "type": "string" }
      }
    },
    "dicom:image": {
      "type": "object",
      "properties": {
        "type": { 
          "type": "array",
          "items": { "type": "string" }
        },
        "acquisition_number": { "type": "integer" },
        "instance_number": { "type": "integer" },
        "image_orientation_patient": {
          "type": "array",
          "items": { "type": "number" },
          "minItems": 6,
          "maxItems": 6
        },
        "image_position_patient": {
          "type": "array",
          "items": { "type": "number" },
          "minItems": 3,
          "maxItems": 3
        },
        "slice_location": { "type": "number" },
        "samples_per_pixel": { "type": "integer" },
        "rows": { "type": "integer" },
        "columns": { "type": "integer" },
        "pixel_spacing": {
          "type": "array",
          "items": { "type": "number" },
          "minItems": 2,
          "maxItems": 2
        },
        "bits_allocated": { "type": "integer" },
        "bits_stored": { "type": "integer" },
        "high_bit": { "type": "integer" },
        "pixel_representation": { "type": "integer" },
        "window_center": { "type": ["number", "string"] },
        "window_width": { "type": ["number", "string"] },
        "rescale_intercept": { "type": "number" },
        "rescale_slope": { "type": "number" },
        "photometric_interpretation": { "type": "string" }
      }
    },
    "dicom:mr": {
      "type": "object",
      "properties": {
        "scanning_sequence": { "type": "string" },
        "sequence_variant": { "type": "string" },
        "scan_options": { "type": "string" },
        "mr_acquisition_type": { "type": "string" },
        "repetition_time": { "type": "number" },
        "echo_time": { "type": "number" },
        "echo_train_length": { "type": "integer" },
        "inversion_time": { "type": "number" },
        "trigger_time": { "type": "number" },
        "flip_angle": { "type": "number" },
        "spacing_between_slices": { "type": "number" },
        "number_of_averages": { "type": "number" },
        "imaging_frequency": { "type": "number" },
        "imaged_nucleus": { "type": "string" },
        "magnetic_field_strength": { "type": "number" },
        "sar": { "type": "number" },
        "db_dt": { "type": "number" },
        "acquisition_matrix": {
          "type": "array",
          "items": { "type": "integer" },
          "minItems": 4,
          "maxItems": 4
        },
        "phase_encoding_direction": { "type": "string" },
        "pixel_bandwidth": { "type": "number" }
      }
    },
    "dicom:ct": {
      "type": "object",
      "properties": {
        "kvp": { "type": "number" },
        "tube_current": { "type": "number" },
        "exposure_time": { "type": "integer" },
        "exposure": { "type": "number" },
        "filter_type": { "type": "string" },
        "convolution_kernel": { "type": "string" },
        "focal_spot": { "type": "number" },
        "rotation_direction": { "type": "string" },
        "exposure_modulation_type": { "type": "string" },
        "estimated_dose_saving": { "type": "number" },
        "ctdi_vol": { "type": "number" },
        "ct_dose_length_product": { "type": "number" },
        "revolution_time": { "type": "number" },
        "single_collimation_width": { "type": "number" },
        "total_collimation_width": { "type": "number" },
        "table_height": { "type": "number" },
        "gantry_detector_tilt": { "type": "number" },
        "table_feed_per_rotation": { "type": "number" },
        "spiral_pitch_factor": { "type": "number" },
        "data_collection_diameter": { "type": "number" },
        "reconstruction_diameter": { "type": "number" },
        "distance_source_to_detector": { "type": "number" },
        "distance_source_to_patient": { "type": "number" }
      }
    },
    "dicom:tag": {
      "type": "object",
      "patternProperties": {
        "^[0-9A-Fa-f]{8}$": {}
      },
      "additionalProperties": false
    },
    "dicom:private_tag": {
      "type": "object",
      "patternProperties": {
        "^[0-9A-Fa-f]{8}$": {
          "type": "object",
          "properties": {
            "creator": { "type": "string" },
            "value": {}
          },
          "required": ["creator", "value"]
        }
      },
      "additionalProperties": false
    }
  }
}
```