"""
JNRRD - JSON-based Nearly Raw Raster Data

A Python library for reading and writing JNRRD files, which combine 
JSON metadata with binary raster data.

## Key Features

- Read and write multi-dimensional array data with comprehensive metadata
- Support for all NumPy dtypes including integers, floats, and complex numbers
- Multiple data encoding formats: raw, gzip, bzip2, zstd, lz4, ascii, hex
- Extensible metadata through namespaced extension fields
- Support for detached data files

## Usage Examples

### Function-based API

```python
import numpy as np
from jnrrd import read, write

# Create a sample dataset
data = np.random.random((10, 20, 30))

# Create header with metadata and extensions
header = {
    'content': 'Random 3D data',
    'space': 'right-anterior-superior',
    'spacings': [1.0, 1.0, 1.0],
    'extensions_data': {
        'meta': {
            'name': 'Sample Dataset',
            'description': 'A sample 3D dataset',
            'creator': {'name': 'JNRRD Python Library'}
        }
    }
}

# Write the data
write('sample.jnrrd', header, data, encoding='gzip')

# Read the data
header, data = read('sample.jnrrd')

# Access extension data
meta = header['extensions_data']['meta']
print(f"Dataset name: {meta['name']}")
```

### Class-based API (Recommended)

```python
import numpy as np
from jnrrd import JnrrdFile

# Create a new JNRRD file object
jnrrd = JnrrdFile()

# Set data
jnrrd.data = np.random.random((10, 20, 30))

# Set header fields
jnrrd.header = {
    'content': 'Random 3D data',
    'space': 'right-anterior-superior',
    'spacings': [1.0, 1.0, 1.0]
}

# Add extension data directly
jnrrd.add_extension('meta', {
    'name': 'Sample Dataset',
    'description': 'A sample 3D dataset',
    'creator': {'name': 'JNRRD Python Library'}
})

# Save to file
jnrrd.save('sample.jnrrd', encoding='gzip')

# Open a JNRRD file
loaded = JnrrdFile.open('sample.jnrrd')

# Access data, header and extensions directly
print(f"Data shape: {loaded.data.shape}")
print(f"Content: {loaded.header['content']}")
print(f"Dataset name: {loaded.extensions['meta']['name']}")
```

For more examples, see the `examples/` directory.
"""

__version__ = '0.2.0'

from .reader import read, read_header, JnrrdFile
from .writer import write, write_detached

__all__ = ['read', 'read_header', 'write', 'write_detached', 'JnrrdFile']