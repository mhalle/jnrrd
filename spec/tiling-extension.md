# JNRRD Tiling Extension Specification

Version: 1.0.0

## 1. Introduction

The JNRRD Tiling Extension defines a standardized approach for organizing large datasets into smaller, regularly-sized chunks (tiles or bricks) that can be accessed independently. This extension enables efficient spatial random access, progressive loading, parallel processing, and memory-efficient operations on large multi-dimensional datasets.

## 2. Extension Declaration

To use this extension, a JNRRD file must include the following extension declaration:

```json
{"extensions": {"tile": "https://jnrrd.org/extensions/tile/v1.0.0"}}
```

## 3. Core Concepts

### 3.1 Tiles/Bricks

A tile (or brick in 3D) is a contiguous, rectangular subset of the complete dataset. Tiles are indexed by their position in the dataset's grid system.

### 3.2 Storage Models

The extension supports two storage models:

1. **Internal tiling**: All tiles are stored within the same JNRRD file
2. **External tiling**: Tiles are stored in separate files, with the main JNRRD file acting as an index

## 4. Required Fields

The following fields are required when using the tiling extension:

| Field | Type | Description |
|-------|------|-------------|
| `tile:enabled` | boolean | Must be set to `true` to enable tiling |
| `tile:dimensions` | array | Array of dimension indices that are tiled |
| `tile:sizes` | array | Size of each tile along each dimension in `tile:dimensions` |
| `tile:storage` | string | Either "internal" or "external" |

Example:
```json
{"tile:enabled": true}
{"tile:dimensions": [0, 1, 2]}
{"tile:sizes": [256, 256, 64]}
{"tile:storage": "internal"}
```

## 5. Internal Tiling

When `tile:storage` is set to "internal", the following additional fields are available:

| Field | Type | Description |
|-------|------|-------------|
| `tile:format` | string | Organization of tiles in file, either "contiguous" or "chunked" |
| `tile:offset_table` | array | Byte offsets for the start of each tile in the file |
| `tile:size_table` | array (optional) | Size in bytes of each tile (required for variable-sized tiles) |

Example:
```json
{"tile:format": "contiguous"}
{"tile:offset_table": [1024, 16778240, 33555456, 50332672, ...]}
```

### 5.1 Tile Ordering

Tiles are indexed in lexicographical (row-major) order, with dimension 0 varying fastest:

For a 3D volume with dimensions [2,2,2]:
- Tile [0,0,0] = Index 0
- Tile [1,0,0] = Index 1
- Tile [0,1,0] = Index 2
- Tile [1,1,0] = Index 3
- Tile [0,0,1] = Index 4
- etc.

### 5.2 Tile Formats

- **Contiguous**: Tiles are stored one after another in the file
- **Chunked**: Tiles can be stored in any order, with their location specified in the offset table

## 6. External Tiling

When `tile:storage` is set to "external", the following fields specify how to locate tile data:

| Field | Type | Description |
|-------|------|-------------|
| `tile:pattern` | string | Pattern for generating filenames, with placeholders |
| `tile:files` | array (alternative) | Explicit list of files for each tile |
| `tile:base_dir` | string (optional) | Base directory for relative file paths |

### 6.1 Pattern-based File Naming

The pattern uses placeholders like `{x}`, `{y}`, `{z}` for coordinates and `{i}` for the linear index of the tile:

```json
{"tile:pattern": "data_{z}_{y}_{x}.raw"}
```

With full file path examples:

```json
{"tile:pattern": "/data/project123/slices/slice_{z}/data_{y}_{x}.raw"}
```

Using a base directory:

```json
{"tile:base_dir": "/data/project123/"}
{"tile:pattern": "slices/slice_{z}/data_{y}_{x}.raw"}
```

Remote HTTP access:

```json
{"tile:pattern": "https://data.example.org/volumes/brain109/tiles/{z}_{y}_{x}.raw"}
```

Cloud storage access:

