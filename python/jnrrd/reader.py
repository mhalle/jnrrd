"""
JNRRD reader module

This module provides functionality for reading JNRRD files.
"""

import json
import os
import numpy as np
from typing import Dict, Tuple, Union, BinaryIO, Optional, Any, List
import gzip
import re
from pathlib import Path


def _process_extension_fields(header: Dict[str, Any]) -> Dict[str, Any]:
    """
    Process extension fields in the header, converting flat notation into hierarchical structures.
    
    Parameters
    ----------
    header : Dict[str, Any]
        Raw header dictionary
        
    Returns
    -------
    Dict[str, Any]
        Processed header with properly structured extension fields
    """
    # Extract and initialize extensions
    extensions = {}
    extensions_decl = header.get('extensions', {})
    
    # Process all fields, looking for extension prefixes (containing ':')
    extension_fields = {}
    regular_fields = {}
    
    for key, value in header.items():
        if ':' in key:
            # This is an extension field
            prefix, path = key.split(':', 1)
            if prefix not in extension_fields:
                extension_fields[prefix] = {}
            extension_fields[prefix][path] = value
        else:
            # This is a regular field
            regular_fields[key] = value
    
    # Process each extension's fields to build hierarchical structures
    for prefix, fields in extension_fields.items():
        extensions[prefix] = {}
        for path, value in fields.items():
            _set_nested_value(extensions[prefix], path, value)
    
    # Rebuild the header
    result = regular_fields.copy()
    
    # Add extension declarations
    if extensions:
        result['extensions'] = extensions_decl
        
        # Store the processed extensions under a special key format
        for prefix, ext_data in extensions.items():
            result[f'jnrrd_ext_{prefix}'] = ext_data
    
    return result


def _set_nested_value(obj: Dict[str, Any], path: str, value: Any) -> None:
    """
    Set a value in a nested dictionary using a dot-notation path.
    Handles array indices in paths.
    
    Parameters
    ----------
    obj : Dict[str, Any]
        Dictionary to modify
    path : str
        Path in dot notation, possibly with array indices like "foo.bar[0].baz"
    value : Any
        Value to set at the path
    """
    # Parse path components, handling array indices
    components = []
    path_pattern = r'([^\.\[\]]+)|\[(\d+)\]'
    for match in re.finditer(path_pattern, path):
        if match.group(1) is not None:
            components.append(match.group(1))  # Normal field
        elif match.group(2) is not None:
            components.append(int(match.group(2)))  # Array index
    
    # Navigate to the right position
    current = obj
    for i, component in enumerate(components[:-1]):
        if isinstance(component, int):
            # Array index
            # Ensure the parent is a list of sufficient size
            if not isinstance(current, list):
                current = []
            while len(current) <= component:
                current.append(None)
            if current[component] is None:
                if isinstance(components[i+1], int):
                    current[component] = []
                else:
                    current[component] = {}
            current = current[component]
        else:
            # Dictionary key
            if component not in current:
                if isinstance(components[i+1], int):
                    current[component] = []
                else:
                    current[component] = {}
            current = current[component]
    
    # Set the value at the final position
    last_component = components[-1]
    if isinstance(last_component, int):
        # Array index
        if not isinstance(current, list):
            current = []
        while len(current) <= last_component:
            current.append(None)
        current[last_component] = value
    else:
        # Dictionary key
        current[last_component] = value


def read_header(filename: str) -> Dict[str, Any]:
    """
    Read only the header from a JNRRD file.

    Parameters
    ----------
    filename : str
        Path to the JNRRD file

    Returns
    -------
    Dict[str, Any]
        Dictionary containing the header fields with processed extensions
    """
    header = {}
    data_start_pos = 0
    
    with open(filename, 'rb') as f:
        while True:
            line_start = f.tell()
            line = f.readline().strip()
            
            if not line:  # Empty line indicates end of header
                data_start_pos = f.tell()
                break
            
            try:
                field = json.loads(line)
                # Each line should be a JSON object with a single key-value pair
                header.update(field)
            except json.JSONDecodeError:
                # If we encounter something that's not valid JSON, we've reached the data section
                data_start_pos = line_start
                break
    
    # Process extension fields
    processed_header = _process_extension_fields(header)
    
    # Store the data start position for later use
    processed_header['__data_start_pos'] = data_start_pos
                
    return processed_header


