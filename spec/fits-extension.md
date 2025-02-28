# JNRRD FITS Extension Specification

Version: 1.0.0

## 1. Introduction

The JNRRD FITS Extension defines a standardized approach for incorporating FITS (Flexible Image Transport System) metadata and capabilities into JNRRD files. This extension enables JNRRD to maintain compatibility with astronomical data workflows while benefiting from JNRRD's JSON-based structure and binary data handling.

FITS is the primary data format used in astronomy and has been a standard since 1981. It provides a rich metadata system through hierarchical "Header Data Units" (HDUs) with standardized keywords and conventions. This extension bridges the gap between JNRRD and the astronomical community.

## 2. Extension Declaration

To use this extension, a JNRRD file must include the following extension declaration:

```json
{"extensions": {"fits": "https://jnrrd.org/extensions/fits/v1.0.0"}}
```

## 3. Core Concepts

### 3.1 Header Data Units (HDUs)

FITS files can contain multiple datasets, each with its own header and data section, called Header Data Units (HDUs):

- **Primary HDU**: The first HDU, which must be present in every FITS file
- **Extension HDUs**: Additional HDUs that follow the primary HDU
- **Types of HDUs**: IMAGE (array data), TABLE (ASCII table), BINTABLE (binary table)

In JNRRD FITS extension, HDUs are represented as a hierarchical structure, with the primary HDU being the default and extension HDUs accessible through indices.

### 3.2 World Coordinate System (WCS)

Astronomical data relies heavily on coordinate transformations to map pixel coordinates to world coordinates (e.g., celestial coordinates):

- **WCS Keywords**: Define how pixels map to physical coordinates
- **Coordinate Systems**: Celestial coordinates, spectral coordinates, temporal coordinates
- **Projections**: Methods for mapping the celestial sphere to a 2D plane

### 3.3 Special Data Types

FITS supports various data types and structures specific to astronomy:

- **Complex Data**: Real and imaginary components
- **Multi-dimensional Data**: Spectral cubes, time series, polarization data
- **Tables**: ASCII and Binary tables for catalogs and other tabular data

## 4. FITS Extension Fields

### 4.1 Basic Information

```json
{"fits:simple": true}
{"fits:bitpix": -32}
{"fits:naxis": 2}
{"fits:extend": true}
```

| Field | Type | Description |
|-------|------|-------------|
| `simple` | boolean | True if file conforms to FITS standard |
| `bitpix` | integer | Bits per pixel (8, 16, 32, -32, -64 for respective types) |
| `naxis` | integer | Number of axes |
| `extend` | boolean | True if file may contain extensions |

### 4.2 Primary Header

```json
{"fits:header": {
  "OBJECT": "M51",
  "TELESCOP": "Hubble Space Telescope",
  "INSTRUME": "WFC3",
  "OBSERVER": "J. Doe",
  "DATE-OBS": "2020-06-15T12:34:56",
  "EXPTIME": 1200.0,
  "FILTER": "F814W",
  "BUNIT": "counts/s",
  "BSCALE": 1.0,
  "BZERO": 0.0,
  "BLANK": -32768,
  "DATAMAX": 65535.0,
  "DATAMIN": 0.0
}}
```

| Field | Type | Description |
|-------|------|-------------|
| `header` | object | Key-value pairs for primary header |

Common header keywords include:
- `OBJECT`: Name of the observed object
- `TELESCOP`: Telescope used
- `INSTRUME`: Instrument used
- `OBSERVER`: Name of the observer
- `DATE-OBS`: Observation date and time
- `EXPTIME`: Exposure time in seconds
- `FILTER`: Filter used
- `BUNIT`: Physical units of the array values
- `BSCALE`: Scale factor for array values
- `BZERO`: Zero point for array values
- `BLANK`: Value used for undefined array elements
- `DATAMAX`: Maximum data value
- `DATAMIN`: Minimum data value

### 4.3 World Coordinate System (WCS)

```json
{"fits:wcs": {
  "CRPIX1": 512.0,
  "CRPIX2": 512.0,
  "CRVAL1": 202.4842,
  "CRVAL2": 47.1953,
  "CTYPE1": "RA---TAN",
  "CTYPE2": "DEC--TAN",
  "CD1_1": -0.000099,
  "CD1_2": 0.0,
  "CD2_1": 0.0,
  "CD2_2": 0.000099,
  "RADESYS": "FK5",
  "EQUINOX": 2000.0
}}
```