```json
{"tile:pattern": "s3://medical-imaging/patient4563/ct-scan/tiles/z{z}_y{y}_x{x}.raw.gz"}
```

### 6.2 Explicit File Listing

Alternatively, files can be listed explicitly:

```json
{"tile:files": [
  {"indices": [0,0,0], "file": "tile_0_0_0.raw"},
  {"indices": [1,0,0], "file": "tile_1_0_0.raw"},
  {"indices": [0,1,0], "file": "tile_0_1_0.raw"},
  ...
]}
```

With mixed local, remote, and cloud storage locations:

```json
{"tile:files": [
  {"indices": [0,0,0], "file": "/local/cache/tile_0_0_0.raw"},
  {"indices": [1,0,0], "file": "https://remote.server.org/tiles/tile_1_0_0.raw"},
  {"indices": [0,1,0], "file": "s3://bucket/tiles/tile_0_1_0.raw"},
  {"indices": [1,1,0], "file": "gs://another-bucket/tiles/tile_1_1_0.raw"},
  {"indices": [0,0,1], "file": "azure://container/tiles/tile_0_0_1.raw"}
]}
```

The format supports the use of mixed storage locations, allowing some tiles to be stored locally and others remotely, which is useful for caching frequently accessed tiles.

## 7. Optional Features

### 7.1 Edge Handling

For datasets whose dimensions are not multiples of tile sizes:

| Field | Type | Description |
|-------|------|-------------|
| `tile:edge_handling` | string | Either "pad" or "variable" (default: "pad") |
| `tile:padding_value` | number (optional) | Value used for padding (default: 0) |

- **pad**: All tiles have the same size, with padding added to edge tiles
- **variable**: Edge tiles are smaller than regular tiles

### 7.2 Tile Overlap

Tiles can optionally overlap to support operations that require neighborhood context:

```json
{"tile:overlap": [8, 8, 0]}  // 8-voxel overlap in dimensions 0 and 1
```

### 7.3 Per-Tile Compression

Compression can be applied on a per-tile basis:

```json
{"tile:compression": "gzip"}  // Compression applied to all tiles
{"tile:compression_levels": [9, 6, 6, 4, ...]}  // Optional per-tile compression levels
```

Valid compression values: "raw", "gzip", "bzip2", "zstd", "lz4"

### 7.4 Multi-Resolution Support

For visualization and progressive access, multiple resolution levels can be stored, creating a pyramid structure where each level represents the data at a different resolution.

#### 7.4.1 Core Multi-Resolution Fields

```json
{"tile:levels": 4}  // Total number of resolution levels (including full resolution)
{"tile:level_scales": [1, 2, 4, 8]}  // Scale factor for each level
{"tile:level_offsets": [1024, 2147484672, 2415919104, 2483027968]}  // Byte offset to start of each level
```

Level 0 is always the full-resolution data, with higher level numbers representing progressively lower resolutions. For a 3D dataset, a scale factor of 2 reduces the volume by a factor of 8.

#### 7.4.2 Data Organization and Layout

For a volume with dimensions 2048×2048×512 and tiles of 256×256×64:

| Level | Dimensions | Scale | Tiles | Total Size |
|-------|------------|-------|-------|------------|
| 0 | 2048×2048×512 | 1 | 8×8×8 = 512 | 2GB |
| 1 | 1024×1024×256 | 2 | 4×4×4 = 64 | 256MB |
| 2 | 512×512×128 | 4 | 2×2×2 = 8 | 32MB |
| 3 | 256×256×64 | 8 | 1×1×1 = 1 | 4MB |

In a file with internal tiling using contiguous format, tiles would be organized sequentially:
```
[Header]
[Level 0 Tiles: 512 tiles in sequence]
[Level 1 Tiles: 64 tiles in sequence]
[Level 2 Tiles: 8 tiles in sequence]
[Level 3 Tiles: 1 tile]
```

The `tile:level_offsets` array points to the beginning of each level in the file.

