"""
JNRRD writer module

This module provides functionality for writing JNRRD files.
"""

import json
import os
import numpy as np
from typing import Dict, Optional, Any, List, Union, Tuple
import gzip
import re


def _flatten_extensions(header: Dict[str, Any]) -> Dict[str, Any]:
    """
    Convert hierarchical extension structures into flat key-value pairs with namespace prefixes.
    
    Parameters
    ----------
    header : Dict[str, Any]
        Header dictionary containing processed extensions
        
    Returns
    -------
    Dict[str, Any]
        Header with flattened extension fields
    """
    result = {}
    
    # Copy regular fields
    for key, value in header.items():
        if not key.startswith('jnrrd_ext_'):
            result[key] = value
    
    # Process extension fields
    for key, value in header.items():
        if key.startswith('jnrrd_ext_'):
            prefix = key[10:]  # Remove 'jnrrd_ext_'
            _flatten_object(prefix, value, '', result)
    
    return result


def _flatten_object(prefix: str, obj: Any, path: str, result: Dict[str, Any]) -> None:
    """
    Recursively flatten a nested object structure into key-value pairs with namespace prefixes.
    
    Parameters
    ----------
    prefix : str
        Extension namespace prefix
    obj : Any
        Object to flatten
    path : str
        Current path in the object hierarchy
    result : Dict[str, Any]
        Dictionary to store the flattened key-value pairs
    """
    if isinstance(obj, dict):
        for key, value in obj.items():
            new_path = f"{path}.{key}" if path else key
            _flatten_object(prefix, value, new_path, result)
    elif isinstance(obj, list):
        # Check if the list contains only simple values
        if all(not isinstance(item, (dict, list)) for item in obj):
            # Store the whole list at once
            result[f"{prefix}:{path}"] = obj
        else:
            # Recursively flatten complex lists
            for i, item in enumerate(obj):
                new_path = f"{path}[{i}]"
                _flatten_object(prefix, item, new_path, result)
    else:
        # Leaf node (simple value)
        result[f"{prefix}:{path}"] = obj


def _get_jnrrd_type(dtype: np.dtype) -> str:
    """
    Map NumPy dtype to JNRRD type string.
    
    Parameters
    ----------
    dtype : np.dtype
        NumPy dtype
        
    Returns
    -------
    str
        JNRRD type string
    """
    # Remove endianness information for the mapping
    base_dtype = np.dtype(dtype.name)
    
    type_map = {
        np.dtype('int8'): 'int8',
        np.dtype('uint8'): 'uint8',
        np.dtype('int16'): 'int16',
        np.dtype('uint16'): 'uint16',
        np.dtype('int32'): 'int32',
        np.dtype('uint32'): 'uint32',
        np.dtype('int64'): 'int64',
        np.dtype('uint64'): 'uint64',
        np.dtype('float16'): 'float16',
        np.dtype('float32'): 'float32',
        np.dtype('float64'): 'float64',
        np.dtype('complex64'): 'complex64',
        np.dtype('complex128'): 'complex128',
    }
    
    if base_dtype in type_map:
        return type_map[base_dtype]
    else:
        raise ValueError(f"Unsupported NumPy dtype: {dtype}")


def _get_endian_from_dtype(dtype: np.dtype) -> str:
    """
    Determine the endianness from a NumPy dtype.
    
    Parameters
    ----------
    dtype : np.dtype
        NumPy dtype
        
    Returns
    -------
    str
        'little' or 'big'
    """
    if dtype.byteorder == '<' or (dtype.byteorder == '=' and np.little_endian):
        return 'little'
    else:
        return 'big'


def _prepare_header(header: Dict[str, Any], data: np.ndarray, encoding: str = 'raw') -> Dict[str, Any]:
    """
    Prepare the header for writing, ensuring all required fields are present.
    
    Parameters
    ----------
    header : Dict[str, Any]
        User-provided header
    data : np.ndarray
        NumPy array containing the data
    encoding : str
        Encoding method to use
        
    Returns
    -------
    Dict[str, Any]
        Prepared header with all required fields
    """
    result = header.copy()
    
    # Ensure required fields
    if 'jnrrd' not in result:
        result['jnrrd'] = '0004'
    
    if 'dimension' not in result:
        result['dimension'] = len(data.shape)
    
    if 'sizes' not in result:
        result['sizes'] = list(data.shape)
    
    if 'type' not in result:
        result['type'] = _get_jnrrd_type(data.dtype)
    
    if 'encoding' not in result:
        result['encoding'] = encoding
    
    if 'endian' not in result:
        result['endian'] = _get_endian_from_dtype(data.dtype)
    
    return result


def _write_raw_data(file: Any, data: np.ndarray) -> None:
    """
    Write raw binary data to a file.
    
    Parameters
    ----------
    file : file-like object
        File to write to
    data : np.ndarray
        NumPy array containing the data
    """
    # Ensure data is in the correct byte order
    if (data.dtype.byteorder == '>' and np.little_endian) or \
       (data.dtype.byteorder == '<' and not np.little_endian):
        # Need to byte-swap
        file.write(data.byteswap().tobytes())
    else:
        # Use native byte order
        file.write(data.tobytes())