| Field | Type | Description |
|-------|------|-------------|
| `wcs` | object | World Coordinate System parameters |

Key WCS parameters include:
- `CRPIXn`: Reference pixel coordinate for axis n
- `CRVALn`: World coordinate value at reference pixel for axis n
- `CTYPEn`: Type of coordinate on axis n (e.g., "RA---TAN")
- `CDi_j`: Transformation matrix element
- `RADESYS`: Reference frame for celestial coordinates
- `EQUINOX`: Equinox of celestial coordinate system

### 4.4 Extensions

FITS files can contain multiple extensions, each with their own header and data. In JNRRD, these are represented as an array:

```json
{"fits:extensions": [
  {
    "xtension": "IMAGE",
    "bitpix": -32,
    "naxis": 2,
    "naxis1": 1024,
    "naxis2": 1024,
    "pcount": 0,
    "gcount": 1,
    "extname": "ERROR",
    "header": {
      "BUNIT": "counts/s",
      "CRPIX1": 512.0,
      "CRPIX2": 512.0
    }
  },
  {
    "xtension": "BINTABLE",
    "bitpix": 8,
    "naxis": 2,
    "naxis1": 32,
    "naxis2": 100,
    "pcount": 0,
    "gcount": 1,
    "extname": "CATALOG",
    "tfields": 4,
    "header": {
      "TFORM1": "1E",
      "TTYPE1": "X_COORD",
      "TUNIT1": "pixel",
      "TFORM2": "1E",
      "TTYPE2": "Y_COORD",
      "TUNIT2": "pixel",
      "TFORM3": "1E",
      "TTYPE3": "FLUX",
      "TUNIT3": "counts/s",
      "TFORM4": "20A",
      "TTYPE4": "NAME",
      "TUNIT4": ""
    }
  }
]}
```

| Field | Type | Description |
|-------|------|-------------|
| `extensions` | array | Array of extension objects |

Each extension object contains:
- `xtension`: Type of extension ("IMAGE", "TABLE", or "BINTABLE")
- `bitpix`: Bits per pixel for this extension
- `naxis`: Number of axes
- `naxisN`: Size of axis N
- `pcount`: Parameter count (usually 0)
- `gcount`: Group count (usually 1)
- `extname`: Extension name
- `header`: Additional header keywords specific to this extension

For BINTABLE extensions, additional fields are:
- `tfields`: Number of fields in the table
- `tformN`: Format of field N
- `ttypeN`: Name of field N
- `tunitN`: Units of field N

### 4.5 Hierarchical Grouping Convention

The FITS Hierarchical Grouping Convention is supported through the following structure:

```json
{"fits:groups": {
  "HIERARCH.ESO.TEL.AMBI.WINDDIR": 235.0,
  "HIERARCH.ESO.TEL.AMBI.WINDSP": 2.5,
  "HIERARCH.ESO.INS.MODE": "IMAGING",
  "HIERARCH.ESO.INS.FILT1.ID": "F814W"
}}
```

| Field | Type | Description |
|-------|------|-------------|
| `groups` | object | Hierarchical keywords using the HIERARCH convention |

### 4.6 Data Storage and Binary Table Representation

For binary tables (BINTABLE extensions), additional information about the data structure is needed:

```json
{"fits:table_data": {
  "structure": [
    {"name": "X_COORD", "format": "1E", "unit": "pixel"},
    {"name": "Y_COORD", "format": "1E", "unit": "pixel"},
    {"name": "FLUX", "format": "1E", "unit": "counts/s"},
    {"name": "NAME", "format": "20A", "unit": ""}
  ],
  "rows": 100
}}
```

| Field | Type | Description |
|-------|------|-------------|
| `table_data.structure` | array | Column descriptions for binary tables |
| `table_data.rows` | integer | Number of rows in the table |

### 4.7 Keyword Comments and History

FITS headers often include comments and history records, which can be represented as:

```json
{"fits:comments": [
  "This is a calibrated image of M51",
  "Calibration performed using standard stars HD123456"
]}
{"fits:history": [
  "2020-06-15: Observed with HST/WFC3",
  "2020-06-16: Processed with calwf3 v3.5.2",
  "2020-06-17: Drizzled with AstroDrizzle v2.3.1"
]}
```