#### 7.4.3 Tile Indexing with Multi-Resolution

To locate a specific tile at a specific resolution level:

1. Calculate the dimensions of the dataset at the given level
   ```
   level_dims = [dim // scale for dim, scale in zip(original_dims, level_scales[level])]
   ```

2. Calculate the tile grid dimensions for this level
   ```
   tile_grid_dims = [math.ceil(dim / tile_size) for dim, tile_size in zip(level_dims, tile_sizes)]
   ```

3. Calculate the linear index within this level
   ```
   level_index = (z * tile_grid_y * tile_grid_x) + (y * tile_grid_x) + x
   ```

4. Calculate the global index by adding offsets from all previous levels
   ```
   global_index = sum(tile_counts_for_all_previous_levels) + level_index
   ```

5. Look up the offset in the offset table
   ```
   offset = offset_table[global_index]
   ```

#### 7.4.4 Downsampling Methods

The method used to generate lower-resolution levels can be specified:

```json
{"tile:downsample_method": "gaussian"}  // Method used for generating lower resolution levels
```

Common downsampling methods include:

| Method | Description |
|--------|-------------|
| `average` | Simple averaging of voxels (default) |
| `gaussian` | Gaussian filtering before downsampling (reduces aliasing) |
| `lanczos` | Lanczos filtering (high quality but computationally expensive) |
| `max` | Maximum value (useful for segmentation data) |
| `min` | Minimum value |
| `mode` | Most common value (useful for label data) |

#### 7.4.5 Advanced Multi-Resolution Features

**Different Tile Sizes Per Level**:
```json
{"tile:level_tile_sizes": [
  [256, 256, 64],  // Level 0
  [128, 128, 32],  // Level 1
  [64, 64, 16],    // Level 2
  [32, 32, 8]      // Level 3
]}
```

**Anisotropic Downsampling**:
```json
{"tile:level_scales": [
  [1, 1, 1],      // Level 0
  [2, 2, 1],      // Level 1 - don't downsample Z dimension
  [4, 4, 2],      // Level 2
  [8, 8, 4]       // Level 3
]}
```

**On-Demand Resolution Generation**:
```json
{"tile:levels_stored": [0, 2]}  // Only store levels 0 and 2
{"tile:levels_virtual": [1, 3]}  // Generate levels 1 and 3 on demand
```

**Quality Metadata**:
```json
{"tile:level_quality": [
  {"psnr": 100.0, "rmse": 0.0},    // Level 0
  {"psnr": 42.3, "rmse": 0.87},    // Level 1
  {"psnr": 36.7, "rmse": 1.43},    // Level 2
  {"psnr": 31.2, "rmse": 2.16}     // Level 3
]}
```

#### 7.4.6 External Multi-Resolution Tiling

For external tiling, multi-resolution is supported using patterns with level placeholders:

```json
{"tile:pattern": "tiles/level_{l}/volume_{z}_{y}_{x}.raw"}
```

With remote storage examples:

```json
{"tile:pattern": "https://data.example.org/datasets/brain/levels/{l}/tiles_{z}_{y}_{x}.raw"}
```

```json
{"tile:pattern": "s3://neuroimaging-archive/patient0042/pyramid/res{l}/{z}/{y}/{x}.raw.gz"}
```

```json
{"tile:base_dir": "https://cdn.imaging-platform.org/public-datasets/"}
{"tile:pattern": "mouse-brain-atlas/version2/resolution_{l}/{z}_{y}_{x}.zstd"}
```

Or explicit file listings with mixed sources:

```json
{"tile:files": [
  {"level": 0, "indices": [0,0,0], "file": "/local/cache/hires/0_0_0.raw"},
  {"level": 0, "indices": [1,0,0], "file": "https://remote.server.org/hires/1_0_0.raw"},
  {"level": 1, "indices": [0,0,0], "file": "s3://bucket/midres/0_0_0.raw"},
  {"level": 2, "indices": [0,0,0], "file": "gs://public-bucket/lowres/0_0_0.raw"},
  ...
]}
```

