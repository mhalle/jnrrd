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
#include "itkJNRRDImageIO.h"
#include "itkVersion.h"

namespace itk
{

JNRRDImageIOFactory::JNRRDImageIOFactory()
{
  this->RegisterOverride(
    "itkImageIOBase", "itkJNRRDImageIO", "JNRRD Image IO", true, CreateObjectFunction<JNRRDImageIO>::New());
}

JNRRDImageIOFactory::~JNRRDImageIOFactory() = default;

const char *
JNRRDImageIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char *
JNRRDImageIOFactory::GetDescription() const
{
  return "JNRRD ImageIO Factory, allows reading and writing JNRRD files.";
}

// Automatically register this factory when the library is loaded
namespace
{
struct JNRRDImageIOFactoryInitializer
{
  JNRRDImageIOFactoryInitializer()
  {
    JNRRDImageIOFactory::RegisterOneFactory();
  }
};

static JNRRDImageIOFactoryInitializer jnrrdIOFactoryInitializer;
} // namespace

} // end namespace itk