def _get_numpy_dtype(jnrrd_type: str, endian: Optional[str] = None) -> np.dtype:
    """
    Map JNRRD type strings to NumPy dtypes.
    
    Parameters
    ----------
    jnrrd_type : str
        JNRRD type string (e.g., 'int8', 'float32')
    endian : Optional[str]
        Endianness ('little' or 'big'), or None for native
        
    Returns
    -------
    np.dtype
        NumPy dtype for the specified JNRRD type
    """
    type_map = {
        'int8': np.int8,
        'uint8': np.uint8,
        'int16': np.int16,
        'uint16': np.uint16,
        'int32': np.int32,
        'uint32': np.uint32,
        'int64': np.int64,
        'uint64': np.uint64,
        'float16': np.float16,
        'bfloat16': np.float16,  # NumPy doesn't directly support bfloat16, map to float16
        'float32': np.float32,
        'float64': np.float64,
        'complex64': np.complex64,
        'complex128': np.complex128,
    }
    
    if jnrrd_type not in type_map:
        raise ValueError(f"Unsupported JNRRD type: {jnrrd_type}")
    
    dtype = np.dtype(type_map[jnrrd_type])
    
    # Handle endianness if specified
    if endian:
        if endian == 'little':
            dtype = dtype.newbyteorder('<')
        elif endian == 'big':
            dtype = dtype.newbyteorder('>')
        else:
            raise ValueError(f"Invalid endian value: {endian}")
    
    return dtype


def _read_data(filename: str, header: Dict[str, Any]) -> np.ndarray:
    """
    Read the binary data portion of a JNRRD file.
    
    Parameters
    ----------
    filename : str
        Path to the JNRRD file
    header : Dict[str, Any]
        Header dictionary with processed fields
        
    Returns
    -------
    np.ndarray
        NumPy array containing the data
    """
    # Check required fields
    if 'type' not in header:
        raise ValueError("Missing 'type' field in header")
    if 'dimension' not in header:
        raise ValueError("Missing 'dimension' field in header")
    if 'sizes' not in header:
        raise ValueError("Missing 'sizes' field in header")
    
    # Get data properties
    jnrrd_type = header['type']
    sizes = header['sizes']
    encoding = header.get('encoding', 'raw')
    endian = header.get('endian', None)
    
    # Get data start position
    data_start_pos = header.get('__data_start_pos', 0)
    
    # Handle detached data
    data_file = filename
    if 'data_file' in header:
        data_path = header['data_file']
        # Check if it's a relative path
        if not os.path.isabs(data_path):
            base_dir = os.path.dirname(filename)
            data_file = os.path.join(base_dir, data_path)
        else:
            data_file = data_path
        # Reset data start position for detached file
        data_start_pos = 0
    
    # Handle line skip in detached file
    if 'line_skip' in header and 'data_file' in header:
        with open(data_file, 'rb') as f:
            for _ in range(header['line_skip']):
                f.readline()
            data_start_pos = f.tell()
    
    # Handle byte skip
    if 'byte_skip' in header:
        data_start_pos += header['byte_skip']
    
    # Get NumPy dtype
    dtype = _get_numpy_dtype(jnrrd_type, endian)
    
    # Read the data based on encoding
    if encoding == 'raw':
        return _read_raw_data(data_file, data_start_pos, dtype, sizes)
    elif encoding in ['gzip', 'gz']:
        return _read_gzip_data(data_file, data_start_pos, dtype, sizes)
    elif encoding in ['bzip2', 'bz2']:
        return _read_bzip2_data(data_file, data_start_pos, dtype, sizes)
    elif encoding == 'zstd':
        return _read_zstd_data(data_file, data_start_pos, dtype, sizes)
    elif encoding == 'lz4':
        return _read_lz4_data(data_file, data_start_pos, dtype, sizes)
    elif encoding == 'ascii' or encoding == 'text' or encoding == 'txt':
        return _read_ascii_data(data_file, data_start_pos, dtype, sizes)
    elif encoding == 'hex':
        return _read_hex_data(data_file, data_start_pos, dtype, sizes)
    else:
        raise ValueError(f"Unsupported encoding: {encoding}")


def _read_raw_data(filename: str, data_start_pos: int, dtype: np.dtype, sizes: List[int]) -> np.ndarray:
    """
    Read raw binary data from a file.
    
    Parameters
    ----------
    filename : str
        Path to the data file
    data_start_pos : int
        Position in the file where data starts
    dtype : np.dtype
        NumPy dtype for the data
    sizes : List[int]
        List of dimension sizes
        
    Returns
    -------
    np.ndarray
        NumPy array containing the data
    """
    with open(filename, 'rb') as f:
        f.seek(data_start_pos)
        # Calculate total number of elements
        total_elements = 1
        for size in sizes:
            total_elements *= size
        
        # Read the data
        data_buffer = f.read(total_elements * dtype.itemsize)
        if len(data_buffer) < total_elements * dtype.itemsize:
            raise ValueError(f"Not enough data in file. Expected {total_elements * dtype.itemsize} bytes, got {len(data_buffer)} bytes")
        
        # Create array from buffer
        data = np.frombuffer(data_buffer, dtype=dtype)
        
        # Reshape to specified dimensions
        if sizes:
            data = data.reshape(sizes)
        
        return data