This approach enables efficient multi-resolution workflows where:
- Low-resolution levels can be stored on faster, more accessible storage
- High-resolution data can be accessed from archival or high-capacity storage
- Frequently accessed tiles can be cached locally
- Visualization applications can quickly load overviews before fetching details

#### 7.4.7 Use Cases for Multi-Resolution Tiling

1. **Progressive Loading**: Display low-resolution data immediately while loading high-resolution tiles
2. **Level-of-Detail Rendering**: Use resolution based on distance from camera or screen-space error
3. **Memory-Efficient Analysis**: Perform initial analysis on lower resolutions, refine on higher resolutions
4. **Multi-Scale Feature Extraction**: Extract features at multiple scales for robust analysis
5. **Overview-and-Detail Visualization**: Show both overview (low-res) and detailed region (high-res)

### 7.5 Tile Metadata

Additional per-tile metadata can be stored:

```json
{"tile:metadata": [
  {"min": 0.1, "max": 255.0, "checksum": "a1b2c3d4"},
  {"min": 0.0, "max": 127.5, "checksum": "e5f6g7h8"},
  ...
]}
```

## 8. Reference Implementation

### 8.1 Calculating Tile Indices

```python
def get_tile_index(coordinates, tile_sizes, dataset_sizes):
    """Convert spatial coordinates to a tile index"""
    tile_coords = [coord // size for coord, size in zip(coordinates, tile_sizes)]
    
    # Calculate number of tiles in each dimension
    tiles_per_dim = [math.ceil(ds / ts) for ds, ts in zip(dataset_sizes, tile_sizes)]
    
    # Calculate linear index
    linear_index = 0
    multiplier = 1
    
    for i, coord in enumerate(tile_coords):
        linear_index += coord * multiplier
        multiplier *= tiles_per_dim[i]
        
    return linear_index
```

### 8.2 Reading a Tile

```python
def read_tile(file_path, tile_coords, header):
    """Read a specific tile from a JNRRD file with internal tiling"""
    # Calculate tile index
    tile_index = get_tile_index(
        tile_coords, 
        header["tile:sizes"], 
        header["sizes"]
    )
    
    # Get offset for this tile
    offset = header["tile:offset_table"][tile_index]
    
    # Read the tile data
    with open(file_path, 'rb') as f:
        f.seek(offset)
        
        # Get tile size
        if "tile:size_table" in header:
            # Variable-sized tiles
            size = header["tile:size_table"][tile_index]
        else:
            # Fixed-size tiles
            tile_voxels = np.prod(header["tile:sizes"])
            bytes_per_voxel = get_bytes_per_voxel(header["type"])
            size = tile_voxels * bytes_per_voxel
        
        # Read the data
        tile_data = f.read(size)
        
        # Handle compression if present
        if "tile:compression" in header:
            compression = header["tile:compression"]
            if compression == "gzip":
                import gzip
                tile_data = gzip.decompress(tile_data)
            # Handle other compression types
            
        # Convert to numpy array
        dtype = get_numpy_dtype(header["type"])
        tile_array = np.frombuffer(tile_data, dtype=dtype).reshape(header["tile:sizes"])
        
        return tile_array
```

## 9. Examples

### 9.1 Internal Tiling Example

```
{"jnrrd": "0004"}
{"type": "float32"}
{"dimension": 3}
{"sizes": [2048, 2048, 512]}
{"endian": "little"}
{"encoding": "raw"}
{"space": "right_anterior_superior"}
{"space_directions": [[0.5, 0.0, 0.0], [0.0, 0.5, 0.0], [0.0, 0.0, 1.2]]}
{"space_origin": [0.0, 0.0, 0.0]}
{"extensions": {"tile": "https://jnrrd.org/extensions/tile/v1.0.0"}}
{"tile:enabled": true}
{"tile:dimensions": [0, 1, 2]}
{"tile:sizes": [256, 256, 64]}
{"tile:storage": "internal"}
{"tile:format": "contiguous"}
{"tile:offset_table": [1024, 16778240, 33555456, 50332672, ...]}
{"tile:compression": "gzip"}

[BINARY DATA OF TILES IN SEQUENCE]
```