| Field | Type | Description |
|-------|------|-------------|
| `comments` | array | Comment records from the FITS header |
| `history` | array | History records from the FITS header |

## 5. Storage of Multi-HDU Data

FITS files often contain multiple HDUs with different data arrays. In JNRRD, this can be handled in two ways:

### 5.1 Single JNRRD File with Multiple HDUs

For FITS files with extensions that are fundamentally related to the primary data:

```json
{"fits:data_storage": "embedded"}
```

In this case, all data is stored in the JNRRD binary section in concatenated form, with offsets to each HDU provided in the metadata.

### 5.2 Detached HDU Data

For FITS files with large or numerous extensions:

```json
{"fits:data_storage": "detached"}
{"fits:primary_data_file": "primary.dat"}
{"fits:extension_data_files": ["ext1.dat", "ext2.dat"]}
```

Here, the primary HDU and each extension's data is stored in separate files referenced by the JNRRD header.

## 6. Data Consistency Requirements

When using the FITS extension with JNRRD, the file writer must ensure consistency between the core JNRRD fields and their corresponding FITS extension fields. Specifically:

1. **Dimensions and Sizes**: The JNRRD `dimension` and `sizes` fields must match the dimensionality specified in the FITS `naxis` and corresponding axis length fields.

2. **Coordinate Systems**: The JNRRD `space_directions` and `space_origin` fields should be consistent with the FITS WCS information when applicable.

3. **Data Type**: The JNRRD `type` field should correspond to the appropriate FITS `bitpix` value.

4. **Scaling**: Any scaling applied to the data must be consistently represented in both JNRRD and FITS metadata (e.g., BSCALE/BZERO).

It is the file writer's responsibility to maintain this consistency. Readers may use either the JNRRD core fields or the FITS extension fields for interpretation, but should expect them to represent the same underlying data structure.

## 7. Integration with JNRRD Space

Astronomical coordinates can be linked to the JNRRD space fields:

```json
{"fits:space_mapping": {
  "jnrrd_space": "right_anterior_superior",
  "fits_coordinates": "celestial_ra_dec"
}}
```

This field defines how the JNRRD coordinate system maps to the FITS WCS coordinates.

## 8. Examples

### 8.1 Simple Astronomical Image

```
{"jnrrd": "0004"}
{"type": "float32"}
{"dimension": 2}
{"sizes": [1024, 1024]}
{"endian": "little"}
{"encoding": "gzip"}
{"extensions": {"fits": "https://jnrrd.org/extensions/fits/v1.0.0"}}
{"fits:simple": true}
{"fits:bitpix": -32}
{"fits:naxis": 2}
{"fits:header": {
  "OBJECT": "M51",
  "TELESCOP": "HST",
  "INSTRUME": "WFC3",
  "DATE-OBS": "2020-06-15T12:34:56",
  "EXPTIME": 1200.0,
  "FILTER": "F814W",
  "BUNIT": "counts/s"
}}
{"fits:wcs": {
  "CRPIX1": 512.0,
  "CRPIX2": 512.0,
  "CRVAL1": 202.4842,
  "CRVAL2": 47.1953,
  "CTYPE1": "RA---TAN",
  "CTYPE2": "DEC--TAN",
  "CD1_1": -0.000099,
  "CD1_2": 0.0,
  "CD2_1": 0.0,
  "CD2_2": 0.000099
}}
{"fits:comments": ["This is a calibrated image of M51"]}
{"fits:history": ["2020-06-15: Observed with HST/WFC3"]}

[BINARY DATA FOLLOWS]
```

### 8.2 Multi-HDU Example (Image + Error + Catalog)

