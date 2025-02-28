# ITK JNRRD I/O Module

This module implements an ITK reader and writer for the JNRRD (JSON-based Nearly Raw Raster Data) format, which is a JSON-based evolution of the NRRD format.

## Features

- Complete ITK integration via the ImageIO framework
- Support for reading and writing JNRRD files
- Support for all standard JNRRD data types
- Support for coordinate system transformation
- Handling of extension namespaces (DICOM, NIfTI, transforms, etc.)
- Support for compressed data (gzip, bzip2, zstd, lz4)
- Support for detached data files (reading)
- Full metadata preservation when reading and writing

## Dependencies

- ITK (>= 5.0.0)
- nlohmann/json (automatically fetched if not found)
- zlib (required)
- bzip2 (optional)
- zstd (optional)
- lz4 (optional)

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

The JNRRD I/O is automatically registered with ITK's ImageIO factory system. You can use it with the standard ITK image I/O classes:

```cpp
#include "itkJNRRDImageIOFactory.h"
#include "itkImageFileReader.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include <nlohmann/json.hpp>

// Register the JNRRD factory
itk::JNRRDImageIOFactory::RegisterOneFactory();

// Create a reader
using ReaderType = itk::ImageFileReader<itk::Image<float, 3>>;
auto reader = ReaderType::New();
reader->SetFileName("image.jnrrd");
reader->Update();

// Get the image
auto image = reader->GetOutput();

// Access basic image information
std::cout << "Image information:" << std::endl;
std::cout << "  Size: " << image->GetBufferedRegion().GetSize() << std::endl;
std::cout << "  Spacing: " << image->GetSpacing() << std::endl;
std::cout << "  Origin: " << image->GetOrigin() << std::endl;
std::cout << "  Direction matrix: " << image->GetDirection() << std::endl;

// Access metadata from the JNRRD header
const auto& dict = reader->GetMetaDataDictionary();
std::string value;

// Access basic JNRRD fields
if (itk::ExposeMetaData(dict, "jnrrd", value)) {
  std::cout << "JNRRD version: " << value << std::endl;
}

if (itk::ExposeMetaData(dict, "type", value)) {
  std::cout << "Data type: " << value << std::endl;
}

if (itk::ExposeMetaData(dict, "dimension", value)) {
  std::cout << "Dimension: " << value << std::endl;
}

if (itk::ExposeMetaData(dict, "space", value)) {
  std::cout << "Coordinate system: " << value << std::endl;
}

// Process a voxel
using PixelType = itk::Image<float, 3>::PixelType;
itk::Index<3> index = {50, 50, 5};  // Example index
PixelType pixel = image->GetPixel(index);
std::cout << "Pixel value at " << index << ": " << pixel << std::endl;

// Convert index to physical point
itk::Point<double, 3> point;
image->TransformIndexToPhysicalPoint(index, point);
std::cout << "Physical coordinates: " << point << std::endl;
```

## Extension Handling

The JNRRD format supports extensions with a namespace prefix mechanism. The reader extracts extension data into metadata dictionary entries with the prefix "jnrrd_ext_" followed by the namespace.

