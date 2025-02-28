"""
Unit tests for JNRRD reader and writer
"""

import os
import numpy as np
import tempfile
import pytest
from pathlib import Path

# Add the parent directory to the path so we can import jnrrd
import sys
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../..')))

# Import the jnrrd module
from jnrrd import read, read_header, write, write_detached


def test_read_write_roundtrip():
    """Test that reading and writing a JNRRD file preserves data and metadata"""
    # Create a temporary directory for test files
    with tempfile.TemporaryDirectory() as temp_dir:
        # Create a simple 3D test array
        data = np.random.random((5, 10, 15)).astype(np.float32)
        
        # Create header with metadata
        header = {
            'content': 'Test Data',
            'space': 'right-anterior-superior',
            'spacings': [1.0, 2.0, 3.0],
            'jnrrd_ext_meta': {
                'name': 'Test Dataset',
                'description': 'A test dataset for unit tests',
            }
        }
        
        # Write to a temporary file
        temp_file = os.path.join(temp_dir, 'test.jnrrd')
        write(temp_file, header, data)
        
        # Read the file back
        read_header_result = read_header(temp_file)
        read_result_header, read_result_data = read(temp_file)
        
        # Check that the header contains all the expected fields
        assert read_header_result['jnrrd'] == '0004'
        assert read_header_result['content'] == 'Test Data'
        assert read_header_result['space'] == 'right-anterior-superior'
        assert read_header_result['spacings'] == [1.0, 2.0, 3.0]
        assert 'jnrrd_ext_meta' in read_header_result
        assert read_header_result['jnrrd_ext_meta']['name'] == 'Test Dataset'
        
        # Check that the data is preserved
        np.testing.assert_array_equal(data, read_result_data)


def test_extension_flattening():
    """Test that hierarchical extensions are properly flattened and reconstructed"""
    # Create a temporary directory for test files
    with tempfile.TemporaryDirectory() as temp_dir:
        # Create a simple test array
        data = np.zeros((2, 2, 2), dtype=np.float32)
        
        # Create header with nested extension data
        header = {
            'jnrrd_ext_test': {
                'level1': {
                    'level2': {
                        'level3': 'deeply_nested_value'
                    },
                    'array': [1, 2, 3, 4],
                    'mixed': [
                        {'name': 'item1', 'value': 10},
                        {'name': 'item2', 'value': 20}
                    ]
                }
            }
        }
        
        # Write to a temporary file
        temp_file = os.path.join(temp_dir, 'test_extensions.jnrrd')
        write(temp_file, header, data)
        
        # Read the file back
        read_result_header, _ = read(temp_file)
        
        # Check that the nested extension data is preserved
        assert 'jnrrd_ext_test' in read_result_header
        assert read_result_header['jnrrd_ext_test']['level1']['level2']['level3'] == 'deeply_nested_value'
        assert read_result_header['jnrrd_ext_test']['level1']['array'] == [1, 2, 3, 4]
        assert read_result_header['jnrrd_ext_test']['level1']['mixed'][0]['name'] == 'item1'
        assert read_result_header['jnrrd_ext_test']['level1']['mixed'][1]['value'] == 20


def test_detached_data():
    """Test writing and reading files with detached data"""
    # Create a temporary directory for test files
    with tempfile.TemporaryDirectory() as temp_dir:
        # Create a simple test array
        data = np.random.random((3, 3, 3)).astype(np.float32)
        
        # Create header
        header = {'content': 'Detached Data Test'}
        
        # Write to temporary files
        header_file = os.path.join(temp_dir, 'header.jnrrd')
        data_file = os.path.join(temp_dir, 'data.raw')
        write_detached(header_file, data_file, header, data)
        
        # Check that both files exist
        assert os.path.exists(header_file)
        assert os.path.exists(data_file)
        
        # Read the header file back
        read_result_header, read_result_data = read(header_file)
        
        # Check that the data_file field was added to the header
        assert 'data_file' in read_result_header
        assert read_result_header['data_file'] == os.path.basename(data_file)
        
        # Check that the data is correctly read
        np.testing.assert_array_equal(data, read_result_data)


def test_compression():
    """Test writing and reading with different compression methods"""
    # Create a temporary directory for test files
    with tempfile.TemporaryDirectory() as temp_dir:
        # Create a simple test array with repetitive data (good for compression)
        data = np.tile(np.arange(16, dtype=np.float32).reshape(4, 4), (10, 10, 1))
        
        # Try different encodings
        encodings = ['raw', 'gzip', 'ascii', 'hex']
        
        for encoding in encodings:
            # Create header
            header = {
                'content': f'Compression Test - {encoding}',
                'encoding': encoding
            }
            
            # Write to a temporary file
            temp_file = os.path.join(temp_dir, f'test_{encoding}.jnrrd')
            write(temp_file, header, data, encoding=encoding)
            
            # Read the file back
            read_result_header, read_result_data = read(temp_file)
            
            # Check that the encoding was preserved
            assert read_result_header['encoding'] == encoding
            
            # Check that the data is preserved
            np.testing.assert_array_almost_equal(data, read_result_data)


def test_data_types():
    """Test writing and reading with different NumPy data types"""
    # Create a temporary directory for test files
    with tempfile.TemporaryDirectory() as temp_dir:
        # Test different data types
        dtypes = [
            np.int8, np.uint8, 
            np.int16, np.uint16, 
            np.int32, np.uint32, 
            np.int64, np.uint64,
            np.float32, np.float64,
            np.complex64, np.complex128
        ]
        
        for dtype in dtypes:
            # Create a simple test array with the given dtype
            if np.issubdtype(dtype, np.integer):
                # For integer types, use a range appropriate for the type
                info = np.iinfo(dtype)
                data = np.random.randint(info.min // 2, info.max // 2, (3, 4, 5)).astype(dtype)
            elif np.issubdtype(dtype, np.floating):
                # For floating point types
                data = np.random.random((3, 4, 5)).astype(dtype)
            elif np.issubdtype(dtype, np.complexfloating):
                # For complex types
                real = np.random.random((3, 4, 5))
                imag = np.random.random((3, 4, 5))
                data = (real + 1j * imag).astype(dtype)
                
            # Write to a temporary file
            temp_file = os.path.join(temp_dir, f'test_{np.dtype(dtype).name}.jnrrd')
            write(temp_file, {}, data)
            
            # Read the file back
            read_result_header, read_result_data = read(temp_file)
            
            # Check that the data type is preserved
            assert read_result_data.dtype == dtype
            
            # Check that the data is preserved
            np.testing.assert_array_equal(data, read_result_data)


if __name__ == "__main__":
    # Run tests
    test_read_write_roundtrip()
    test_extension_flattening()
    test_detached_data()
    test_compression()
    test_data_types()
    
    print("All tests passed!")