```
{"jnrrd": "0004"}
{"type": "float32"}
{"dimension": 2}
{"sizes": [1024, 1024]}
{"endian": "little"}
{"encoding": "gzip"}
{"extensions": {"fits": "https://jnrrd.org/extensions/fits/v1.0.0"}}
{"fits:simple": true}
{"fits:bitpix": -32}
{"fits:naxis": 2}
{"fits:extend": true}
{"fits:header": {
  "OBJECT": "M101",
  "TELESCOP": "HST",
  "INSTRUME": "ACS",
  "DATE-OBS": "2021-03-10T08:23:45",
  "EXPTIME": 1800.0
}}
{"fits:wcs": {
  "CRPIX1": 512.0,
  "CRPIX2": 512.0,
  "CRVAL1": 210.8025,
  "CRVAL2": 54.3492,
  "CTYPE1": "RA---TAN",
  "CTYPE2": "DEC--TAN"
}}
{"fits:extensions": [
  {
    "xtension": "IMAGE",
    "bitpix": -32,
    "naxis": 2,
    "naxis1": 1024,
    "naxis2": 1024,
    "extname": "ERROR",
    "header": {
      "BUNIT": "counts/s"
    }
  },
  {
    "xtension": "BINTABLE",
    "bitpix": 8,
    "naxis": 2,
    "naxis1": 32,
    "naxis2": 150,
    "extname": "CATALOG",
    "tfields": 4,
    "header": {
      "TFORM1": "1E",
      "TTYPE1": "X_COORD",
      "TFORM2": "1E",
      "TTYPE2": "Y_COORD",
      "TFORM3": "1E",
      "TTYPE3": "FLUX",
      "TFORM4": "20A",
      "TTYPE4": "NAME"
    }
  }
]}
{"fits:data_storage": "embedded"}
{"fits:comments": ["Science image with error array and catalog"]}

[BINARY DATA FOLLOWS]
```

### 8.3 Hierarchical Notation Example

```
{"jnrrd": "0004"}
{"type": "float32"}
{"dimension": 2}
{"sizes": [2048, 2048]}
{"endian": "little"}
{"encoding": "gzip"}
{"extensions": {"fits": "https://jnrrd.org/extensions/fits/v1.0.0"}}
{"fits:simple": true}
{"fits:bitpix": -32}
{"fits:naxis": 2}
{"fits:header.OBJECT": "NGC 3372"}
{"fits:header.TELESCOP": "ESO VLT"}
{"fits:header.INSTRUME": "MUSE"}
{"fits:header.DATE-OBS": "2022-01-15T02:45:18"}
{"fits:header.EXPTIME": 900.0}
{"fits:header.FILTER": "None"}
{"fits:header.BUNIT": "1e-20 erg/s/cm^2/A"}
{"fits:wcs.CRPIX1": 1024.0}
{"fits:wcs.CRPIX2": 1024.0}
{"fits:wcs.CRVAL1": 161.2650}
{"fits:wcs.CRVAL2": -59.6827}
{"fits:wcs.CTYPE1": "RA---TAN"}
{"fits:wcs.CTYPE2": "DEC--TAN"}
{"fits:wcs.CD1_1": -1.38e-5}
{"fits:wcs.CD2_2": 1.38e-5}
{"fits:groups.HIERARCH.ESO.TEL.AMBI.WINDDIR": 235.0}
{"fits:groups.HIERARCH.ESO.TEL.AMBI.WINDSP": 2.5}
{"fits:groups.HIERARCH.ESO.INS.MODE": "WFM"}
{"fits:comments[0]": "MUSE datacube collapsed along wavelength axis"}
{"fits:comments[1]": "Shows H-alpha emission in the Carina Nebula"}
{"fits:history[0]": "2022-01-16: Processed with MUSE pipeline v2.8.3"}
{"fits:history[1]": "2022-01-17: Flux calibrated using standard star CD-32 9927"}

[BINARY DATA FOLLOWS]
```

### 8.4 Spectral Cube Example with Consistency Notes

