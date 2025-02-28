#!/usr/bin/env python
"""
JNRRD Example Script

This script demonstrates basic usage of the jnrrd module for reading and writing JNRRD files.
"""

import numpy as np
import os
import sys
import time

# Add the parent directory to the path so we can import jnrrd
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from jnrrd import read, read_header, write, write_detached, JnrrdFile


def create_sample_data(shape=(10, 20, 30)):
    """
    Create a simple 3D numpy array for testing.
    
    Returns
    -------
    np.ndarray
        Test data array
    """
    # Create a 3D array with a simple pattern
    x, y, z = np.meshgrid(np.linspace(0, 1, shape[2]), 
                         np.linspace(0, 1, shape[1]), 
                         np.linspace(0, 1, shape[0]), 
                         indexing='ij')
    
    data = np.sin(2 * np.pi * x) * np.cos(4 * np.pi * y) * np.exp(-((z - 0.5) ** 2) / 0.25)
    
    return data


def example_write():
    """
    Example of writing a JNRRD file with metadata and extensions.
    """
    print("Creating sample data...")
    data = create_sample_data()
    
    print(f"Data shape: {data.shape}, dtype: {data.dtype}")
    
    # Create header with metadata
    header = {
        'content': '3D Sine-Cosine-Gaussian Pattern',
        'space': 'right-anterior-superior',
        'spacings': [0.1, 0.1, 0.1],  # 0.1mm spacing in each dimension
        'units': ['mm', 'mm', 'mm'],
        'labels': ['x', 'y', 'z'],
        'kinds': ['space', 'space', 'space'],
        
        # Extension data stored in extensions_data dictionary
        'extensions_data': {
            'meta': {
                'name': 'Sample JNRRD Dataset',
                'description': 'A sample 3D dataset created with the JNRRD Python library',
                'creator': {
                    'name': 'JNRRD Python Library',
                    'url': 'https://github.com/example/jnrrd'
                },
                'dateCreated': time.strftime('%Y-%m-%d'),
                'keywords': ['sample', 'test', 'jnrrd', 'python']
            },
            
            'custom': {
                'parameters': {
                    'frequency_x': 2.0,
                    'frequency_y': 4.0,
                    'gaussian_sigma': 0.5
                },
                'versions': {
                    'library': '0.1.0',
                    'numpy': np.__version__
                }
            }
        }
    }
    
    # Write the file with different encodings
    encodings = ['raw', 'gzip', 'ascii']
    
    for encoding in encodings:
        filename = f'sample_{encoding}.jnrrd'
        print(f"Writing {filename}...")
        write(filename, header, data, encoding=encoding)
        print(f"  File size: {os.path.getsize(filename):,} bytes")
    
    # Write a detached file version
    header_file = 'sample_detached.jnrrd'
    data_file = 'sample_detached.raw'
    print(f"Writing detached files: {header_file} and {data_file}...")
    write_detached(header_file, data_file, header, data)
    print(f"  Header size: {os.path.getsize(header_file):,} bytes")
    print(f"  Data size: {os.path.getsize(data_file):,} bytes")
    
    return encodings


def example_read(encodings):
    """
    Example of reading JNRRD files.
    
    Parameters
    ----------
    encodings : list
        List of encodings to read
    """
    for encoding in encodings:
        filename = f'sample_{encoding}.jnrrd'
        print(f"\nReading {filename}...")
        
        # First read just the header
        header = read_header(filename)
        
        # Print some basic info
        print(f"  JNRRD version: {header.get('jnrrd')}")
        print(f"  Encoding: {header.get('encoding')}")
        print(f"  Content: {header.get('content')}")
        print(f"  Dimensions: {header.get('dimension')}")
        print(f"  Sizes: {header.get('sizes')}")
        
        # Check for extensions (in the new structure)
        if 'extensions_data' in header:
            print(f"  Extensions: {', '.join(header['extensions_data'].keys())}")
            
            # Access extension data directly
            if 'meta' in header['extensions_data']:
                meta = header['extensions_data']['meta']
                print(f"    Dataset name: {meta.get('name')}")
                print(f"    Created: {meta.get('dateCreated')}")
        
        # Now read the full file with data
        full_header, data = read(filename)
        
        print(f"  Data shape: {data.shape}")
        print(f"  Data type: {data.dtype}")
        print(f"  Data range: [{data.min():.4f}, {data.max():.4f}]")
    
    # Try reading the detached file
    header_file = 'sample_detached.jnrrd'
    print(f"\nReading detached file {header_file}...")
    
    header, data = read(header_file)
    
    print(f"  Data file: {header.get('data_file')}")
    print(f"  Data shape: {data.shape}")
    print(f"  Data type: {data.dtype}")
    print(f"  Data range: [{data.min():.4f}, {data.max():.4f}]")


def example_class_based():
    """
    Example using the more Pythonic class-based API.
    """
    print("\nDemonstrating class-based API...")
    
    # Create sample data
    data = create_sample_data((5, 10, 15))
    
    # Create a new JNRRD file object
    jnrrd = JnrrdFile()
    jnrrd.data = data
    
    # Set core header fields
    jnrrd.header = {
        'content': 'Class-based API Example',
        'space': 'right-anterior-superior',
        'spacings': [0.5, 0.5, 1.0],
        'labels': ['x', 'y', 'z']
    }
    
    # Add extensions directly
    jnrrd.add_extension('meta', {
        'name': 'Class API Example',
        'description': 'Example using the JnrrdFile class API',
        'creator': {
            'name': 'JNRRD Python Library',
            'version': '0.2.0'
        }
    })
    
    # Add technical parameters as a separate extension
    jnrrd.add_extension('params', {
        'algorithm': 'sample',
        'created': time.strftime('%Y-%m-%d %H:%M:%S')
    })
    
    # Save the file
    filename = 'class_example.jnrrd'
    jnrrd.save(filename, encoding='gzip')
    print(f"  Saved {filename}, size: {os.path.getsize(filename):,} bytes")
    
    # Read it back using the class API
    print("  Reading back the file...")
    loaded = JnrrdFile.open(filename)
    
    # Display header and extensions
    print(f"  Content: {loaded.header.get('content')}")
    print(f"  Data shape: {loaded.data.shape}")
    
    # Access extensions directly
    if 'meta' in loaded.extensions:
        meta = loaded.extensions['meta']
        print(f"  Dataset name: {meta.get('name')}")
        print(f"  Creator: {meta.get('creator', {}).get('name')}")
    
    if 'params' in loaded.extensions:
        params = loaded.extensions['params']
        print(f"  Created: {params.get('created')}")
    
    # Save with detached data
    header_file = 'class_example.header.jnrrd'
    data_file = 'class_example.data.raw'
    loaded.save_detached(header_file, data_file)
    print(f"  Saved detached files:")
    print(f"    Header: {os.path.getsize(header_file):,} bytes")
    print(f"    Data: {os.path.getsize(data_file):,} bytes")


def main():
    """Main function"""
    print("JNRRD Python Library Example\n")
    
    # Create example directory if it doesn't exist
    os.makedirs('output', exist_ok=True)
    
    # Change to the output directory
    os.chdir('output')
    
    # Run examples
    encodings = example_write()
    example_read(encodings)
    
    # Run class-based example
    example_class_based()
    
    print("\nExample completed successfully!")


if __name__ == "__main__":
    main()