def _write_gzip_data(file: Any, data: np.ndarray, compression_level: int = 9) -> None:
    """
    Write gzip-compressed data to a file.
    
    Parameters
    ----------
    file : file-like object
        File to write to
    data : np.ndarray
        NumPy array containing the data
    compression_level : int
        Compression level (1-9, 9 being highest)
    """
    # Convert data to bytes
    data_bytes = data.tobytes()
    
    # Compress the data
    compressed_data = gzip.compress(data_bytes, compression_level)
    
    # Write the compressed data
    file.write(compressed_data)


def _write_bzip2_data(file: Any, data: np.ndarray, compression_level: int = 9) -> None:
    """
    Write bzip2-compressed data to a file.
    
    Parameters
    ----------
    file : file-like object
        File to write to
    data : np.ndarray
        NumPy array containing the data
    compression_level : int
        Compression level (1-9, 9 being highest)
    """
    try:
        import bz2
    except ImportError:
        raise ImportError("bz2 module not available. Install it with 'pip install bz2file'")
    
    # Convert data to bytes
    data_bytes = data.tobytes()
    
    # Compress the data
    compressed_data = bz2.compress(data_bytes, compression_level)
    
    # Write the compressed data
    file.write(compressed_data)


def _write_zstd_data(file: Any, data: np.ndarray, compression_level: int = 10) -> None:
    """
    Write zstd-compressed data to a file.
    
    Parameters
    ----------
    file : file-like object
        File to write to
    data : np.ndarray
        NumPy array containing the data
    compression_level : int
        Compression level (1-22, higher means more compression)
    """
    try:
        import zstandard as zstd
    except ImportError:
        raise ImportError("zstandard module not available. Install it with 'pip install zstandard'")
    
    # Convert data to bytes
    data_bytes = data.tobytes()
    
    # Compress the data
    cctx = zstd.ZstdCompressor(level=compression_level)
    compressed_data = cctx.compress(data_bytes)
    
    # Write the compressed data
    file.write(compressed_data)


def _write_lz4_data(file: Any, data: np.ndarray, compression_level: int = 9) -> None:
    """
    Write LZ4-compressed data to a file.
    
    Parameters
    ----------
    file : file-like object
        File to write to
    data : np.ndarray
        NumPy array containing the data
    compression_level : int
        Compression level (1-12, higher means more compression)
    """
    try:
        import lz4.frame
    except ImportError:
        raise ImportError("lz4 module not available. Install it with 'pip install lz4'")
    
    # Convert data to bytes
    data_bytes = data.tobytes()
    
    # Compress the data
    compressed_data = lz4.frame.compress(data_bytes, compression_level=compression_level)
    
    # Write the compressed data
    file.write(compressed_data)


def _write_ascii_data(file: Any, data: np.ndarray) -> None:
    """
    Write data as ASCII text.
    
    Parameters
    ----------
    file : file-like object
        File to write to
    data : np.ndarray
        NumPy array containing the data
    """
    # Flatten the array
    flat_data = data.flatten()
    
    # Write data as space-separated values with newlines
    chunk_size = 8  # Number of values per line
    for i in range(0, len(flat_data), chunk_size):
        chunk = flat_data[i:i+chunk_size]
        line = ' '.join(str(val) for val in chunk) + '\n'
        file.write(line.encode('utf-8'))


def _write_hex_data(file: Any, data: np.ndarray) -> None:
    """
    Write data as hexadecimal text.
    
    Parameters
    ----------
    file : file-like object
        File to write to
    data : np.ndarray
        NumPy array containing the data
    """
    # Convert data to bytes
    data_bytes = data.tobytes()
    
    # Convert to hex
    hex_data = data_bytes.hex()
    
    # Write in chunks with newlines
    chunk_size = 64  # Characters per line
    for i in range(0, len(hex_data), chunk_size):
        chunk = hex_data[i:i+chunk_size] + '\n'
        file.write(chunk.encode('utf-8'))