```
{"jnrrd": "0004"}
{"type": "float32"}          // Corresponds to fits:bitpix: -32
{"dimension": 3}             // Matches fits:naxis: 3
{"sizes": [256, 256, 3600]}  // Dimensions match FITS structure
{"endian": "little"}         // FITS typically uses big-endian, this is explicitly converted
{"encoding": "zstd"}
{"space": "right_anterior_superior"}  // Astronomical coordinates
{"space_directions": [       // Derived from the WCS CDELT values
  [-0.000083, 0.0, 0.0],     // CDELT1 = -0.000083 degrees
  [0.0, 0.000083, 0.0],      // CDELT2 = 0.000083 degrees
  [0.0, 0.0, 1.0e6]          // CDELT3 = 1.0e6 Hz
]}
{"space_origin": [49.9509, 41.5118, 115.271e9]}  // From CRVAL values
{"extensions": {"fits": "https://jnrrd.org/extensions/fits/v1.0.0"}}
{"fits:simple": true}
{"fits:bitpix": -32}         // Corresponds to JNRRD type: float32
{"fits:naxis": 3}            // Matches JNRRD dimension: 3
{"fits:header": {
  "OBJECT": "NGC 1275",
  "TELESCOP": "ALMA",
  "INSTRUME": "ALMA",
  "DATE-OBS": "2020-10-25T14:32:56",
  "BUNIT": "Jy/beam"
}}
{"fits:wcs": {
  "CRPIX1": 128.0,           // Reference pixel for axis 1
  "CRPIX2": 128.0,           // Reference pixel for axis 2
  "CRPIX3": 1800.0,          // Reference pixel for axis 3
  "CRVAL1": 49.9509,         // Corresponds to space_origin[0]
  "CRVAL2": 41.5118,         // Corresponds to space_origin[1]
  "CRVAL3": 115.271e9,       // Corresponds to space_origin[2]
  "CTYPE1": "RA---SIN",      // RA with orthographic projection
  "CTYPE2": "DEC--SIN",      // DEC with orthographic projection
  "CTYPE3": "FREQ",          // Frequency axis
  "CUNIT3": "Hz",            // Units for axis 3
  "CDELT1": -0.000083,       // Corresponds to space_directions[0][0]
  "CDELT2": 0.000083,        // Corresponds to space_directions[1][1]
  "CDELT3": 1.0e6,           // Corresponds to space_directions[2][2]
  "SPECSYS": "LSRK",         // Spectral reference frame
  "RESTFRQ": 115.271e9       // Rest frequency of spectral line
}}
{"fits:space_mapping": {
  "jnrrd_space": "right_anterior_superior",
  "fits_coordinates": "celestial_ra_dec"
}}
{"fits:comments": ["ALMA spectral cube of CO emission in NGC 1275"]}

[BINARY DATA FOLLOWS]
```

Note the careful alignment between the JNRRD core fields and the FITS extension fields:
1. The `type: float32` corresponds to `fits:bitpix: -32`
2. The `dimension: 3` matches `fits:naxis: 3`
3. The `sizes` array contains the correct dimensions
4. The `space_directions` values align with the CDELT values in the WCS
5. The `space_origin` values match the CRVAL values in the WCS

## 9. Converting Between FITS and JNRRD

### 9.1 Converting FITS to JNRRD+FITS Extension

When converting from a FITS file to JNRRD with the FITS extension:

1. Extract header information from the FITS primary HDU
2. Map FITS header keywords to corresponding JNRRD FITS extension fields
3. Process WCS information into the `fits:wcs` object
4. Handle COMMENT and HISTORY records
5. Process any extension HDUs
6. Set appropriate JNRRD core fields (type, dimension, sizes)
7. Extract the binary data and store with appropriate encoding

### 9.2 Converting JNRRD+FITS Extension to FITS

When converting from JNRRD with FITS extension to a FITS file:

1. Create a new FITS primary header
2. Map JNRRD FITS extension fields to FITS header keywords
3. Create appropriate WCS headers from the `fits:wcs` object
4. Add any COMMENT and HISTORY records
5. Create extension HDUs as needed
6. Copy the binary data, applying any needed byte order transformations

## 10. Implementation Notes

1. **Keyword Handling**: FITS has strict requirements for keyword length (8 characters) and value formatting
2. **Hierarch Convention**: For keywords longer than 8 characters, use the HIERARCH convention
3. **Endianness**: FITS is typically stored in big-endian byte order, regardless of the machine architecture
4. **Missing Values**: In FITS, undefined floating-point values are often represented by NaN
5. **String Length**: FITS character strings are padded to a fixed length with spaces
6. **Binary Tables**: FITS binary tables have specific format requirements that need special handling

## 11. Compatibility with JNRRD Extensions

The FITS extension can be used together with other JNRRD extensions:

- **JNRRD Tiling Extension**: Enables efficient access to large astronomical datasets
- **OME Extension**: Useful for multi-modal astronomical data with microscopy components

## 12. JSON Schema

