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

#include "itkJNRRDImageIOFactory.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include <iostream>
#include <string>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

// Simple test program for reading JNRRD files
int main(int argc, char * argv[])
{
  if (argc < 2)
  {
    std::cerr << "Usage: " << argv[0] << " <input_file> [output_file]" << std::endl;
    return EXIT_FAILURE;
  }

  try
  {
    // Register the JNRRD factory
    itk::JNRRDImageIOFactory::RegisterOneFactory();

    // Read the input file
    std::cout << "Reading JNRRD file: " << argv[1] << std::endl;
    
    // Create a reader
    using ReaderType = itk::ImageFileReader<itk::Image<float, 3>>;
    auto reader = ReaderType::New();
    reader->SetFileName(argv[1]);
    
    // Update the reader to read the file
    try
    {
      reader->Update();
    }
    catch (const itk::ExceptionObject & error)
    {
      std::cerr << "Error reading the file: " << error << std::endl;
      return EXIT_FAILURE;
    }
    
    // Get the image from the reader
    auto image = reader->GetOutput();
    
    // Print image information
    std::cout << "Image Information:" << std::endl;
    std::cout << "  Size: " << image->GetBufferedRegion().GetSize() << std::endl;
    std::cout << "  Spacing: " << image->GetSpacing() << std::endl;
    std::cout << "  Origin: " << image->GetOrigin() << std::endl;
    std::cout << "  Direction: " << image->GetDirection() << std::endl;
    
    // Get the metadata dictionary
    const auto & dict = reader->GetMetaDataDictionary();
    
    // Print some basic metadata
    std::string value;
    if (itk::ExposeMetaData(dict, "type", value))
    {
      std::cout << "  Type: " << value << std::endl;
    }
    
    if (itk::ExposeMetaData(dict, "dimension", value))
    {
      std::cout << "  Dimension: " << value << std::endl;
    }
    
    if (itk::ExposeMetaData(dict, "encoding", value))
    {
      std::cout << "  Encoding: " << value << std::endl;
    }
    
    if (itk::ExposeMetaData(dict, "space", value))
    {
      std::cout << "  Space: " << value << std::endl;
    }
    
    // Check for extensions
    for (const std::string & key : dict.GetKeys())
    {
      if (key.substr(0, 10) == "jnrrd_ext_")
      {
        std::string extName = key.substr(10);
        std::string extValue;
        itk::ExposeMetaData(dict, key, extValue);
        std::cout << "  Extension: " << extName << " - " << extValue << std::endl;
      }
    }
    
    // Write the image if an output file is provided
    if (argc > 2)
    {
      std::cout << "Writing to: " << argv[2] << std::endl;
      
      using WriterType = itk::ImageFileWriter<itk::Image<float, 3>>;
      auto writer = WriterType::New();
      writer->SetFileName(argv[2]);
      writer->SetInput(image);
      
      // Add some additional metadata for testing
      auto & writeDict = writer->GetMetaDataDictionary();
      
      // Add content description
      itk::EncapsulateMetaData<std::string>(writeDict, "content", "Test JNRRD file from ITK");
      
      // Add an extension
      json metadataExt = {
        {"name", "Test Image"},
        {"description", "A test image created with ITK JNRRD writer"},
        {"creator", {
          {"name", "ITK", "url", "https://itk.org"}
        }},
        {"dateCreated", "2025-02-28"}
      };
      
      itk::EncapsulateMetaData<std::string>(
        writeDict, "jnrrd_ext_metadata", metadataExt.dump());
      
      // Set compression if specified
      if (argc > 3 && std::string(argv[3]) == "compress")
      {
        std::cout << "Using gzip compression" << std::endl;
        itk::EncapsulateMetaData<std::string>(writeDict, "encoding", "gzip");
      }
      
      try
      {
        writer->Update();
        std::cout << "Successfully wrote JNRRD file: " << argv[2] << std::endl;
      }
      catch (const itk::ExceptionObject & error)
      {
        std::cerr << "Error writing the file: " << error << std::endl;
        return EXIT_FAILURE;
      }
    }
    
    std::cout << "Test completed successfully!" << std::endl;
  }
  catch (const itk::ExceptionObject & error)
  {
    std::cerr << "Error: " << error << std::endl;
    return EXIT_FAILURE;
  }
  catch (const std::exception & error)
  {
    std::cerr << "Error: " << error.what() << std::endl;
    return EXIT_FAILURE;
  }
  catch (...)
  {
    std::cerr << "Unknown error!" << std::endl;
    return EXIT_FAILURE;
  }
  
  return EXIT_SUCCESS;
}