### 9.2 External Tiling Examples

#### 9.2.1 Local File System Example

```
{"jnrrd": "0004"}
{"type": "uint16"}
{"dimension": 3}
{"sizes": [4096, 4096, 1024]}
{"endian": "little"}
{"space": "right_anterior_superior"}
{"extensions": {"tile": "https://jnrrd.org/extensions/tile/v1.0.0"}}
{"tile:enabled": true}
{"tile:dimensions": [0, 1, 2]}
{"tile:sizes": [512, 512, 128]}
{"tile:storage": "external"}
{"tile:pattern": "tiles/volume_{z}_{y}_{x}.raw.gz"}
{"tile:base_dir": "/data/large_volume/"}
{"tile:compression": "gzip"}

[NO BINARY DATA - TILES STORED IN EXTERNAL FILES]
```

#### 9.2.2 Cloud Storage Example

```
{"jnrrd": "0004"}
{"type": "float32"}
{"dimension": 3}
{"sizes": [8192, 8192, 2048]}
{"endian": "little"}
{"space": "right_anterior_superior"}
{"extensions": {"tile": "https://jnrrd.org/extensions/tile/v1.0.0"}}
{"tile:enabled": true}
{"tile:dimensions": [0, 1, 2]}
{"tile:sizes": [512, 512, 128]}
{"tile:storage": "external"}
{"tile:pattern": "s3://neuroimaging-bucket/acquisitions/2023-05-12/subject0042/tiles/{z}/{y}/{x}.zstd"}
{"tile:compression": "zstd"}

[NO BINARY DATA - TILES STORED IN CLOUD STORAGE]
```

#### 9.2.3 HTTP CDN Example

```
{"jnrrd": "0004"}
{"type": "uint8"}
{"dimension": 3}
{"sizes": [16384, 16384, 4096]}
{"endian": "little"}
{"space": "right_anterior_superior"}
{"extensions": {"tile": "https://jnrrd.org/extensions/tile/v1.0.0"}}
{"extensions": {"dicom": "https://jnrrd.org/extensions/dicom/v1.0.0"}}
{"tile:enabled": true}
{"tile:dimensions": [0, 1, 2]}
{"tile:sizes": [256, 256, 64]}
{"tile:storage": "external"}
{"tile:pattern": "https://tiles.medical-imaging.example.org/studies/{study_id}/series/{series_id}/tiles/{z}/{y}/{x}.lz4"}
{"tile:compression": "lz4"}
{"dicom:study.instance_uid": "1.2.840.113619.2.334.3.2831183778.864.1629754903.547"}
{"dicom:series.instance_uid": "1.2.840.113619.2.334.3.2831183778.864.1629754903.550"}

[NO BINARY DATA - TILES STORED ON HTTP SERVER]
```

#### 9.2.4 Mixed Storage Locations Example

```
{"jnrrd": "0004"}
{"type": "uint16"}
{"dimension": 3}
{"sizes": [4096, 4096, 1024]}
{"endian": "little"}
{"space": "right_anterior_superior"}
{"extensions": {"tile": "https://jnrrd.org/extensions/tile/v1.0.0"}}
{"tile:enabled": true}
{"tile:dimensions": [0, 1, 2]}
{"tile:sizes": [256, 256, 64]}
{"tile:storage": "external"}
{"tile:files": [
  {"indices": [0,0,0], "file": "/local/cache/0_0_0.raw"},
  {"indices": [1,0,0], "file": "/local/cache/1_0_0.raw"},
  {"indices": [0,1,0], "file": "https://cdn.example.org/tiles/0_1_0.raw"},
  {"indices": [1,1,0], "file": "https://cdn.example.org/tiles/1_1_0.raw"},
  {"indices": [0,0,1], "file": "s3://image-archive/tiles/0_0_1.raw"},
  {"indices": [1,0,1], "file": "s3://image-archive/tiles/1_0_1.raw"},
  {"indices": [0,1,1], "file": "s3://image-archive/tiles/0_1_1.raw"},
  {"indices": [1,1,1], "file": "s3://image-archive/tiles/1_1_1.raw"}
]}

[NO BINARY DATA - TILES STORED IN MIXED LOCATIONS]
```

