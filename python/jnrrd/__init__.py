"""
JNRRD - JSON-based Nearly Raw Raster Data

A Python library for reading and writing JNRRD files, which combine 
JSON metadata with binary raster data.
"""

__version__ = '0.2.0'

from .reader import read, read_header
from .writer import write, write_detached

__all__ = ['read', 'read_header', 'write', 'write_detached']