The following JSON Schema can be used to validate the FITS extension fields:

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "JNRRD FITS Extension Schema",
  "description": "Schema for validating JNRRD FITS extension fields",
  "type": "object",
  "properties": {
    "simple": {
      "type": "boolean",
      "description": "True if file conforms to FITS standard (stored as 'fits:simple' in JNRRD files)"
    },
    "bitpix": {
      "type": "integer",
      "description": "Bits per pixel (stored as 'fits:bitpix' in JNRRD files)",
      "enum": [8, 16, 32, 64, -32, -64]
    },
    "naxis": {
      "type": "integer",
      "description": "Number of axes (stored as 'fits:naxis' in JNRRD files)",
      "minimum": 0
    },
    "extend": {
      "type": "boolean",
      "description": "True if file may contain extensions (stored as 'fits:extend' in JNRRD files)"
    },
    "header": {
      "type": "object",
      "description": "Primary header keywords (stored as 'fits:header' in JNRRD files)",
      "additionalProperties": true
    },
    "wcs": {
      "type": "object",
      "description": "World Coordinate System parameters (stored as 'fits:wcs' in JNRRD files)",
      "additionalProperties": true
    },
    "extensions": {
      "type": "array",
      "description": "Extension HDUs (stored as 'fits:extensions' in JNRRD files)",
      "items": {
        "type": "object",
        "properties": {
          "xtension": {
            "type": "string",
            "description": "Extension type",
            "enum": ["IMAGE", "TABLE", "BINTABLE"]
          },
          "bitpix": {
            "type": "integer",
            "description": "Bits per pixel",
            "enum": [8, 16, 32, 64, -32, -64]
          },
          "naxis": {
            "type": "integer",
            "description": "Number of axes",
            "minimum": 0
          },
          "pcount": {
            "type": "integer",
            "description": "Parameter count",
            "default": 0
          },
          "gcount": {
            "type": "integer",
            "description": "Group count",
            "default": 1
          },
          "extname": {
            "type": "string",
            "description": "Extension name"
          },
          "header": {
            "type": "object",
            "description": "Extension header keywords",
            "additionalProperties": true
          }
        },
        "required": ["xtension", "bitpix", "naxis"]
      }
    },
    "groups": {
      "type": "object",
      "description": "Hierarchical keywords (stored as 'fits:groups' in JNRRD files)",
      "additionalProperties": true
    },
    "table_data": {
      "type": "object",
      "description": "Binary table structure (stored as 'fits:table_data' in JNRRD files)",
      "properties": {
        "structure": {
          "type": "array",
          "description": "Column descriptions",
          "items": {
            "type": "object",
            "properties": {
              "name": {
                "type": "string",
                "description": "Column name"
              },
              "format": {
                "type": "string",
                "description": "Column format"
              },
              "unit": {
                "type": "string",
                "description": "Column unit"
              }
            },
            "required": ["name", "format"]
          }
        },
        "rows": {
          "type": "integer",
          "description": "Number of rows",
          "minimum": 0
        }
      },
      "required": ["structure", "rows"]
    },
    "comments": {
      "type": "array",
      "description": "Comment records (stored as 'fits:comments' in JNRRD files)",
      "items": {
        "type": "string"
      }
    },
    "history": {
      "type": "array",
      "description": "History records (stored as 'fits:history' in JNRRD files)",
      "items": {
        "type": "string"
      }
    },
    "data_storage": {
      "type": "string",
      "description": "How multi-HDU data is stored (stored as 'fits:data_storage' in JNRRD files)",
      "enum": ["embedded", "detached"]
    },
    "primary_data_file": {
      "type": "string",
      "description": "File containing primary HDU data (stored as 'fits:primary_data_file' in JNRRD files)"
    },
    "extension_data_files": {
      "type": "array",
      "description": "Files containing extension HDU data (stored as 'fits:extension_data_files' in JNRRD files)",
      "items": {
        "type": "string"
      }
    },
    "space_mapping": {
      "type": "object",
      "description": "Mapping between JNRRD and FITS spaces (stored as 'fits:space_mapping' in JNRRD files)",
      "properties": {
        "jnrrd_space": {
          "type": "string",
          "description": "JNRRD space identifier"
        },
        "fits_coordinates": {
          "type": "string",
          "description": "FITS coordinate system identifier",
          "enum": [
            "celestial_ra_dec",
            "galactic",
            "ecliptic",
            "spectral",
            "temporal"
          ]
        }
      },
      "required": ["jnrrd_space", "fits_coordinates"]
    }
  }
}
```