### 9.3 Multi-Resolution Examples

#### 9.3.1 Internal Multi-Resolution Example

```
{"jnrrd": "0004"}
{"type": "uint8"}
{"dimension": 3}
{"sizes": [8192, 8192, 2048]}
{"endian": "little"}
{"encoding": "raw"}
{"extensions": {"tile": "https://jnrrd.org/extensions/tile/v1.0.0"}}
{"tile:enabled": true}
{"tile:dimensions": [0, 1, 2]}
{"tile:sizes": [256, 256, 64]}
{"tile:storage": "internal"}
{"tile:format": "contiguous"}
{"tile:levels": 4}
{"tile:level_scales": [1, 2, 4, 8]}
{"tile:level_offsets": [1024, 2147484672, 2415919104, 2483027968]}
{"tile:offset_table": [1024, 4198400, 8395776, ...]}

[BINARY DATA OF TILES IN SEQUENCE, ALL RESOLUTION LEVELS]
```

#### 9.3.2 Remote Multi-Resolution Example

```
{"jnrrd": "0004"}
{"type": "float32"}
{"dimension": 3}
{"sizes": [16384, 16384, 4096]}
{"endian": "little"}
{"extensions": {"tile": "https://jnrrd.org/extensions/tile/v1.0.0"}}
{"tile:enabled": true}
{"tile:dimensions": [0, 1, 2]}
{"tile:sizes": [256, 256, 64]}
{"tile:storage": "external"}
{"tile:levels": 5}
{"tile:level_scales": [1, 2, 4, 8, 16]}
{"tile:pattern": "https://brain-atlas.example.org/datasets/hbm2023/pyramid/level{l}/{z}/{y}/{x}.zstd"}
{"tile:compression": "zstd"}
{"tile:downsample_method": "lanczos"}

[NO BINARY DATA - MULTI-RESOLUTION TILES STORED ON REMOTE SERVER]
```

#### 9.3.3 Hybrid Storage Multi-Resolution Example

```
{"jnrrd": "0004"}
{"type": "uint16"}
{"dimension": 3}
{"sizes": [32768, 32768, 8192]}
{"endian": "little"}
{"extensions": {"tile": "https://jnrrd.org/extensions/tile/v1.0.0"}}
{"tile:enabled": true}
{"tile:dimensions": [0, 1, 2]}
{"tile:sizes": [256, 256, 64]}
{"tile:storage": "external"}
{"tile:levels": 6}
{"tile:level_scales": [1, 2, 4, 8, 16, 32]}
{"tile:files": [
  {"level": 5, "indices": [0,0,0], "file": "/local/cache/tiles/level5/0_0_0.raw"},
  {"level": 4, "indices": [0,0,0], "file": "/local/cache/tiles/level4/0_0_0.raw"},
  {"level": 4, "indices": [1,0,0], "file": "/local/cache/tiles/level4/1_0_0.raw"}
]}
{"tile:pattern": "s3://massive-microscopy-dataset/acquisition003/pyramid/{l}/{z}/{y}/{x}.lz4"}
{"tile:compression": "lz4"}
{"tile:downsample_method": "gaussian"}

[NO BINARY DATA - MULTI-RESOLUTION TILES STORED IN HYBRID LOCATIONS]
```

In this example, the lowest resolutions (levels 4 and 5) and some tiles from higher resolutions are cached locally, while the bulk of the high-resolution data remains in cloud storage. This allows for efficient browsing of the dataset with immediate access to overview images, while detailed exploration can fetch additional tiles from the cloud as needed.

