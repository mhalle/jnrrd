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
#ifndef itkJNRRDImageIO_h
#define itkJNRRDImageIO_h

#include "itkImageIOBase.h"
#include <fstream>
#include <string>
#include <map>
#include <vector>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

namespace itk
{
/**
 * \class JNRRDImageIO
 * \brief ImageIO object for reading and writing JNRRD (JSON-based Nearly Raw Raster Data) images
 *
 * This class reads and writes JNRRD format files, which is a JSON-based evolution of the
 * original NRRD format from the Teem project.
 *
 * The format consists of a header section with line-delimited JSON objects, followed by
 * a binary data section. The header describes the dimensions, data type, coordinate
 * transformations, and other metadata.
 *
 * The JNRRD format supports extensions through a namespaced prefix mechanism, allowing
 * domain-specific metadata (DICOM, NIfTI, etc.) to be included in the same file.
 *
 * \ingroup IOFilters
 */
class ITK_TEMPLATE_EXPORT JNRRDImageIO : public ImageIOBase
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(JNRRDImageIO);

  /** Standard class type aliases. */
  using Self = JNRRDImageIO;
  using Superclass = ImageIOBase;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(JNRRDImageIO, ImageIOBase);

  /** Determine whether the file can be read with this ImageIO implementation */
  bool CanReadFile(const char * filename) override;

  /** Determine whether the file can be written with this ImageIO implementation */
  bool CanWriteFile(const char * filename) override;

  /** Set the spacing and dimension information for the set filename */
  void ReadImageInformation() override;

  /** Reads the data from disk into the memory buffer provided */
  void Read(void * buffer) override;

  /** Writes the data from the memory buffer provided */
  void Write(const void * buffer) override;

  /** Get the extension to use for JNRRD files */
  std::string GetFileExtensions() const override { return ".jnrrd"; }

  /** Get a suitable description of this Writer */
  std::string GetDescription() const override { return "JNRRD JSON-based Nearly Raw Raster Data"; }

  /** Get whether the given filename is a JNRRD file */
  bool SupportsDimension(unsigned long dim) override;

  /** Get the list of file name extensions supported by this ImageIO */
  ArrayViewParameter<const std::string> GetSupportedFileExtensions() const override
  {
    return { &m_FileExtensions[0], m_FileExtensions.size() };
  }

protected:
  JNRRDImageIO();
  ~JNRRDImageIO() override;

  void PrintSelf(std::ostream & os, Indent indent) const override;

private:
  // Reading methods
  bool ReadHeader();
  void ReadHeaderLine(std::ifstream & inputStream);
  void ProcessHeaderField(const json & field);
  void ParseSpaceDirections();
  void ParseSpaceOrigin();
  void SetupPixelType();
  
  template <typename T>
  void ReadDataAs(void * buffer);
  
  // Helper for reading compressed data
  void ReadCompressedData(void * buffer);
  
  // Handle extension fields
  void HandleExtensionField(const std::string & key, const json & value);
  
  // Process hierarchical path in extension field
  void ProcessHierarchicalPath(const std::string & namespace_prefix, 
                              const std::string & path, 
                              const json & value);
  
  // Writing methods
  void PrepareHeaderForWrite();
  void WriteHeaderToFile(std::ofstream & file);
  void WriteExtensionToFile(std::ofstream & file, const std::string & prefix, const json & value, const std::string & path = "");
  void WriteDataToFile(const void * buffer, std::ofstream & file);
  
  // Helper for writing compressed data
  template <typename T>
  void WriteDataAs(const void * buffer, std::ofstream & file);
  void WriteCompressedData(const void * buffer, std::ofstream & file);
  
  // Helper for generating space directions from direction and spacing
  void GenerateSpaceDirections();
  
  // Convert ITK component type to JNRRD type string
  std::string GetJNRRDTypeString(IOComponentType componentType);

  std::vector<std::string> m_FileExtensions{ ".jnrrd" };
  std::map<std::string, json> m_Header;     // JNRRD header fields
  std::map<std::string, json> m_Extensions; // Extension objects
  std::streampos m_BinaryDataStart;         // Position of binary data in file
  std::string m_FileName;                   // Current file name
  std::string m_DataFileName;               // Detached data file name, if applicable
};

} // end namespace itk

#endif // itkJNRRDImageIO_h