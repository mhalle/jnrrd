/*=========================================================================
 *
 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         https://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#include "itkJNRRDImageIO.h"
#include "itkMetaDataObject.h"
#include "itkByteSwapper.h"
#include "itksys/SystemTools.hxx"
#include "itkMacro.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <regex>

// Compression libraries
#include <zlib.h>
#ifdef JNRRD_USE_BZ2
#include <bzlib.h>
#endif
#ifdef JNRRD_USE_ZSTD
#include <zstd.h>
#endif
#ifdef JNRRD_USE_LZ4
#include <lz4frame.h>
#endif

namespace itk
{

JNRRDImageIO::JNRRDImageIO()
{
  // Initialize metadata dictionaries
  this->m_Header.clear();
  this->m_Extensions.clear();
  this->m_BinaryDataStart = 0;
  this->m_FileName = "";
  this->m_DataFileName = "";
  
  // By default, set pixel type to float
  this->SetPixelType(IOPixelType::SCALAR);
  this->SetComponentType(IOComponentType::FLOAT);
  this->SetNumberOfComponents(1);
  this->SetNumberOfDimensions(3);  // Default to 3D
  
  // Set metadata dictionary
  this->SetMetaDataDictionary();
}

JNRRDImageIO::~JNRRDImageIO() = default;

bool JNRRDImageIO::SupportsDimension(unsigned long dim)
{
  // JNRRD supports arbitrary dimension
  return true;
}

bool JNRRDImageIO::CanReadFile(const char * filename)
{
  // First check the filename extension
  std::string fname = filename;
  
  if (!this->HasSupportedReadExtension(fname.c_str(), false))
  {
    return false;
  }
  
  // Now check the file content
  std::ifstream file;
  file.open(filename, std::ios::in | std::ios::binary);
  if (!file.is_open())
  {
    return false;
  }
  
  // Read the first line - should be a JSON object with "jnrrd" key
  std::string line;
  std::getline(file, line);
  file.close();
  
  try
  {
    json jline = json::parse(line);
    // Check if it has the JNRRD magic line
    if (jline.contains("jnrrd"))
    {
      return true;
    }
  }
  catch (json::parse_error & e)
  {
    // Not a valid JSON
    return false;
  }
  
  return false;
}

bool JNRRDImageIO::CanWriteFile(const char * filename)
{
  // Check the extension
  std::string fname = filename;
  
  if (!this->HasSupportedWriteExtension(fname.c_str(), false))
  {
    return false;
  }
  
  return true;
}

bool JNRRDImageIO::ReadHeader()
{
  std::ifstream file;
  file.open(this->m_FileName.c_str(), std::ios::in | std::ios::binary);
  if (!file.is_open())
  {
    itkExceptionMacro(<< "Could not open file " << this->m_FileName);
  }
  
  // Read the header line by line
  while (file.good())
  {
    std::streampos lineStart = file.tellg();
    std::string line;
    std::getline(file, line);
    
    // Check if this is an empty line, which marks the end of the header
    if (line.empty())
    {
      this->m_BinaryDataStart = file.tellg();
      break;
    }
    
    // Try to parse the line as JSON
    try
    {
      json jline = json::parse(line);
      // Check if it's a valid JNRRD field (single key-value pair)
      if (jline.size() == 1)
      {
        ProcessHeaderField(jline);
      }
      else
      {
        // Invalid format - end of header
        this->m_BinaryDataStart = lineStart;
        break;
      }
    }
    catch (json::parse_error & e)
    {
      // Not a valid JSON, assume it's the start of binary data
      this->m_BinaryDataStart = lineStart;
      break;
    }
  }
  
  file.close();
  
  // Validate required fields
  if (!this->m_Header.count("jnrrd"))
  {
    itkExceptionMacro(<< "Missing required JNRRD magic field in header");
  }
  
  if (!this->m_Header.count("type"))
  {
    itkExceptionMacro(<< "Missing required 'type' field in header");
  }
  
  if (!this->m_Header.count("dimension"))
  {
    itkExceptionMacro(<< "Missing required 'dimension' field in header");
  }
  
  if (!this->m_Header.count("sizes"))
  {
    itkExceptionMacro(<< "Missing required 'sizes' field in header");
  }
  
  if (!this->m_Header.count("encoding"))
  {
    // Default to raw if not specified
    m_Header["encoding"] = json("raw");
  }
  
  // Check for detached data
  if (this->m_Header.count("data_file"))
  {
    this->m_DataFileName = this->m_Header["data_file"].get<std::string>();
  }
  
  return true;
}

void JNRRDImageIO::ProcessHeaderField(const json & field)
{
  // Get the first (and only) key-value pair
  for (auto it = field.begin(); it != field.end(); ++it)
  {
    std::string key = it.key();
    json value = it.value();
    
    // Check if this is an extension field (contains a colon)
    if (key.find(':') != std::string::npos)
    {
      HandleExtensionField(key, value);
    }
    else
    {
      // Regular header field
      this->m_Header[key] = value;
    }
  }
}

void JNRRDImageIO::HandleExtensionField(const std::string & key, const json & value)
{
  // Split the key into namespace and path
  size_t colonPos = key.find(':');
  std::string namespacePrefix = key.substr(0, colonPos);
  std::string path = key.substr(colonPos + 1);
  
  // Make sure the extensions map is initialized
  if (!this->m_Header.count("extensions"))
  {
    this->m_Header["extensions"] = json::object();
  }
  
  // Initialize the extension namespace if needed
  if (!this->m_Extensions.count(namespacePrefix))
  {
    this->m_Extensions[namespacePrefix] = json::object();
  }
  
  // Process hierarchical path
  ProcessHierarchicalPath(namespacePrefix, path, value);
}

void JNRRDImageIO::ProcessHierarchicalPath(const std::string & namespacePrefix, 
                                          const std::string & path, 
                                          const json & value)
{
  // Handle dots and brackets in the path
  // This is a simple implementation that doesn't handle all possible cases
  
  json & current = this->m_Extensions[namespacePrefix];
  
  // Use regex to extract path components
  std::regex pathRegex(R"(([^\.\[\]]+)|\[(\d+)\])");
  std::sregex_iterator it(path.begin(), path.end(), pathRegex);
  std::sregex_iterator end;
  
  // Track the previous matches to build the current path
  std::vector<std::string> pathComponents;
  
  for (auto i = it; i != end; ++i)
  {
    std::smatch match = *i;
    std::string component;
    bool isArrayIndex = false;
    int arrayIndex = -1;
    
    if (match[1].matched)
    {
      // Regular field name
      component = match[1].str();
    }
    else if (match[2].matched)
    {
      // Array index
      isArrayIndex = true;
      arrayIndex = std::stoi(match[2].str());
    }
    
    // Skip if empty
    if (component.empty() && !isArrayIndex)
    {
      continue;
    }
    
    // Add to path components
    if (isArrayIndex)
    {
      pathComponents.push_back("[" + std::to_string(arrayIndex) + "]");
    }
    else
    {
      pathComponents.push_back(component);
    }
    
    // Build current json pointer
    std::string jsonPointer;
    for (const auto & comp : pathComponents)
    {
      if (comp.front() == '[' && comp.back() == ']')
      {
        jsonPointer += comp;
      }
      else
      {
        jsonPointer += "/" + comp;
      }
    }
    
    // If this is the last component, set the value
    auto nextIt = i;
    ++nextIt;
    if (nextIt == end)
    {
      json::json_pointer ptr(jsonPointer);
      if (isArrayIndex)
      {
        // Make sure parent is an array of sufficient size
        json::json_pointer parentPtr = ptr.parent_pointer();
        if (!current.contains(parentPtr))
        {
          current[parentPtr] = json::array();
        }
        
        // Resize array if needed
        while (current[parentPtr].size() <= static_cast<size_t>(arrayIndex))
        {
          current[parentPtr].push_back(nullptr);
        }
      }
      
      current[ptr] = value;
    }
    else
    {
      // Ensure the intermediate object exists
      json::json_pointer ptr(jsonPointer);
      if (!current.contains(ptr))
      {
        if (isArrayIndex)
        {
          // Ensure parent is an array
          json::json_pointer parentPtr = ptr.parent_pointer();
          if (!current.contains(parentPtr))
          {
            current[parentPtr] = json::array();
          }
          
          // Resize array if needed
          while (current[parentPtr].size() <= static_cast<size_t>(arrayIndex))
          {
            current[parentPtr].push_back(nullptr);
          }
          
          current[ptr] = json::object();
        }
        else
        {
          current[ptr] = json::object();
        }
      }
    }
  }
}

void JNRRDImageIO::ReadImageInformation()
{
  this->m_FileName = this->GetFileName();
  
  if (!this->ReadHeader())
  {
    itkExceptionMacro(<< "Failed to read JNRRD header");
  }
  
  // Set component type based on "type" field
  std::string jnrrdType = this->m_Header["type"].get<std::string>();
  if (jnrrdType == "int8")
  {
    this->SetComponentType(IOComponentType::CHAR);
  }
  else if (jnrrdType == "uint8")
  {
    this->SetComponentType(IOComponentType::UCHAR);
  }
  else if (jnrrdType == "int16")
  {
    this->SetComponentType(IOComponentType::SHORT);
  }
  else if (jnrrdType == "uint16")
  {
    this->SetComponentType(IOComponentType::USHORT);
  }
  else if (jnrrdType == "int32")
  {
    this->SetComponentType(IOComponentType::INT);
  }
  else if (jnrrdType == "uint32")
  {
    this->SetComponentType(IOComponentType::UINT);
  }
  else if (jnrrdType == "int64")
  {
    this->SetComponentType(IOComponentType::LONGLONG);
  }
  else if (jnrrdType == "uint64")
  {
    this->SetComponentType(IOComponentType::ULONGLONG);
  }
  else if (jnrrdType == "float16" || jnrrdType == "bfloat16")
  {
    // ITK doesn't have direct half-precision float support, use float
    this->SetComponentType(IOComponentType::FLOAT);
  }
  else if (jnrrdType == "float32")
  {
    this->SetComponentType(IOComponentType::FLOAT);
  }
  else if (jnrrdType == "float64")
  {
    this->SetComponentType(IOComponentType::DOUBLE);
  }
  else if (jnrrdType == "complex64")
  {
    this->SetComponentType(IOComponentType::FLOAT);
    this->SetPixelType(IOPixelType::COMPLEX);
    this->SetNumberOfComponents(2);
  }
  else if (jnrrdType == "complex128")
  {
    this->SetComponentType(IOComponentType::DOUBLE);
    this->SetPixelType(IOPixelType::COMPLEX);
    this->SetNumberOfComponents(2);
  }
  else if (jnrrdType == "block")
  {
    itkExceptionMacro(<< "Block type not supported in ITK JNRRD reader");
  }
  else
  {
    itkExceptionMacro(<< "Unknown JNRRD type: " << jnrrdType);
  }
  
  // Set dimensions
  unsigned int dimension = this->m_Header["dimension"].get<unsigned int>();
  this->SetNumberOfDimensions(dimension);
  
  // Set sizes
  auto sizes = this->m_Header["sizes"].get<std::vector<unsigned int>>();
  if (sizes.size() != dimension)
  {
    itkExceptionMacro(<< "JNRRD sizes array length doesn't match dimension");
  }
  
  for (unsigned int i = 0; i < dimension; ++i)
  {
    this->SetDimensions(i, sizes[i]);
  }
  
  // Set spacing - can come from "spacings" or "space_directions"
  if (this->m_Header.count("spacings"))
  {
    auto spacings = this->m_Header["spacings"].get<std::vector<double>>();
    if (spacings.size() != dimension)
    {
      itkExceptionMacro(<< "JNRRD spacings array length doesn't match dimension");
    }
    
    for (unsigned int i = 0; i < dimension; ++i)
    {
      this->SetSpacing(i, spacings[i]);
    }
  }
  else if (this->m_Header.count("space_directions"))
  {
    // Need to compute spacing from space_directions
    this->ParseSpaceDirections();
  }
  else
  {
    // Default to spacing 1.0 in all dimensions
    for (unsigned int i = 0; i < dimension; ++i)
    {
      this->SetSpacing(i, 1.0);
    }
  }
  
  // Set origin - from "space_origin"
  if (this->m_Header.count("space_origin"))
  {
    this->ParseSpaceOrigin();
  }
  else
  {
    // Default to origin 0.0 in all dimensions
    for (unsigned int i = 0; i < dimension; ++i)
    {
      this->SetOrigin(i, 0.0);
    }
  }
  
  // Set direction - from "space_directions"
  if (this->m_Header.count("space_directions"))
  {
    // Parse space directions to calculate direction cosines
    auto space_directions = this->m_Header["space_directions"].get<std::vector<std::vector<double>>>();
    
    // Default to identity direction
    Matrix<double, 3, 3> directionMatrix;
    directionMatrix.SetIdentity();
    
    // Check if we need to handle the LPS to RAS conversion
    bool needsCoordinateSystemFlip = false;
    if (this->m_Header.count("space"))
    {
      std::string space = this->m_Header["space"].get<std::string>();
      if (space == "left-posterior-superior" || space == "LPS" || 
          space == "left_posterior_superior" || space == "left-posterior-superior")
      {
        needsCoordinateSystemFlip = true;
      }
    }
    
    // Calculate direction cosines from space_directions
    for (unsigned int i = 0; i < std::min(dimension, 3u); ++i)
    {
      if (space_directions[i].size() >= 3)
      {
        Vector<double, 3> v;
        v[0] = space_directions[i][0];
        v[1] = space_directions[i][1];
        v[2] = space_directions[i][2];
        
        double magnitude = v.GetNorm();
        if (magnitude > 0)
        {
          v /= magnitude;
        }
        
        // If we're converting from LPS to RAS, flip the x and y coordinates
        if (needsCoordinateSystemFlip)
        {
          v[0] = -v[0];
          v[1] = -v[1];
        }
        
        for (unsigned int j = 0; j < 3; ++j)
        {
          directionMatrix[j][i] = v[j];
        }
      }
    }
    
    // Set the direction matrix
    this->SetDirection(directionMatrix);
  }
  else
  {
    // Default to identity direction
    for (unsigned int i = 0; i < dimension; ++i)
    {
      std::vector<double> direction(dimension, 0.0);
      direction[i] = 1.0;
      this->SetDirection(i, direction);
    }
  }
  
  // Set metadata dictionary with all header fields
  auto & dict = this->GetMetaDataDictionary();
  
  // Add regular header fields
  for (const auto & item : this->m_Header)
  {
    std::string key = item.first;
    json value = item.second;
    
    // Store as string metadata
    if (value.is_string())
    {
      EncapsulateMetaData<std::string>(dict, key, value.get<std::string>());
    }
    else
    {
      // For non-string values, convert to string
      EncapsulateMetaData<std::string>(dict, key, value.dump());
    }
  }
  
  // Add extension fields
  for (const auto & ext : this->m_Extensions)
  {
    std::string namespacePrefix = ext.first;
    json extValue = ext.second;
    
    // Store the extensions object with its namespace prefix
    EncapsulateMetaData<std::string>(dict, "jnrrd_ext_" + namespacePrefix, extValue.dump());
  }
}

void JNRRDImageIO::ParseSpaceDirections()
{
  auto space_directions = this->m_Header["space_directions"].get<std::vector<std::vector<double>>>();
  unsigned int dimension = this->GetNumberOfDimensions();
  
  if (space_directions.size() != dimension)
  {
    itkExceptionMacro(<< "JNRRD space_directions array length doesn't match dimension");
  }
  
  // Compute spacing from magnitude of each direction vector
  for (unsigned int i = 0; i < dimension; ++i)
  {
    double sum = 0.0;
    for (unsigned int j = 0; j < space_directions[i].size(); ++j)
    {
      sum += space_directions[i][j] * space_directions[i][j];
    }
    double magnitude = std::sqrt(sum);
    this->SetSpacing(i, magnitude > 0 ? magnitude : 1.0);
  }
}

void JNRRDImageIO::ParseSpaceOrigin()
{
  auto space_origin = this->m_Header["space_origin"].get<std::vector<double>>();
  unsigned int dimension = this->GetNumberOfDimensions();
  
  // Check if we need to handle the LPS to RAS conversion
  bool needsCoordinateSystemFlip = false;
  if (this->m_Header.count("space"))
  {
    std::string space = this->m_Header["space"].get<std::string>();
    if (space == "left-posterior-superior" || space == "LPS" || 
        space == "left_posterior_superior" || space == "left-posterior-superior")
    {
      needsCoordinateSystemFlip = true;
    }
  }
  
  // Set origin values
  for (unsigned int i = 0; i < std::min(dimension, space_origin.size()); ++i)
  {
    double value = space_origin[i];
    
    // If we're converting from LPS to RAS, flip the x and y coordinates
    if (needsCoordinateSystemFlip && i < 2)
    {
      value = -value;
    }
    
    this->SetOrigin(i, value);
  }
  
  // Set any remaining dimensions to 0
  for (unsigned int i = space_origin.size(); i < dimension; ++i)
  {
    this->SetOrigin(i, 0.0);
  }
}

void JNRRDImageIO::Read(void * buffer)
{
  // Make sure header info is already read
  if (this->m_Header.empty())
  {
    this->ReadImageInformation();
  }
  
  // Read the binary data based on component type
  switch (this->GetComponentType())
  {
    case IOComponentType::CHAR:
      this->ReadDataAs<char>(buffer);
      break;
    case IOComponentType::UCHAR:
      this->ReadDataAs<unsigned char>(buffer);
      break;
    case IOComponentType::SHORT:
      this->ReadDataAs<short>(buffer);
      break;
    case IOComponentType::USHORT:
      this->ReadDataAs<unsigned short>(buffer);
      break;
    case IOComponentType::INT:
      this->ReadDataAs<int>(buffer);
      break;
    case IOComponentType::UINT:
      this->ReadDataAs<unsigned int>(buffer);
      break;
    case IOComponentType::LONG:
      this->ReadDataAs<long>(buffer);
      break;
    case IOComponentType::ULONG:
      this->ReadDataAs<unsigned long>(buffer);
      break;
    case IOComponentType::LONGLONG:
      this->ReadDataAs<long long>(buffer);
      break;
    case IOComponentType::ULONGLONG:
      this->ReadDataAs<unsigned long long>(buffer);
      break;
    case IOComponentType::FLOAT:
      this->ReadDataAs<float>(buffer);
      break;
    case IOComponentType::DOUBLE:
      this->ReadDataAs<double>(buffer);
      break;
    default:
      itkExceptionMacro(<< "Unknown component type: " << this->GetComponentTypeAsString(this->GetComponentType()));
  }
}

template <typename T>
void JNRRDImageIO::ReadDataAs(void * buffer)
{
  // Determine the file to read from (attached or detached data)
  std::string dataFile = this->m_FileName;
  std::streampos dataStart = this->m_BinaryDataStart;
  
  if (!this->m_DataFileName.empty())
  {
    // Reading from detached data file
    dataFile = this->m_DataFileName;
    
    // Check if it's a relative path
    if (!itksys::SystemTools::FileIsFullPath(dataFile.c_str()))
    {
      dataFile = itksys::SystemTools::GetFilenamePath(this->m_FileName) + "/" + dataFile;
    }
    
    // Start at beginning of detached file
    dataStart = 0;
    
    // Handle line skip
    if (this->m_Header.count("line_skip"))
    {
      int lineSkip = this->m_Header["line_skip"].get<int>();
      
      std::ifstream file(dataFile.c_str(), std::ios::in | std::ios::binary);
      if (!file.is_open())
      {
        itkExceptionMacro(<< "Could not open detached data file: " << dataFile);
      }
      
      for (int i = 0; i < lineSkip; ++i)
      {
        std::string line;
        std::getline(file, line);
      }
      
      dataStart = file.tellg();
      file.close();
    }
    
    // Handle byte skip
    if (this->m_Header.count("byte_skip"))
    {
      dataStart += this->m_Header["byte_skip"].get<std::streampos>();
    }
  }
  
  // Get the encoding type
  std::string encoding = this->m_Header["encoding"].get<std::string>();
  
  if (encoding == "raw")
  {
    // Read raw binary data
    std::ifstream file(dataFile.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
      itkExceptionMacro(<< "Could not open file for reading: " << dataFile);
    }
    
    file.seekg(dataStart);
    
    // Calculate buffer size
    size_t elementSize = sizeof(T);
    size_t componentCount = this->GetNumberOfComponents();
    size_t totalElements = this->GetImageSizeInPixels();
    size_t totalBytes = totalElements * componentCount * elementSize;
    
    // Read the data
    file.read(static_cast<char *>(buffer), totalBytes);
    
    // Check if we read the expected amount
    if (static_cast<size_t>(file.gcount()) != totalBytes)
    {
      itkExceptionMacro(<< "File size doesn't match expected size. Read " << file.gcount() << " instead of " << totalBytes);
    }
    
    file.close();
    
    // Handle endianness
    if (this->m_Header.count("endian"))
    {
      std::string endian = this->m_Header["endian"].get<std::string>();
      
      // Swap bytes if necessary
      // (if file endianness doesn't match machine endianness)
      bool shouldSwap = false;
      
      // Check machine endianness
#ifdef JNRRD_BIG_ENDIAN
      bool machineIsBigEndian = true;
#else
      bool machineIsBigEndian = false;
#endif
      
      if ((endian == "big" && !machineIsBigEndian) || (endian == "little" && machineIsBigEndian))
      {
        shouldSwap = true;
      }
      
      if (shouldSwap)
      {
        ByteSwapper<T>::SwapRangeFromSystemToBigEndian(static_cast<T *>(buffer), totalElements * componentCount);
      }
    }
  }
  else
  {
    // Handle compressed data
    this->ReadCompressedData(buffer);
  }
}

void JNRRDImageIO::ReadCompressedData(void * buffer)
{
  // Get encoding type
  std::string encoding = this->m_Header["encoding"].get<std::string>();
  
  // Determine the file to read from (attached or detached data)
  std::string dataFile = this->m_FileName;
  std::streampos dataStart = this->m_BinaryDataStart;
  
  if (!this->m_DataFileName.empty())
  {
    // Reading from detached data file
    dataFile = this->m_DataFileName;
    
    // Check if it's a relative path
    if (!itksys::SystemTools::FileIsFullPath(dataFile.c_str()))
    {
      dataFile = itksys::SystemTools::GetFilenamePath(this->m_FileName) + "/" + dataFile;
    }
    
    // Start at beginning of detached file (after handling skips)
    dataStart = 0;
    
    // Handle line skip
    if (this->m_Header.count("line_skip"))
    {
      int lineSkip = this->m_Header["line_skip"].get<int>();
      
      std::ifstream file(dataFile.c_str(), std::ios::in | std::ios::binary);
      if (!file.is_open())
      {
        itkExceptionMacro(<< "Could not open detached data file: " << dataFile);
      }
      
      for (int i = 0; i < lineSkip; ++i)
      {
        std::string line;
        std::getline(file, line);
      }
      
      dataStart = file.tellg();
      file.close();
    }
    
    // Handle byte skip
    if (this->m_Header.count("byte_skip"))
    {
      dataStart += this->m_Header["byte_skip"].get<std::streampos>();
    }
  }
  
  // Read the compressed data
  std::ifstream file(dataFile.c_str(), std::ios::in | std::ios::binary);
  if (!file.is_open())
  {
    itkExceptionMacro(<< "Could not open file for reading: " << dataFile);
  }
  
  file.seekg(0, std::ios::end);
  size_t fileSize = static_cast<size_t>(file.tellg()) - static_cast<size_t>(dataStart);
  file.seekg(dataStart);
  
  // Read the compressed data into a buffer
  auto compressedData = new unsigned char[fileSize];
  file.read(reinterpret_cast<char *>(compressedData), fileSize);
  file.close();
  
  // Calculate uncompressed size
  size_t elementSize = this->GetComponentSize();
  size_t componentCount = this->GetNumberOfComponents();
  size_t totalElements = this->GetImageSizeInPixels();
  size_t uncompressedSize = totalElements * componentCount * elementSize;
  
  // Decompress based on encoding
  if (encoding == "gzip" || encoding == "gz")
  {
    z_stream zs;
    memset(&zs, 0, sizeof(zs));
    
    if (inflateInit2(&zs, 15 + 32) != Z_OK)
    {
      delete[] compressedData;
      itkExceptionMacro(<< "Failed to initialize zlib inflate");
    }
    
    zs.next_in = reinterpret_cast<Bytef *>(compressedData);
    zs.avail_in = fileSize;
    zs.next_out = reinterpret_cast<Bytef *>(buffer);
    zs.avail_out = uncompressedSize;
    
    int ret = inflate(&zs, Z_FINISH);
    inflateEnd(&zs);
    
    if (ret != Z_STREAM_END)
    {
      delete[] compressedData;
      itkExceptionMacro(<< "Failed to decompress gzip data");
    }
    
    if (zs.total_out != uncompressedSize)
    {
      delete[] compressedData;
      itkExceptionMacro(<< "Uncompressed size doesn't match expected size");
    }
  }
#ifdef JNRRD_USE_BZ2
  else if (encoding == "bzip2" || encoding == "bz2")
  {
    bz_stream bzs;
    memset(&bzs, 0, sizeof(bzs));
    
    if (BZ2_bzDecompressInit(&bzs, 0, 0) != BZ_OK)
    {
      delete[] compressedData;
      itkExceptionMacro(<< "Failed to initialize bzip2 decompress");
    }
    
    bzs.next_in = reinterpret_cast<char *>(compressedData);
    bzs.avail_in = fileSize;
    bzs.next_out = reinterpret_cast<char *>(buffer);
    bzs.avail_out = uncompressedSize;
    
    int ret = BZ2_bzDecompress(&bzs);
    BZ2_bzDecompressEnd(&bzs);
    
    if (ret != BZ_STREAM_END)
    {
      delete[] compressedData;
      itkExceptionMacro(<< "Failed to decompress bzip2 data");
    }
    
    if (static_cast<size_t>(bzs.total_out_lo32) != uncompressedSize)
    {
      delete[] compressedData;
      itkExceptionMacro(<< "Uncompressed size doesn't match expected size");
    }
  }
#endif
#ifdef JNRRD_USE_ZSTD
  else if (encoding == "zstd")
  {
    size_t result = ZSTD_decompress(buffer, uncompressedSize, compressedData, fileSize);
    
    if (ZSTD_isError(result))
    {
      delete[] compressedData;
      itkExceptionMacro(<< "Failed to decompress zstd data: " << ZSTD_getErrorName(result));
    }
    
    if (result != uncompressedSize)
    {
      delete[] compressedData;
      itkExceptionMacro(<< "Uncompressed size doesn't match expected size");
    }
  }
#endif
#ifdef JNRRD_USE_LZ4
  else if (encoding == "lz4")
  {
    LZ4F_dctx * dctx;
    LZ4F_createDecompressionContext(&dctx, LZ4F_VERSION);
    
    size_t srcSize = fileSize;
    size_t dstSize = uncompressedSize;
    
    LZ4F_decompressOptions_t options = {};
    LZ4F_errorCode_t result = LZ4F_decompress(dctx, buffer, &dstSize, compressedData, &srcSize, &options);
    
    LZ4F_freeDecompressionContext(dctx);
    
    if (LZ4F_isError(result))
    {
      delete[] compressedData;
      itkExceptionMacro(<< "Failed to decompress lz4 data: " << LZ4F_getErrorName(result));
    }
    
    if (dstSize != uncompressedSize)
    {
      delete[] compressedData;
      itkExceptionMacro(<< "Uncompressed size doesn't match expected size");
    }
  }
#endif
  else
  {
    delete[] compressedData;
    itkExceptionMacro(<< "Unsupported encoding: " << encoding);
  }
  
  delete[] compressedData;
  
  // Handle endianness
  if (this->m_Header.count("endian"))
  {
    std::string endian = this->m_Header["endian"].get<std::string>();
    
    // Swap bytes if necessary
    // (if file endianness doesn't match machine endianness)
    bool shouldSwap = false;
    
    // Check machine endianness
#ifdef JNRRD_BIG_ENDIAN
    bool machineIsBigEndian = true;
#else
    bool machineIsBigEndian = false;
#endif
    
    if ((endian == "big" && !machineIsBigEndian) || (endian == "little" && machineIsBigEndian))
    {
      shouldSwap = true;
    }
    
    if (shouldSwap)
    {
      switch (this->GetComponentType())
      {
        case IOComponentType::CHAR:
          ByteSwapper<char>::SwapRangeFromSystemToBigEndian(
            static_cast<char *>(buffer), this->GetImageSizeInComponents());
          break;
        case IOComponentType::UCHAR:
          ByteSwapper<unsigned char>::SwapRangeFromSystemToBigEndian(
            static_cast<unsigned char *>(buffer), this->GetImageSizeInComponents());
          break;
        case IOComponentType::SHORT:
          ByteSwapper<short>::SwapRangeFromSystemToBigEndian(
            static_cast<short *>(buffer), this->GetImageSizeInComponents());
          break;
        case IOComponentType::USHORT:
          ByteSwapper<unsigned short>::SwapRangeFromSystemToBigEndian(
            static_cast<unsigned short *>(buffer), this->GetImageSizeInComponents());
          break;
        case IOComponentType::INT:
          ByteSwapper<int>::SwapRangeFromSystemToBigEndian(
            static_cast<int *>(buffer), this->GetImageSizeInComponents());
          break;
        case IOComponentType::UINT:
          ByteSwapper<unsigned int>::SwapRangeFromSystemToBigEndian(
            static_cast<unsigned int *>(buffer), this->GetImageSizeInComponents());
          break;
        case IOComponentType::LONG:
          ByteSwapper<long>::SwapRangeFromSystemToBigEndian(
            static_cast<long *>(buffer), this->GetImageSizeInComponents());
          break;
        case IOComponentType::ULONG:
          ByteSwapper<unsigned long>::SwapRangeFromSystemToBigEndian(
            static_cast<unsigned long *>(buffer), this->GetImageSizeInComponents());
          break;
        case IOComponentType::LONGLONG:
          ByteSwapper<long long>::SwapRangeFromSystemToBigEndian(
            static_cast<long long *>(buffer), this->GetImageSizeInComponents());
          break;
        case IOComponentType::ULONGLONG:
          ByteSwapper<unsigned long long>::SwapRangeFromSystemToBigEndian(
            static_cast<unsigned long long *>(buffer), this->GetImageSizeInComponents());
          break;
        case IOComponentType::FLOAT:
          ByteSwapper<float>::SwapRangeFromSystemToBigEndian(
            static_cast<float *>(buffer), this->GetImageSizeInComponents());
          break;
        case IOComponentType::DOUBLE:
          ByteSwapper<double>::SwapRangeFromSystemToBigEndian(
            static_cast<double *>(buffer), this->GetImageSizeInComponents());
          break;
        default:
          itkExceptionMacro(<< "Unknown component type: " << this->GetComponentTypeAsString(this->GetComponentType()));
      }
    }
  }
}

void JNRRDImageIO::Write(const void * buffer)
{
  // Prepare the header data
  PrepareHeaderForWrite();
  
  // Open file for writing
  std::ofstream file;
  file.open(this->GetFileName(), std::ios::out | std::ios::binary);
  if (!file.is_open())
  {
    itkExceptionMacro(<< "Could not open file for writing: " << this->GetFileName());
  }
  
  // Write header fields
  WriteHeaderToFile(file);
  
  // Add an empty line to separate header from data
  file.write("\n", 1);
  
  // Write binary data
  WriteDataToFile(buffer, file);
  
  file.close();
}

void JNRRDImageIO::PrepareHeaderForWrite()
{
  // Clear any existing header data
  this->m_Header.clear();
  this->m_Extensions.clear();
  
  // Required fields
  this->m_Header["jnrrd"] = "0004";
  this->m_Header["dimension"] = this->GetNumberOfDimensions();
  
  // Set data type
  this->m_Header["type"] = GetJNRRDTypeString(this->GetComponentType());
  
  // Set sizes
  std::vector<unsigned int> sizes;
  for (unsigned int i = 0; i < this->GetNumberOfDimensions(); ++i)
  {
    sizes.push_back(this->GetDimensions(i));
  }
  this->m_Header["sizes"] = sizes;
  
  // Set encoding - default to raw
  this->m_Header["encoding"] = "raw";
  
  // Set endianness based on machine architecture
#ifdef JNRRD_BIG_ENDIAN
  this->m_Header["endian"] = "big";
#else
  this->m_Header["endian"] = "little";
#endif
  
  // Set space information if available
  if (this->GetNumberOfDimensions() <= 3)
  {
    if (this->GetNumberOfDimensions() == 3)
    {
      this->m_Header["space"] = "right_anterior_superior"; // Default to RAS
    }
    else if (this->GetNumberOfDimensions() == 2)
    {
      this->m_Header["space"] = "right_anterior"; // 2D default
    }
    
    // Generate space directions from ITK direction and spacing
    GenerateSpaceDirections();
    
    // Set origin
    std::vector<double> origin;
    for (unsigned int i = 0; i < this->GetNumberOfDimensions(); ++i)
    {
      origin.push_back(this->GetOrigin(i));
    }
    this->m_Header["space_origin"] = origin;
    
    // Set spacings
    std::vector<double> spacings;
    for (unsigned int i = 0; i < this->GetNumberOfDimensions(); ++i)
    {
      spacings.push_back(this->GetSpacing(i));
    }
    this->m_Header["spacings"] = spacings;
  }
  
  // Transfer metadata from ITK dictionary to JNRRD header
  const MetaDataDictionary & dict = this->GetMetaDataDictionary();
  for (auto iter = dict.Begin(); iter != dict.End(); ++iter)
  {
    std::string key = iter->first;
    
    // Skip internal ITK keys
    if (key.substr(0, 4) == "ITK_")
    {
      continue;
    }
    
    // Handle extension fields
    if (key.find("jnrrd_ext_") == 0)
    {
      std::string extName = key.substr(10); // remove "jnrrd_ext_"
      std::string value;
      ExposeMetaData<std::string>(dict, key, value);
      try
      {
        // Parse the JSON string and add it to the extensions map
        this->m_Extensions[extName] = json::parse(value);
      }
      catch (json::parse_error & e)
      {
        itkWarningMacro(<< "Failed to parse extension JSON for key: " << key);
      }
    }
    else
    {
      // Try to get regular string metadata
      std::string value;
      if (ExposeMetaData<std::string>(dict, key, value))
      {
        try
        {
          // Try to parse as JSON first (for structured metadata)
          this->m_Header[key] = json::parse(value);
        }
        catch (json::parse_error & e)
        {
          // Not valid JSON, store as string
          this->m_Header[key] = value;
        }
      }
    }
  }
  
  // If we have extensions, add them to header
  if (!this->m_Extensions.empty())
  {
    json extensions = json::object();
    for (const auto & ext : this->m_Extensions)
    {
      // Add a placeholder URL for each extension
      extensions[ext.first] = "https://jnrrd.org/extensions/" + ext.first + "/v1.0.0";
    }
    this->m_Header["extensions"] = extensions;
  }
}

void JNRRDImageIO::GenerateSpaceDirections()
{
  unsigned int dimensions = this->GetNumberOfDimensions();
  unsigned int spaceDimensions = std::min(dimensions, 3u); // Max 3D space
  
  // Create nested array for space_directions
  std::vector<std::vector<double>> spaceDirections;
  
  // Handle direction and spacing
  for (unsigned int i = 0; i < dimensions; ++i)
  {
    std::vector<double> dirVector;
    
    // For spatial dimensions
    if (i < spaceDimensions)
    {
      for (unsigned int j = 0; j < spaceDimensions; ++j)
      {
        double direction = 0.0;
        if (j < this->GetDirection(i).size())
        {
          direction = this->GetDirection(i)[j];
        }
        else if (i == j)
        {
          direction = 1.0; // identity for missing values
        }
        
        // Scale by spacing
        dirVector.push_back(direction * this->GetSpacing(i));
      }
    }
    else
    {
      // For non-spatial dimensions (like time), use null
      dirVector = std::vector<double>(); // empty vector represents null in JSON
    }
    
    spaceDirections.push_back(dirVector);
  }
  
  this->m_Header["space_directions"] = spaceDirections;
}

std::string JNRRDImageIO::GetJNRRDTypeString(IOComponentType componentType)
{
  switch (componentType)
  {
    case IOComponentType::CHAR:
      return "int8";
    case IOComponentType::UCHAR:
      return "uint8";
    case IOComponentType::SHORT:
      return "int16";
    case IOComponentType::USHORT:
      return "uint16";
    case IOComponentType::INT:
      return "int32";
    case IOComponentType::UINT:
      return "uint32";
    case IOComponentType::LONG:
      if (sizeof(long) == 4)
        return "int32";
      else
        return "int64";
    case IOComponentType::ULONG:
      if (sizeof(unsigned long) == 4)
        return "uint32";
      else
        return "uint64";
    case IOComponentType::LONGLONG:
      return "int64";
    case IOComponentType::ULONGLONG:
      return "uint64";
    case IOComponentType::FLOAT:
      return "float32";
    case IOComponentType::DOUBLE:
      return "float64";
    default:
      itkExceptionMacro(<< "Unsupported component type: " << this->GetComponentTypeAsString(componentType));
      return "float32"; // default
  }
}

void JNRRDImageIO::WriteHeaderToFile(std::ofstream & file)
{
  // Write JNRRD magic line first
  if (this->m_Header.count("jnrrd"))
  {
    json magic = {{"jnrrd", this->m_Header["jnrrd"]}};
    file << magic.dump() << "\n";
  }
  
  // Write other regular header fields
  for (const auto & item : this->m_Header)
  {
    // Skip the jnrrd field since we've already written it
    if (item.first == "jnrrd")
    {
      continue;
    }
    
    json field = {{item.first, item.second}};
    file << field.dump() << "\n";
  }
  
  // Write extension fields
  for (const auto & ext : this->m_Extensions)
  {
    std::string namespacePrefix = ext.first;
    json extValue = ext.second;
    
    // For nested objects, write them flat with namespace prefix
    WriteExtensionToFile(file, namespacePrefix, extValue);
  }
}

void JNRRDImageIO::WriteExtensionToFile(std::ofstream & file, const std::string & prefix, const json & value, const std::string & path)
{
  if (value.is_object())
  {
    // For objects, recurse into their properties
    for (auto it = value.begin(); it != value.end(); ++it)
    {
      std::string newPath = path.empty() ? it.key() : path + "." + it.key();
      WriteExtensionToFile(file, prefix, it.value(), newPath);
    }
  }
  else if (value.is_array())
  {
    // Either write as a complete array or individual elements
    bool writeAsWhole = true;
    
    // Check if any element is an object or array - if so, flatten
    for (const auto & element : value)
    {
      if (element.is_object() || element.is_array())
      {
        writeAsWhole = false;
        break;
      }
    }
    
    if (writeAsWhole)
    {
      // Write whole array
      std::string key = prefix + ":" + path;
      json field = {{key, value}};
      file << field.dump() << "\n";
    }
    else
    {
      // Write elements individually
      for (size_t i = 0; i < value.size(); ++i)
      {
        std::string newPath = path + "[" + std::to_string(i) + "]";
        WriteExtensionToFile(file, prefix, value[i], newPath);
      }
    }
  }
  else
  {
    // Leaf node, write as simple key-value
    std::string key = prefix + ":" + path;
    json field = {{key, value}};
    file << field.dump() << "\n";
  }
}

void JNRRDImageIO::WriteDataToFile(const void * buffer, std::ofstream & file)
{
  std::string encoding = this->m_Header["encoding"].get<std::string>();
  
  if (encoding == "raw")
  {
    // Handle raw data writing based on component type
    switch (this->GetComponentType())
    {
      case IOComponentType::CHAR:
        this->WriteDataAs<char>(buffer, file);
        break;
      case IOComponentType::UCHAR:
        this->WriteDataAs<unsigned char>(buffer, file);
        break;
      case IOComponentType::SHORT:
        this->WriteDataAs<short>(buffer, file);
        break;
      case IOComponentType::USHORT:
        this->WriteDataAs<unsigned short>(buffer, file);
        break;
      case IOComponentType::INT:
        this->WriteDataAs<int>(buffer, file);
        break;
      case IOComponentType::UINT:
        this->WriteDataAs<unsigned int>(buffer, file);
        break;
      case IOComponentType::LONG:
        this->WriteDataAs<long>(buffer, file);
        break;
      case IOComponentType::ULONG:
        this->WriteDataAs<unsigned long>(buffer, file);
        break;
      case IOComponentType::LONGLONG:
        this->WriteDataAs<long long>(buffer, file);
        break;
      case IOComponentType::ULONGLONG:
        this->WriteDataAs<unsigned long long>(buffer, file);
        break;
      case IOComponentType::FLOAT:
        this->WriteDataAs<float>(buffer, file);
        break;
      case IOComponentType::DOUBLE:
        this->WriteDataAs<double>(buffer, file);
        break;
      default:
        itkExceptionMacro(<< "Unknown component type: " << this->GetComponentTypeAsString(this->GetComponentType()));
    }
  }
  else
  {
    // Handle compressed formats
    WriteCompressedData(buffer, file);
  }
}

template <typename T>
void JNRRDImageIO::WriteDataAs(const void * buffer, std::ofstream & file)
{
  // Calculate buffer size
  size_t elementSize = sizeof(T);
  size_t componentCount = this->GetNumberOfComponents();
  size_t totalElements = this->GetImageSizeInPixels();
  size_t totalBytes = totalElements * componentCount * elementSize;
  
  // Handle endianness
  std::string endian = this->m_Header["endian"].get<std::string>();
  bool shouldSwap = false;
  
  // Check machine endianness
#ifdef JNRRD_BIG_ENDIAN
  bool machineIsBigEndian = true;
#else
  bool machineIsBigEndian = false;
#endif
  
  if ((endian == "big" && !machineIsBigEndian) || (endian == "little" && machineIsBigEndian))
  {
    shouldSwap = true;
  }
  
  if (shouldSwap)
  {
    // Create a copy of the buffer for byte swapping
    T * tempBuffer = new T[totalElements * componentCount];
    memcpy(tempBuffer, buffer, totalBytes);
    
    // Swap bytes in the copy
    ByteSwapper<T>::SwapRangeFromSystemToBigEndian(tempBuffer, totalElements * componentCount);
    
    // Write the swapped data
    file.write(reinterpret_cast<const char *>(tempBuffer), totalBytes);
    
    // Clean up
    delete[] tempBuffer;
  }
  else
  {
    // Write the data directly
    file.write(static_cast<const char *>(buffer), totalBytes);
  }
}

void JNRRDImageIO::WriteCompressedData(const void * buffer, std::ofstream & file)
{
  // Calculate data size
  size_t elementSize = this->GetComponentSize();
  size_t componentCount = this->GetNumberOfComponents();
  size_t totalElements = this->GetImageSizeInPixels();
  size_t dataSize = totalElements * componentCount * elementSize;
  
  // Get the raw data, properly byte-swapped if needed
  const unsigned char * rawData = static_cast<const unsigned char *>(buffer);
  std::vector<unsigned char> swappedData;
  
  // Handle endianness
  std::string endian = this->m_Header["endian"].get<std::string>();
  bool shouldSwap = false;
  
  // Check machine endianness
#ifdef JNRRD_BIG_ENDIAN
  bool machineIsBigEndian = true;
#else
  bool machineIsBigEndian = false;
#endif
  
  if ((endian == "big" && !machineIsBigEndian) || (endian == "little" && machineIsBigEndian))
  {
    shouldSwap = true;
  }
  
  if (shouldSwap)
  {
    // Create a copy for byte swapping
    swappedData.resize(dataSize);
    memcpy(swappedData.data(), rawData, dataSize);
    
    // Swap bytes based on component type
    switch (this->GetComponentType())
    {
      case IOComponentType::CHAR:
        ByteSwapper<char>::SwapRangeFromSystemToBigEndian(
          reinterpret_cast<char *>(swappedData.data()), totalElements * componentCount);
        break;
      case IOComponentType::UCHAR:
        // No swapping needed for single byte
        break;
      case IOComponentType::SHORT:
        ByteSwapper<short>::SwapRangeFromSystemToBigEndian(
          reinterpret_cast<short *>(swappedData.data()), totalElements * componentCount);
        break;
      case IOComponentType::USHORT:
        ByteSwapper<unsigned short>::SwapRangeFromSystemToBigEndian(
          reinterpret_cast<unsigned short *>(swappedData.data()), totalElements * componentCount);
        break;
      case IOComponentType::INT:
        ByteSwapper<int>::SwapRangeFromSystemToBigEndian(
          reinterpret_cast<int *>(swappedData.data()), totalElements * componentCount);
        break;
      case IOComponentType::UINT:
        ByteSwapper<unsigned int>::SwapRangeFromSystemToBigEndian(
          reinterpret_cast<unsigned int *>(swappedData.data()), totalElements * componentCount);
        break;
      case IOComponentType::LONG:
        ByteSwapper<long>::SwapRangeFromSystemToBigEndian(
          reinterpret_cast<long *>(swappedData.data()), totalElements * componentCount);
        break;
      case IOComponentType::ULONG:
        ByteSwapper<unsigned long>::SwapRangeFromSystemToBigEndian(
          reinterpret_cast<unsigned long *>(swappedData.data()), totalElements * componentCount);
        break;
      case IOComponentType::LONGLONG:
        ByteSwapper<long long>::SwapRangeFromSystemToBigEndian(
          reinterpret_cast<long long *>(swappedData.data()), totalElements * componentCount);
        break;
      case IOComponentType::ULONGLONG:
        ByteSwapper<unsigned long long>::SwapRangeFromSystemToBigEndian(
          reinterpret_cast<unsigned long long *>(swappedData.data()), totalElements * componentCount);
        break;
      case IOComponentType::FLOAT:
        ByteSwapper<float>::SwapRangeFromSystemToBigEndian(
          reinterpret_cast<float *>(swappedData.data()), totalElements * componentCount);
        break;
      case IOComponentType::DOUBLE:
        ByteSwapper<double>::SwapRangeFromSystemToBigEndian(
          reinterpret_cast<double *>(swappedData.data()), totalElements * componentCount);
        break;
      default:
        itkExceptionMacro(<< "Unknown component type: " << this->GetComponentTypeAsString(this->GetComponentType()));
    }
    
    // Use swapped data
    rawData = swappedData.data();
  }
  
  // Get encoding
  std::string encoding = this->m_Header["encoding"].get<std::string>();
  
  // Compress based on encoding
  if (encoding == "gzip" || encoding == "gz")
  {
    // Create a buffer for compressed data - worst case size
    uLongf compressedSize = compressBound(dataSize);
    std::vector<Bytef> compressedData(compressedSize);
    
    // Compress the data
    int ret = compress2(compressedData.data(), &compressedSize, 
                      static_cast<const Bytef *>(rawData), dataSize, 
                      Z_BEST_COMPRESSION);
    
    if (ret != Z_OK)
    {
      itkExceptionMacro(<< "Failed to compress with gzip");
    }
    
    // Write the compressed data
    file.write(reinterpret_cast<const char *>(compressedData.data()), compressedSize);
  }
#ifdef JNRRD_USE_BZ2
  else if (encoding == "bzip2" || encoding == "bz2")
  {
    // Create a buffer for compressed data - worst case size
    unsigned int compressedSize = static_cast<unsigned int>(
      dataSize * 1.01 + 600); // BZ2 worst case estimate
    std::vector<char> compressedData(compressedSize);
    
    // Compress the data
    int ret = BZ2_bzBuffToBuffCompress(
      compressedData.data(), &compressedSize,
      const_cast<char *>(reinterpret_cast<const char *>(rawData)), static_cast<unsigned int>(dataSize),
      9, 0, 0); // block size, verbosity, work factor
    
    if (ret != BZ_OK)
    {
      itkExceptionMacro(<< "Failed to compress with bzip2");
    }
    
    // Write the compressed data
    file.write(compressedData.data(), compressedSize);
  }
#endif
#ifdef JNRRD_USE_ZSTD
  else if (encoding == "zstd")
  {
    // Get the maximum compressed size
    size_t compressedSize = ZSTD_compressBound(dataSize);
    std::vector<char> compressedData(compressedSize);
    
    // Compress the data
    compressedSize = ZSTD_compress(
      compressedData.data(), compressedSize,
      rawData, dataSize,
      ZSTD_maxCLevel()); // Maximum compression level
    
    if (ZSTD_isError(compressedSize))
    {
      itkExceptionMacro(<< "Failed to compress with zstd: " << ZSTD_getErrorName(compressedSize));
    }
    
    // Write the compressed data
    file.write(compressedData.data(), compressedSize);
  }
#endif
#ifdef JNRRD_USE_LZ4
  else if (encoding == "lz4")
  {
    // Create the LZ4 preferences
    LZ4F_preferences_t prefs = LZ4F_INIT_PREFERENCES;
    prefs.compressionLevel = LZ4F_compressionLevel_max(); // Maximum compression
    
    // Get the maximum compressed size
    size_t compressedSize = LZ4F_compressFrameBound(dataSize, &prefs);
    std::vector<char> compressedData(compressedSize);
    
    // Compress the data
    compressedSize = LZ4F_compressFrame(
      compressedData.data(), compressedSize,
      rawData, dataSize,
      &prefs);
    
    if (LZ4F_isError(compressedSize))
    {
      itkExceptionMacro(<< "Failed to compress with lz4: " << LZ4F_getErrorName(compressedSize));
    }
    
    // Write the compressed data
    file.write(compressedData.data(), compressedSize);
  }
#endif
  else
  {
    itkExceptionMacro(<< "Unsupported encoding: " << encoding);
  }
}

void JNRRDImageIO::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  
  os << indent << "FileName: " << this->m_FileName << std::endl;
  os << indent << "DataFileName: " << this->m_DataFileName << std::endl;
  os << indent << "BinaryDataStart: " << this->m_BinaryDataStart << std::endl;
  
  os << indent << "Header Fields: " << std::endl;
  Indent indent2 = indent.GetNextIndent();
  
  for (const auto & item : this->m_Header)
  {
    os << indent2 << item.first << ": ";
    
    if (item.second.is_string())
    {
      os << item.second.get<std::string>();
    }
    else
    {
      os << item.second.dump();
    }
    
    os << std::endl;
  }
  
  os << indent << "Extensions: " << std::endl;
  
  for (const auto & ext : this->m_Extensions)
  {
    os << indent2 << ext.first << ": " << ext.second.dump() << std::endl;
  }
}

} // end namespace itk