#### 9.3.4 Visualization Client Logic Example

When a client opens this dataset, it might implement logic like:

```python
def visualize_dataset(jnrrd_path):
    # Load JNRRD header
    header = load_jnrrd_header(jnrrd_path)
    
    # Start by loading the lowest resolution (highest level number)
    level = header["tile:levels"] - 1
    overview_tiles = load_level_tiles(header, level)
    display_overview(overview_tiles)
    
    # As user zooms in, load higher resolution data
    while user_is_zooming_in():
        visible_region = get_current_view_region()
        
        # Determine appropriate resolution level based on zoom
        appropriate_level = calculate_level_for_zoom(header, zoom_factor)
        
        # Load only visible tiles at the appropriate resolution
        visible_tiles = get_visible_tile_indices(header, appropriate_level, visible_region)
        
        for tile_index in visible_tiles:
            if not is_tile_cached(tile_index, appropriate_level):
                # Fetch tile from remote/cloud storage
                tile_data = fetch_tile(header, appropriate_level, tile_index)
                cache_tile(tile_data, appropriate_level, tile_index)
            
            # Display the tile
            display_tile(get_cached_tile(appropriate_level, tile_index))
```

## 10. Future Extensions

Future versions of this extension may include:

1. **Sparse tiling**: Only storing non-empty tiles
2. **Tile-level transform**: Different transformations for different tiles
3. **Progressive encoding**: For streaming visualization
4. **Cache hints**: Suggestions for efficient memory utilization
5. **Compression dictionaries**: Shared dictionaries for better compression

## 11. Compatibility Notes

1. JNRRD files using this extension remain backward compatible with readers that don't support tiling
2. Non-tiling-aware readers will interpret the file as a standard JNRRD file and attempt to read the entire dataset

## 12. JSON Schema

