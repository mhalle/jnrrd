# JNRRD

JNRRD (JSON-based Nearly Raw Raster Data) is a modern evolution of the NRRD file format, designed to combine the structural advantages of JSON with the efficiency of binary raster data storage.

## About

JNRRD aims to preserve the key concepts of NRRD (Nearly Raw Raster Data) while leveraging JSON's flexibility, extensibility, and widespread adoption. Like NRRD, JNRRD provides a framework for storing scientific and medical imaging data with comprehensive metadata, but with improved interoperability and extensibility.

## Documentation

- [JNRRD Specification](/spec/JNRRD-SPECIFICATION.md) - The formal specification document
- [NRRD Extensions Specification](/spec/NRRD-EXTENSIONS-SPEC.md) - Backward-compatible extension mechanism for NRRD files
- [NRRD Format](/docs/NRRD-SPEC.txt) - Reference documentation for the original NRRD format
- [JNRRD Concept](/docs/JNRRD-CONCEPT.md) - Original concept document for JNRRD

## Design Goals

- Maintain NRRD's powerful capabilities for describing n-dimensional arrays
- Leverage JSON's standardized format and universal support
- Provide robust metadata capabilities with a clear extension mechanism
- Support all scientific data types and orientations
- Ensure backward compatibility with NRRD concepts
- Improve readability and reduce ambiguity through standardized naming

## Implementation

The repository will contain reference implementations for working with JNRRD files in:

- Python (upcoming)
- JavaScript (upcoming)

## License

[MIT](LICENSE)