def _read_gzip_data(filename: str, data_start_pos: int, dtype: np.dtype, sizes: List[int]) -> np.ndarray:
    """
    Read gzip-compressed data from a file.
    
    Parameters
    ----------
    filename : str
        Path to the data file
    data_start_pos : int
        Position in the file where compressed data starts
    dtype : np.dtype
        NumPy dtype for the data
    sizes : List[int]
        List of dimension sizes
        
    Returns
    -------
    np.ndarray
        NumPy array containing the decompressed data
    """
    # Calculate total number of elements
    total_elements = 1
    for size in sizes:
        total_elements *= size
    
    with open(filename, 'rb') as f:
        f.seek(data_start_pos)
        compressed_data = f.read()
    
    # Decompress the data
    decompressed_data = gzip.decompress(compressed_data)
    
    # Check if we have enough data
    if len(decompressed_data) < total_elements * dtype.itemsize:
        raise ValueError(f"Not enough data after decompression. Expected {total_elements * dtype.itemsize} bytes, got {len(decompressed_data)} bytes")
    
    # Create array from buffer
    data = np.frombuffer(decompressed_data, dtype=dtype)
    
    # Reshape to specified dimensions
    if sizes:
        data = data.reshape(sizes)
    
    return data


def _read_bzip2_data(filename: str, data_start_pos: int, dtype: np.dtype, sizes: List[int]) -> np.ndarray:
    """
    Read bzip2-compressed data from a file.
    
    Parameters
    ----------
    filename : str
        Path to the data file
    data_start_pos : int
        Position in the file where compressed data starts
    dtype : np.dtype
        NumPy dtype for the data
    sizes : List[int]
        List of dimension sizes
        
    Returns
    -------
    np.ndarray
        NumPy array containing the decompressed data
    """
    try:
        import bz2
    except ImportError:
        raise ImportError("bz2 module not available. Install it with 'pip install bz2file'")
    
    # Calculate total number of elements
    total_elements = 1
    for size in sizes:
        total_elements *= size
    
    with open(filename, 'rb') as f:
        f.seek(data_start_pos)
        compressed_data = f.read()
    
    # Decompress the data
    decompressed_data = bz2.decompress(compressed_data)
    
    # Check if we have enough data
    if len(decompressed_data) < total_elements * dtype.itemsize:
        raise ValueError(f"Not enough data after decompression. Expected {total_elements * dtype.itemsize} bytes, got {len(decompressed_data)} bytes")
    
    # Create array from buffer
    data = np.frombuffer(decompressed_data, dtype=dtype)
    
    # Reshape to specified dimensions
    if sizes:
        data = data.reshape(sizes)
    
    return data


def _read_zstd_data(filename: str, data_start_pos: int, dtype: np.dtype, sizes: List[int]) -> np.ndarray:
    """
    Read zstd-compressed data from a file.
    
    Parameters
    ----------
    filename : str
        Path to the data file
    data_start_pos : int
        Position in the file where compressed data starts
    dtype : np.dtype
        NumPy dtype for the data
    sizes : List[int]
        List of dimension sizes
        
    Returns
    -------
    np.ndarray
        NumPy array containing the decompressed data
    """
    try:
        import zstandard as zstd
    except ImportError:
        raise ImportError("zstandard module not available. Install it with 'pip install zstandard'")
    
    # Calculate total number of elements
    total_elements = 1
    for size in sizes:
        total_elements *= size
    
    with open(filename, 'rb') as f:
        f.seek(data_start_pos)
        compressed_data = f.read()
    
    # Decompress the data
    dctx = zstd.ZstdDecompressor()
    decompressed_data = dctx.decompress(compressed_data)
    
    # Check if we have enough data
    if len(decompressed_data) < total_elements * dtype.itemsize:
        raise ValueError(f"Not enough data after decompression. Expected {total_elements * dtype.itemsize} bytes, got {len(decompressed_data)} bytes")
    
    # Create array from buffer
    data = np.frombuffer(decompressed_data, dtype=dtype)
    
    # Reshape to specified dimensions
    if sizes:
        data = data.reshape(sizes)
    
    return data