The following JSON Schema can be used to validate the tiling extension fields:

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "JNRRD Tiling Extension Schema",
  "description": "Schema for validating JNRRD tiling extension fields",
  "type": "object",
  "required": ["enabled", "dimensions", "sizes", "storage"],
  "properties": {
    "enabled": {
      "type": "boolean",
      "description": "Enable tiling support (stored as 'tile:enabled' in JNRRD files)",
      "enum": [true]
    },
    "dimensions": {
      "type": "array",
      "description": "Dimensions to tile (stored as 'tile:dimensions' in JNRRD files)",
      "items": {
        "type": "integer",
        "minimum": 0
      },
      "minItems": 1
    },
    "sizes": {
      "type": "array",
      "description": "Size of each tile in each dimension (stored as 'tile:sizes' in JNRRD files)",
      "items": {
        "type": "integer",
        "minimum": 1
      }
    },
    "storage": {
      "type": "string",
      "description": "Storage model for tiles (stored as 'tile:storage' in JNRRD files)",
      "enum": ["internal", "external"]
    },
    "format": {
      "type": "string",
      "description": "Format of internal tile storage (stored as 'tile:format' in JNRRD files)",
      "enum": ["contiguous", "chunked"]
    },
    "offset_table": {
      "type": "array",
      "description": "Byte offsets for each tile (stored as 'tile:offset_table' in JNRRD files)",
      "items": {
        "type": "integer",
        "minimum": 0
      }
    },
    "size_table": {
      "type": "array",
      "description": "Size in bytes of each tile (stored as 'tile:size_table' in JNRRD files)",
      "items": {
        "type": "integer",
        "minimum": 1
      }
    },
    "pattern": {
      "type": "string",
      "description": "Pattern for external tile filenames (stored as 'tile:pattern' in JNRRD files)"
    },
    "files": {
      "type": "array",
      "description": "Explicit list of files for each tile (stored as 'tile:files' in JNRRD files)",
      "items": {
        "type": "object",
        "required": ["indices", "file"],
        "properties": {
          "indices": {
            "type": "array",
            "items": {
              "type": "integer",
              "minimum": 0
            }
          },
          "file": {
            "type": "string"
          }
        }
      }
    },
    "base_dir": {
      "type": "string",
      "description": "Base directory for external tile files (stored as 'tile:base_dir' in JNRRD files)"
    },
    "edge_handling": {
      "type": "string",
      "description": "How to handle edges when dataset size isn't a multiple of tile size (stored as 'tile:edge_handling' in JNRRD files)",
      "enum": ["pad", "variable"],
      "default": "pad"
    },
    "padding_value": {
      "type": "number",
      "description": "Value to use for padding (stored as 'tile:padding_value' in JNRRD files)",
      "default": 0
    },
    "overlap": {
      "type": "array",
      "description": "Overlap between adjacent tiles in each dimension (stored as 'tile:overlap' in JNRRD files)",
      "items": {
        "type": "integer",
        "minimum": 0
      }
    },
    "compression": {
      "type": "string",
      "description": "Compression method for tiles (stored as 'tile:compression' in JNRRD files)",
      "enum": ["raw", "gzip", "bzip2", "zstd", "lz4"]
    },
    "compression_levels": {
      "type": "array",
      "description": "Compression level for each tile (stored as 'tile:compression_levels' in JNRRD files)",
      "items": {
        "type": "integer",
        "minimum": 0
      }
    },
    "levels": {
      "type": "integer",
      "description": "Number of resolution levels (stored as 'tile:levels' in JNRRD files)",
      "minimum": 1
    },
    "level_scales": {
      "type": "array",
      "description": "Scale factor for each resolution level (stored as 'tile:level_scales' in JNRRD files)",
      "items": {
        "type": "integer",
        "minimum": 1
      }
    },
    "level_offsets": {
      "type": "array",
      "description": "Byte offset for the start of each resolution level (stored as 'tile:level_offsets' in JNRRD files)",
      "items": {
        "type": "integer",
        "minimum": 0
      }
    },
    "downsample_method": {
      "type": "string",
      "description": "Method used for generating lower resolution levels (stored as 'tile:downsample_method' in JNRRD files)",
      "enum": ["average", "gaussian", "lanczos", "max", "min", "mode"]
    },
    "level_tile_sizes": {
      "type": "array",
      "description": "Different tile sizes for each resolution level (stored as 'tile:level_tile_sizes' in JNRRD files)",
      "items": {
        "type": "array",
        "items": {
          "type": "integer",
          "minimum": 1
        }
      }
    },
    "levels_stored": {
      "type": "array",
      "description": "Indices of resolution levels that are stored in the file (stored as 'tile:levels_stored' in JNRRD files)",
      "items": {
        "type": "integer",
        "minimum": 0
      }
    },
    "levels_virtual": {
      "type": "array",
      "description": "Indices of resolution levels that are generated on demand (stored as 'tile:levels_virtual' in JNRRD files)",
      "items": {
        "type": "integer",
        "minimum": 0
      }
    },
    "level_quality": {
      "type": "array",
      "description": "Quality metrics for each resolution level (stored as 'tile:level_quality' in JNRRD files)",
      "items": {
        "type": "object"
      }
    },
    "metadata": {
      "type": "array",
      "description": "Metadata for each tile (stored as 'tile:metadata' in JNRRD files)",
      "items": {
        "type": "object"
      }
    }
  },
  "dependencies": {
    "storage": {
      "oneOf": [
        {
          "properties": {
            "storage": { "enum": ["internal"] }
          },
          "required": ["offset_table"]
        },
        {
          "properties": {
            "storage": { "enum": ["external"] }
          },
          "oneOf": [
            { "required": ["pattern"] },
            { "required": ["files"] }
          ]
        }
      ]
    },
    "levels": {
      "required": ["level_scales"]
    },
    "levels_stored": {
      "required": ["levels_virtual"]
    },
    "level_tile_sizes": {
      "required": ["levels"]
    }
  }
}
```