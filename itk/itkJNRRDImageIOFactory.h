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
#ifndef itkJNRRDImageIOFactory_h
#define itkJNRRDImageIOFactory_h

#include "itkObjectFactoryBase.h"
#include "itkImageIOBase.h"

namespace itk
{
/**
 * \class JNRRDImageIOFactory
 * \brief Factory for JNRRD (JSON-based Nearly Raw Raster Data) image format.
 *
 * This factory registers the JNRRDImageIO class with the ITK object factory
 * system, allowing JNRRDImageIO to be automatically created when needed.
 *
 * \ingroup IOFilters
 */
class ITK_TEMPLATE_EXPORT JNRRDImageIOFactory : public ObjectFactoryBase
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(JNRRDImageIOFactory);

  /** Standard class type aliases. */
  using Self = JNRRDImageIOFactory;
  using Superclass = ObjectFactoryBase;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** Class methods used to interface with the registered factories. */
  const char * GetITKSourceVersion() const override;
  const char * GetDescription() const override;

  /** Method for class instantiation. */
  itkFactorylessNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(JNRRDImageIOFactory, ObjectFactoryBase);

  /** Register one factory of this type. */
  static void RegisterOneFactory()
  {
    auto factory = Self::New();
    ObjectFactoryBase::RegisterFactoryInternal(factory);
  }

protected:
  JNRRDImageIOFactory();
  ~JNRRDImageIOFactory() override;
};

} // end namespace itk

#endif // itkJNRRDImageIOFactory_h