def write(filename: str, header: Dict[str, Any], data: np.ndarray, encoding: str = 'raw', 
          compression_level: int = 9) -> None:
    """
    Write a JNRRD file.

    Parameters
    ----------
    filename : str
        Path to the output JNRRD file
    header : Dict[str, Any]
        Dictionary containing the header fields and extensions
    data : np.ndarray
        NumPy array containing the data to write
    encoding : str, optional
        Encoding method to use ('raw', 'gzip', 'bzip2', 'zstd', 'lz4', 'ascii', 'hex')
    compression_level : int, optional
        Compression level for compressed encodings (higher means more compression)
        
    Examples
    --------
    >>> import numpy as np
    >>> from jnrrd import write
    >>> 
    >>> # Create a sample 3D array
    >>> data = np.random.random((10, 20, 30))
    >>> 
    >>> # Create header with metadata
    >>> header = {
    ...     'content': '3D random data',
    ...     'space': 'right-anterior-superior',
    ...     'spacings': [1.0, 0.5, 0.5],
    ...     'jnrrd_ext_meta': {
    ...         'name': 'Sample Dataset',
    ...         'creator': {'name': 'JNRRD Python Library'},
    ...         'dateCreated': '2025-02-28'
    ...     }
    ... }
    >>> 
    >>> # Write the file
    >>> write('sample.jnrrd', header, data, encoding='gzip')
    """
    # Prepare the header
    prepared_header = _prepare_header(header, data, encoding)
    
    # Flatten extensions
    flat_header = _flatten_extensions(prepared_header)
    
    with open(filename, 'wb') as f:
        # Write header fields
        # First, write the magic field
        if 'jnrrd' in flat_header:
            jnrrd_field = {'jnrrd': flat_header['jnrrd']}
            line = json.dumps(jnrrd_field).encode('utf-8') + b'\n'
            f.write(line)
        
        # Then write the rest of the fields
        for key, value in flat_header.items():
            if key != 'jnrrd':  # Skip the magic field already written
                field = {key: value}
                line = json.dumps(field).encode('utf-8') + b'\n'
                f.write(line)
        
        # Add a blank line to separate header from data
        f.write(b'\n')
        
        # Write data based on encoding
        if encoding == 'raw':
            _write_raw_data(f, data)
        elif encoding in ['gzip', 'gz']:
            _write_gzip_data(f, data, compression_level)
        elif encoding in ['bzip2', 'bz2']:
            _write_bzip2_data(f, data, compression_level)
        elif encoding == 'zstd':
            _write_zstd_data(f, data, compression_level)
        elif encoding == 'lz4':
            _write_lz4_data(f, data, compression_level)
        elif encoding in ['ascii', 'text', 'txt']:
            _write_ascii_data(f, data)
        elif encoding == 'hex':
            _write_hex_data(f, data)
        else:
            raise ValueError(f"Unsupported encoding: {encoding}")


def write_detached(header_file: str, data_file: str, header: Dict[str, Any], data: np.ndarray,
                  encoding: str = 'raw', compression_level: int = 9) -> None:
    """
    Write a JNRRD file with detached data.

    Parameters
    ----------
    header_file : str
        Path to the output JNRRD header file
    data_file : str
        Path to the output JNRRD data file
    header : Dict[str, Any]
        Dictionary containing the header fields and extensions
    data : np.ndarray
        NumPy array containing the data to write
    encoding : str, optional
        Encoding method to use ('raw', 'gzip', 'bzip2', 'zstd', 'lz4', 'ascii', 'hex')
    compression_level : int, optional
        Compression level for compressed encodings (higher means more compression)
        
    Examples
    --------
    >>> import numpy as np
    >>> from jnrrd import write_detached
    >>> 
    >>> # Create a sample 3D array
    >>> data = np.random.random((10, 20, 30))
    >>> 
    >>> # Create header with metadata
    >>> header = {
    ...     'content': '3D random data',
    ...     'space': 'right-anterior-superior',
    ...     'spacings': [1.0, 0.5, 0.5]
    ... }
    >>> 
    >>> # Write detached files
    >>> write_detached('sample.jnrrd', 'sample.raw', header, data)
    """
    # Prepare the header
    prepared_header = _prepare_header(header, data, encoding)
    
    # Add data_file field to header
    # Use relative path if the data file is in the same directory
    if os.path.dirname(header_file) == os.path.dirname(data_file):
        prepared_header['data_file'] = os.path.basename(data_file)
    else:
        prepared_header['data_file'] = data_file
    
    # Flatten extensions
    flat_header = _flatten_extensions(prepared_header)
    
    # Write header file
    with open(header_file, 'wb') as f:
        # Write header fields
        # First, write the magic field
        if 'jnrrd' in flat_header:
            jnrrd_field = {'jnrrd': flat_header['jnrrd']}
            line = json.dumps(jnrrd_field).encode('utf-8') + b'\n'
            f.write(line)
        
        # Then write the rest of the fields
        for key, value in flat_header.items():
            if key != 'jnrrd':  # Skip the magic field already written
                field = {key: value}
                line = json.dumps(field).encode('utf-8') + b'\n'
                f.write(line)
        
        # Add a blank line at the end of the header
        f.write(b'\n')
    
    # Write data file
    with open(data_file, 'wb') as f:
        # Write data based on encoding
        if encoding == 'raw':
            _write_raw_data(f, data)
        elif encoding in ['gzip', 'gz']:
            _write_gzip_data(f, data, compression_level)
        elif encoding in ['bzip2', 'bz2']:
            _write_bzip2_data(f, data, compression_level)
        elif encoding == 'zstd':
            _write_zstd_data(f, data, compression_level)
        elif encoding == 'lz4':
            _write_lz4_data(f, data, compression_level)
        elif encoding in ['ascii', 'text', 'txt']:
            _write_ascii_data(f, data)
        elif encoding == 'hex':
            _write_hex_data(f, data)
        else:
            raise ValueError(f"Unsupported encoding: {encoding}")