```cpp
// Access extension data (e.g., DICOM, NIfTI, transforms)
using json = nlohmann::json;

// Example 1: Accessing DICOM extension data
std::string dicomData;
if (itk::ExposeMetaData(dict, "jnrrd_ext_dicom", dicomData)) {
  // Parse the JSON string back to a JSON object
  json dicomJson = json::parse(dicomData);
  
  // Access patient information
  if (dicomJson.contains("patient")) {
    auto& patient = dicomJson["patient"];
    std::cout << "Patient ID: " << patient["id"].get<std::string>() << std::endl;
    std::cout << "Patient Sex: " << patient["sex"].get<std::string>() << std::endl;
  }
  
  // Access study information
  if (dicomJson.contains("study")) {
    auto& study = dicomJson["study"];
    std::cout << "Study Description: " << study["description"].get<std::string>() << std::endl;
    std::cout << "Study Date: " << study["date"].get<std::string>() << std::endl;
  }
}

// Example 2: Accessing transforms extension data
std::string transformData;
if (itk::ExposeMetaData(dict, "jnrrd_ext_transform", transformData)) {
  json transformJson = json::parse(transformData);
  
  // Access coordinate systems
  if (transformJson.contains("coordinateSystems")) {
    for (const auto& cs : transformJson["coordinateSystems"]) {
      std::cout << "Coordinate system: " << cs["name"].get<std::string>() << std::endl;
      
      // List axes
      for (const auto& axis : cs["axes"]) {
        std::cout << "  Axis: " << axis["name"].get<std::string>() 
                  << " (" << axis["type"].get<std::string>() << ")" << std::endl;
      }
    }
  }
  
  // Access transformations
  if (transformJson.contains("transformations")) {
    for (const auto& transform : transformJson["transformations"]) {
      std::cout << "Transform: " << transform["name"].get<std::string>() << std::endl;
      std::cout << "  Type: " << transform["type"].get<std::string>() << std::endl;
      std::cout << "  Input space: " << transform["input"].get<std::string>() << std::endl;
      std::cout << "  Output space: " << transform["output"].get<std::string>() << std::endl;
    }
  }
}

// Example 3: Accessing NIfTI extension data
std::string niftiData;
if (itk::ExposeMetaData(dict, "jnrrd_ext_nifti", niftiData)) {
  json niftiJson = json::parse(niftiData);
  
  if (niftiJson.contains("sform_code")) {
    std::cout << "NIfTI sform_code: " << niftiJson["sform_code"].get<int>() << std::endl;
  }
  
  if (niftiJson.contains("intent_code")) {
    std::cout << "NIfTI intent_code: " << niftiJson["intent_code"].get<int>() << std::endl;
  }
}
```

For a JNRRD file containing extension data like:
```json
{"dicom:patient": {"id": "12345", "sex": "M"}}
{"dicom:study": {"description": "BRAIN MRI", "date": "20240115"}}
```

The extensions will be accessible in the metadata dictionary via the key "jnrrd_ext_dicom" with the value:
```json
{"patient": {"id": "12345", "sex": "M"}, "study": {"description": "BRAIN MRI", "date": "20240115"}}
```

## Coordinate System Support

The reader handles coordinate system mapping between JNRRD's coordinate systems and ITK's internal representation:

- When JNRRD uses "right-anterior-superior" (RAS), this maps directly to ITK's coordinate system
- When JNRRD uses "left-posterior-superior" (LPS), the reader automatically flips the X and Y axes to convert to RAS

## Writing JNRRD Files

The JNRRD writer provides the capability to write images with complete metadata. Here's a simple example:

```cpp
#include "itkJNRRDImageIOFactory.h"
#include "itkImageFileWriter.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include <nlohmann/json.hpp>

// Register the JNRRD factory
itk::JNRRDImageIOFactory::RegisterOneFactory();

// Prepare the image
auto image = itk::Image<float, 3>::New();
// ... set up image dimensions, allocate, fill with data ...

// Create a writer
using WriterType = itk::ImageFileWriter<itk::Image<float, 3>>;
auto writer = WriterType::New();
writer->SetFileName("output.jnrrd");
writer->SetInput(image);

// Add metadata
auto & dict = writer->GetMetaDataDictionary();

// Add basic metadata
itk::EncapsulateMetaData<std::string>(dict, "content", "My JNRRD dataset");

// Add extension data
using json = nlohmann::json;
json metadataExt = {
  {"name", "My Dataset"},
  {"description", "A sample dataset created with ITK JNRRD writer"},
  {"dateCreated", "2025-02-28"}
};

// Store the extension in the metadata dictionary
itk::EncapsulateMetaData<std::string>(
  dict, "jnrrd_ext_metadata", metadataExt.dump());

// Write with compression
itk::EncapsulateMetaData<std::string>(dict, "encoding", "gzip");

// Write the image
writer->Update();
```

### Writer Features

- Support for all ITK image types
- Automatic handling of metadata and extensions
- Support for coordinate system transformation
- Multiple compression options (raw, gzip, bzip2, zstd, lz4)
- Customizable encoding parameters

## Limitations

- Only basic support for block-type data
- No support for writing detached data files

## License

This code is distributed under the Apache 2.0 license, consistent with ITK's licensing.