def _read_lz4_data(filename: str, data_start_pos: int, dtype: np.dtype, sizes: List[int]) -> np.ndarray:
    """
    Read LZ4-compressed data from a file.
    
    Parameters
    ----------
    filename : str
        Path to the data file
    data_start_pos : int
        Position in the file where compressed data starts
    dtype : np.dtype
        NumPy dtype for the data
    sizes : List[int]
        List of dimension sizes
        
    Returns
    -------
    np.ndarray
        NumPy array containing the decompressed data
    """
    try:
        import lz4.frame
    except ImportError:
        raise ImportError("lz4 module not available. Install it with 'pip install lz4'")
    
    # Calculate total number of elements
    total_elements = 1
    for size in sizes:
        total_elements *= size
    
    with open(filename, 'rb') as f:
        f.seek(data_start_pos)
        compressed_data = f.read()
    
    # Decompress the data
    decompressed_data = lz4.frame.decompress(compressed_data)
    
    # Check if we have enough data
    if len(decompressed_data) < total_elements * dtype.itemsize:
        raise ValueError(f"Not enough data after decompression. Expected {total_elements * dtype.itemsize} bytes, got {len(decompressed_data)} bytes")
    
    # Create array from buffer
    data = np.frombuffer(decompressed_data, dtype=dtype)
    
    # Reshape to specified dimensions
    if sizes:
        data = data.reshape(sizes)
    
    return data


def _read_ascii_data(filename: str, data_start_pos: int, dtype: np.dtype, sizes: List[int]) -> np.ndarray:
    """
    Read ASCII text data from a file.
    
    Parameters
    ----------
    filename : str
        Path to the data file
    data_start_pos : int
        Position in the file where data starts
    dtype : np.dtype
        NumPy dtype for the data
    sizes : List[int]
        List of dimension sizes
        
    Returns
    -------
    np.ndarray
        NumPy array containing the data
    """
    with open(filename, 'rb') as f:
        f.seek(data_start_pos)
        text_data = f.read().decode('utf-8')
    
    # Parse the ASCII data
    values = []
    for line in text_data.splitlines():
        line = line.strip()
        if line:
            for val in line.split():
                values.append(val)
    
    # Convert to the appropriate type
    if np.issubdtype(dtype, np.integer):
        values = [int(v) for v in values]
    elif np.issubdtype(dtype, np.floating):
        values = [float(v) for v in values]
    elif np.issubdtype(dtype, np.complexfloating):
        # Handle complex values (format like "1.2+3.4j")
        values = [complex(v) for v in values]
    
    # Create array
    data = np.array(values, dtype=dtype)
    
    # Calculate total number of elements
    total_elements = 1
    for size in sizes:
        total_elements *= size
    
    # Check if we have enough data
    if len(data) < total_elements:
        raise ValueError(f"Not enough data in ASCII text. Expected {total_elements} elements, got {len(data)} elements")
    
    # Reshape to specified dimensions
    if sizes:
        data = data[:total_elements].reshape(sizes)
    
    return data


def _read_hex_data(filename: str, data_start_pos: int, dtype: np.dtype, sizes: List[int]) -> np.ndarray:
    """
    Read hexadecimal data from a file.
    
    Parameters
    ----------
    filename : str
        Path to the data file
    data_start_pos : int
        Position in the file where data starts
    dtype : np.dtype
        NumPy dtype for the data
    sizes : List[int]
        List of dimension sizes
        
    Returns
    -------
    np.ndarray
        NumPy array containing the data
    """
    with open(filename, 'rb') as f:
        f.seek(data_start_pos)
        hex_data = f.read().decode('utf-8')
    
    # Remove whitespace and non-hex characters
    hex_data = ''.join([c for c in hex_data if c.isalnum()])
    
    # Convert hex to binary
    binary_data = bytes.fromhex(hex_data)
    
    # Create array from buffer
    data = np.frombuffer(binary_data, dtype=dtype)
    
    # Calculate total number of elements
    total_elements = 1
    for size in sizes:
        total_elements *= size
    
    # Check if we have enough data
    if len(data) < total_elements:
        raise ValueError(f"Not enough data in hex encoding. Expected {total_elements} elements, got {len(data)} elements")
    
    # Reshape to specified dimensions
    if sizes:
        data = data[:total_elements].reshape(sizes)
    
    return data


def read(filename: str) -> Tuple[Dict[str, Any], np.ndarray]:
    """
    Read a JNRRD file, returning both the header and data.

    Parameters
    ----------
    filename : str
        Path to the JNRRD file

    Returns
    -------
    Tuple[Dict[str, Any], np.ndarray]
        A tuple containing:
        - Dictionary with header fields
        - NumPy array containing the data
        
    Examples
    --------
    >>> header, data = read('example.jnrrd')
    >>> print(f"Data shape: {data.shape}")
    >>> print(f"Data type: {data.dtype}")
    >>> print(f"Content: {header.get('content', 'Unknown')}")
    >>> # Access any extension fields
    >>> if 'jnrrd_ext_meta' in header:
    ...     print(f"Dataset name: {header['jnrrd_ext_meta'].get('name', 'Unnamed')}")
    """
    # Read the header
    header = read_header(filename)
    
    # Remove internal fields from the returned header
    data_start_pos = header.pop('__data_start_pos', 0)
    
    # Read the data
    data = _read_data(filename, header)
    
    return header, data