# JNRRD Metadata Extension

Version: 1.0.0

## 1. Introduction

The JNRRD Metadata Extension provides a standardized approach for incorporating rich, descriptive metadata into JNRRD files based on the [schema.org](https://schema.org) vocabulary. This extension enables JNRRD files to be self-describing, discoverable, and interoperable with existing metadata ecosystems.

Schema.org is a collaborative, community-driven effort to create structured data schemas for the web. By leveraging this widely-adopted vocabulary, the JNRRD Metadata Extension ensures compatibility with existing metadata standards while providing the flexibility needed for scientific data.

## 2. Extension Declaration

To use this extension, a JNRRD file must include the following extension declaration:

```json
{"extensions": {"meta": "https://jnrrd.org/extensions/metadata/v1.0.0"}}
```

## 3. Core Concepts

### 3.1 Schema.org Type System

The metadata extension uses schema.org's type system, which organizes entities into a hierarchical structure. The most relevant types for scientific data include:

- [Dataset](https://schema.org/Dataset) - A collection of data, published or curated by a single agent
- [CreativeWork](https://schema.org/CreativeWork) - The most generic kind of creative work
- [Person](https://schema.org/Person) - An individual human
- [Organization](https://schema.org/Organization) - An organization such as a university or company
- [ImageObject](https://schema.org/ImageObject) - An image file
- [MedicalEntity](https://schema.org/MedicalEntity) - Root type for medical entities

### 3.2 Metadata Categories

The extension organizes metadata into logical categories:

1. **Core Information** - Basic information about the dataset
2. **Attribution** - Creator, contributor, and rights information
3. **Provenance** - Information about the dataset's origin and history
4. **Project Metadata** - Research project context
5. **Subject Metadata** - Information about the subject of the data (for medical/biological data)
6. **Scientific Context** - Keywords, fields of study, etc.
7. **Technical Metadata** - Technical details about the dataset format

### 3.3 Property Mapping

Schema.org properties are mapped directly to JNRRD extension fields using the `meta:` prefix. For example, the schema.org property `name` becomes `meta:name` in a JNRRD file.

Hierarchical properties use dot notation or nested objects, following the JNRRD specification:

```json
{"meta:author.name": "Jane Doe"}
```

or 

```json
{"meta:author": {"name": "Jane Doe", "affiliation": {"name": "University of Science"}}}
```

## 4. Core Metadata Fields

### 4.1 Dataset Identity

Basic identification information for the dataset:

| Field | Type | Description | Schema.org Property |
|-------|------|-------------|---------------------|
| `meta:@type` | string | Type of the metadata entity (usually "Dataset") | @type |
| `meta:name` | string | Human-readable name for the dataset | name |
| `meta:alternateName` | string | Alternative name | alternateName |
| `meta:description` | string | Textual description of the dataset | description |
| `meta:url` | string | URL of the dataset landing page | url |
| `meta:identifier` | string | Formal identifier (DOI, etc.) | identifier |
| `meta:version` | string | Version of the dataset | version |

Example:

```json
{"meta:@type": "Dataset"}
{"meta:name": "3D Brain MRI Dataset - Healthy Controls"}
{"meta:description": "T1-weighted brain MRI scans of 50 healthy control subjects collected as part of the Brain Function Study."}
{"meta:identifier": "doi:10.12345/braindata.2023.001"}
{"meta:version": "1.0.0"}
```

### 4.2 Temporal and Status Information

Information about the dataset's temporal context and status:

| Field | Type | Description | Schema.org Property |
|-------|------|-------------|---------------------|
| `meta:dateCreated` | string | Creation date (ISO 8601) | dateCreated |
| `meta:dateModified` | string | Last modification date (ISO 8601) | dateModified |
| `meta:datePublished` | string | Publication date (ISO 8601) | datePublished |
| `meta:temporalCoverage` | string | Time period covered by the dataset | temporalCoverage |
| `meta:isAccessibleForFree` | boolean | Whether the dataset is freely accessible | isAccessibleForFree |
| `meta:creativeWorkStatus` | string | Status of the dataset (e.g., "Draft", "Published") | creativeWorkStatus |

Example:

```json
{"meta:dateCreated": "2023-05-15T08:30:00Z"}
{"meta:dateModified": "2023-06-20T14:45:00Z"}
{"meta:datePublished": "2023-07-01T00:00:00Z"}
{"meta:temporalCoverage": "2022-01/2022-12"}
{"meta:creativeWorkStatus": "Published"}
{"meta:isAccessibleForFree": true}
```

### 4.3 Rights Information

Information about licensing and usage rights:

| Field | Type | Description | Schema.org Property |
|-------|------|-------------|---------------------|
| `meta:license` | string | License URL or identifier | license |
| `meta:usageInfo` | string | Information about usage restrictions | usageInfo |
| `meta:copyrightHolder` | object | Rights holder organization or person | copyrightHolder |
| `meta:copyrightYear` | number | Year of copyright | copyrightYear |
| `meta:conditionsOfAccess` | string | Conditions for accessing the dataset | conditionsOfAccess |

Example:

```json
{"meta:license": "https://creativecommons.org/licenses/by/4.0/"}
{"meta:usageInfo": "Data is available for research purposes with proper attribution."}
{"meta:copyrightHolder": {"@type": "Organization", "name": "University Medical Center"}}
{"meta:copyrightYear": 2023}
{"meta:conditionsOfAccess": "Requires approved research protocol and data use agreement."}
```

## 5. Creators and Attribution

### 5.1 Creators and Contributors

Information about individuals and organizations responsible for the dataset:

| Field | Type | Description | Schema.org Property |
|-------|------|-------------|---------------------|
| `meta:author` | object/array | Primary author(s) of the dataset | author |
| `meta:creator` | object/array | Creator of the dataset (can be Person or Organization) | creator |
| `meta:contributor` | object/array | Contributors to the dataset | contributor |
| `meta:publisher` | object | Organization that published the dataset | publisher |
| `meta:editor` | object/array | Editors of the dataset | editor |
| `meta:funder` | object/array | Funding organizations or grants | funder |
| `meta:accountablePerson` | object | Person accountable for the dataset | accountablePerson |

For Person entities, the following properties are commonly used:

| Field | Type | Description | Schema.org Property |
|-------|------|-------------|---------------------|
| `name` | string | Full name of person | name |
| `givenName` | string | First name | givenName |
| `familyName` | string | Last name | familyName |
| `email` | string | Email address | email |
| `affiliation` | object | Organization affiliation | affiliation |
| `jobTitle` | string | Job title | jobTitle |
| `identifier` | string | Person identifier (ORCID, etc.) | identifier |
| `url` | string | URL of personal website | url |

For Organization entities:

| Field | Type | Description | Schema.org Property |
|-------|------|-------------|---------------------|
| `name` | string | Organization name | name |
| `url` | string | Organization website | url |
| `address` | object | Physical address | address |
| `department` | string | Department name | department |
| `identifier` | string | Organization identifier (ROR, etc.) | identifier |

Example:

```json
{"meta:creator": [
  {
    "@type": "Person",
    "name": "Jane Doe",
    "givenName": "Jane",
    "familyName": "Doe",
    "email": "jane.doe@example.edu",
    "identifier": "https://orcid.org/0000-0001-2345-6789",
    "affiliation": {
      "@type": "Organization",
      "name": "University of Science",
      "department": "Department of Neuroscience"
    }
  },
  {
    "@type": "Person",
    "name": "John Smith",
    "identifier": "https://orcid.org/0000-0002-3456-7890",
    "affiliation": {
      "@type": "Organization",
      "name": "Medical Research Institute"
    }
  }
]}

{"meta:funder": {
  "@type": "Organization",
  "name": "National Science Foundation",
  "identifier": "https://ror.org/021nxhr62"
}}

{"meta:publisher": {
  "@type": "Organization",
  "name": "Scientific Data Repository",
  "url": "https://repository.example.org"
}}
```

Using dot notation:

```json
{"meta:creator[0].@type": "Person"}
{"meta:creator[0].name": "Jane Doe"}
{"meta:creator[0].identifier": "https://orcid.org/0000-0001-2345-6789"}
{"meta:creator[0].affiliation.name": "University of Science"}
{"meta:creator[1].@type": "Person"}
{"meta:creator[1].name": "John Smith"}
```

## 6. Scientific Context

### 6.1 Research Context

Information about the scientific context of the dataset:

| Field | Type | Description | Schema.org Property |
|-------|------|-------------|---------------------|
| `meta:keywords` | array | Keywords describing the dataset | keywords |
| `meta:about` | object/array | The subject matter of the dataset | about |
| `meta:citation` | string/object | Citation for the dataset | citation |
| `meta:isBasedOn` | object/array | Source datasets or methods | isBasedOn |
| `meta:isPartOf` | object | Larger dataset or collection | isPartOf |
| `meta:hasPart` | object/array | Component datasets | hasPart |
| `meta:abstract` | string | Abstract describing the dataset | abstract |
| `meta:spatialCoverage` | object/string | Geographic area covered | spatialCoverage |

Example:

```json
{"meta:keywords": ["neuroimaging", "MRI", "T1-weighted", "brain", "healthy controls"]}

{"meta:about": [
  {"@type": "Thing", "name": "Brain Structure"},
  {"@type": "Thing", "name": "Neuroimaging"},
  {"@type": "MedicalCondition", "name": "Healthy Controls"}
]}

{"meta:citation": "Doe, J., Smith, J. (2023). 3D Brain MRI Dataset. Scientific Data Repository. doi:10.12345/braindata.2023.001"}

{"meta:isBasedOn": {
  "@type": "CreativeWork",
  "name": "Brain Function Study Protocol",
  "url": "https://protocols.io/view/brain-function-study-az6hd3n"
}}

{"meta:abstract": "This dataset contains high-resolution T1-weighted MRI scans of 50 healthy control subjects. The scans were acquired using a standardized protocol with a 3T Siemens Prisma scanner. The dataset includes demographic information and quality metrics."}
```

### 6.2 Technical Details

Technical information about the dataset:

| Field | Type | Description | Schema.org Property |
|-------|------|-------------|---------------------|
| `meta:contentUrl` | string | Direct URL to the dataset file | contentUrl |
| `meta:contentSize` | string | Size of the dataset | contentSize |
| `meta:encodingFormat` | string | Format of the dataset | encodingFormat |
| `meta:distribution` | object/array | Distribution details | distribution |
| `meta:measurementTechnique` | string/array | Techniques used to collect data | measurementTechnique |
| `meta:variableMeasured` | string/array | Variables measured in the dataset | variableMeasured |

Example:

```json
{"meta:contentUrl": "https://repository.example.org/datasets/brain_mri_001.jnrrd"}
{"meta:contentSize": "2.5 GB"}
{"meta:encodingFormat": "application/jnrrd"}

{"meta:measurementTechnique": [
  "3T MRI",
  "T1-weighted MPRAGE sequence"
]}

{"meta:variableMeasured": [
  "Brain volume",
  "Gray matter morphometry",
  "White matter integrity"
]}

{"meta:distribution": {
  "@type": "DataDownload",
  "contentUrl": "https://repository.example.org/downloads/brain_mri_001.jnrrd",
  "encodingFormat": "application/jnrrd",
  "contentSize": "2.5 GB"
}}
```

## 7. Medical and Biological Context

### 7.1 Medical Subject Information

For medical datasets, information about the subjects:

| Field | Type | Description | Schema.org Property |
|-------|------|-------------|---------------------|
| `meta:healthCondition` | object | The health condition studied | healthCondition |
| `meta:subjectOf` | object | The medical study | subjectOf |
| `meta:studySubject` | object/array | The study subjects | studySubject |
| `meta:biologicalSpecimen` | object/array | Biological specimens | - |
| `meta:population` | object | The population studied | - |

Example:

```json
{"meta:healthCondition": {
  "@type": "MedicalCondition",
  "name": "Healthy",
  "code": {
    "@type": "MedicalCode",
    "codeValue": "Z00.00",
    "codingSystem": "ICD-10"
  }
}}

{"meta:studySubject": {
  "@type": "Patient",
  "subjectType": "Human",
  "demographicData": {
    "ageRange": "18-65",
    "gender": "Mixed",
    "ethnicity": "Mixed"
  },
  "numberOfParticipants": 50
}}
```

## 8. Project Information

### 8.1 Research Project Context

Information about the research project:

| Field | Type | Description | Schema.org Property |
|-------|------|-------------|---------------------|
| `meta:project` | object | Research project information | - |
| `meta:experiment` | object | Experiment information | - |
| `meta:grant` | object/array | Grant funding the research | - |
| `meta:protocol` | object | Research protocol | - |

Example:

```json
{"meta:project": {
  "name": "Brain Function Study",
  "identifier": "BFS-2023",
  "description": "A study of brain function in healthy adults using multimodal neuroimaging.",
  "principalInvestigator": {
    "@type": "Person",
    "name": "Dr. Jane Doe",
    "identifier": "https://orcid.org/0000-0001-2345-6789"
  }
}}

{"meta:grant": {
  "name": "Comprehensive Brain Mapping Initiative",
  "identifier": "NSF-CB-2023-01245",
  "fundingAgency": {
    "@type": "Organization",
    "name": "National Science Foundation"
  },
  "fundingAmount": "1,200,000 USD"
}}

{"meta:protocol": {
  "name": "Brain Imaging Standard Protocol v2.1",
  "url": "https://protocols.io/view/brain-imaging-standard-protocol-v2-1-bz36p7re",
  "version": "2.1"
}}
```

## 9. Provenance Information

### 9.1 Processing History

Information about data processing and transformations:

| Field | Type | Description | Schema.org Property |
|-------|------|-------------|---------------------|
| `meta:processingSteps` | array | Processing steps applied | - |
| `meta:derivedFrom` | object/array | Source data | - |
| `meta:softwareUsed` | object/array | Software used for processing | - |
| `meta:generatedBy` | object | Entity that generated the data | - |

Example:

```json
{"meta:processingSteps": [
  {
    "name": "Motion Correction",
    "description": "6-parameter rigid body motion correction",
    "software": {
      "name": "FSL MCFLIRT",
      "version": "6.0.5"
    },
    "datePerformed": "2023-05-20T14:30:00Z",
    "performedBy": {
      "@type": "Person",
      "name": "John Smith"
    }
  },
  {
    "name": "Brain Extraction",
    "description": "Skull stripping using BET",
    "software": {
      "name": "FSL BET",
      "version": "6.0.5"
    }
  }
]}

{"meta:softwareUsed": [
  {
    "@type": "SoftwareApplication",
    "name": "FSL",
    "version": "6.0.5",
    "url": "https://fsl.fmrib.ox.ac.uk/"
  },
  {
    "@type": "SoftwareApplication",
    "name": "Custom Processing Pipeline",
    "version": "1.2.0",
    "codeRepository": "https://github.com/example/pipeline"
  }
]}

{"meta:derivedFrom": {
  "@type": "Dataset",
  "name": "Raw DICOM Brain Scans",
  "identifier": "doi:10.12345/raw-brain-dicom"
}}
```

## 10. Full Example

A complete example of a JNRRD file with comprehensive metadata:

```
{"jnrrd": "0004"}
{"type": "float32"}
{"dimension": 3}
{"sizes": [256, 256, 180]}
{"endian": "little"}
{"encoding": "raw"}
{"space": "right_anterior_superior"}
{"space_directions": [[1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0]]}
{"space_origin": [0.0, 0.0, 0.0]}
{"extensions": {"meta": "https://jnrrd.org/extensions/metadata/v1.0.0"}}

{"meta:@type": "Dataset"}
{"meta:name": "High-Resolution T1-weighted Brain MRI Dataset"}
{"meta:description": "3D T1-weighted MRI brain scan of healthy adult subject, part of the Brain Function Study."}
{"meta:identifier": "doi:10.12345/braindata.2023.001"}
{"meta:version": "1.0.0"}
{"meta:dateCreated": "2023-05-15T08:30:00Z"}
{"meta:datePublished": "2023-07-01T00:00:00Z"}
{"meta:license": "https://creativecommons.org/licenses/by/4.0/"}
{"meta:keywords": ["neuroimaging", "MRI", "T1-weighted", "brain", "healthy controls"]}

{"meta:creator": [
  {
    "@type": "Person",
    "name": "Jane Doe",
    "givenName": "Jane",
    "familyName": "Doe",
    "email": "jane.doe@example.edu",
    "identifier": "https://orcid.org/0000-0001-2345-6789",
    "affiliation": {
      "@type": "Organization",
      "name": "University of Science",
      "department": "Department of Neuroscience"
    }
  },
  {
    "@type": "Person",
    "name": "John Smith",
    "identifier": "https://orcid.org/0000-0002-3456-7890",
    "affiliation": {
      "@type": "Organization",
      "name": "Medical Research Institute"
    }
  }
]}

{"meta:publisher": {
  "@type": "Organization",
  "name": "Scientific Data Repository",
  "url": "https://repository.example.org"
}}

{"meta:funder": {
  "@type": "Organization",
  "name": "National Science Foundation",
  "identifier": "https://ror.org/021nxhr62"
}}

{"meta:about": [
  {"@type": "Thing", "name": "Brain Structure"},
  {"@type": "Thing", "name": "Neuroimaging"},
  {"@type": "MedicalCondition", "name": "Healthy Controls"}
]}

{"meta:citation": "Doe, J., Smith, J. (2023). 3D Brain MRI Dataset. Scientific Data Repository. doi:10.12345/braindata.2023.001"}

{"meta:contentUrl": "https://repository.example.org/datasets/brain_mri_001.jnrrd"}
{"meta:contentSize": "2.5 GB"}
{"meta:encodingFormat": "application/jnrrd"}

{"meta:measurementTechnique": [
  "3T MRI",
  "T1-weighted MPRAGE sequence"
]}

{"meta:healthCondition": {
  "@type": "MedicalCondition",
  "name": "Healthy",
  "code": {
    "@type": "MedicalCode",
    "codeValue": "Z00.00",
    "codingSystem": "ICD-10"
  }
}}

{"meta:studySubject": {
  "@type": "Patient",
  "subjectType": "Human",
  "demographicData": {
    "ageRange": "25-30",
    "gender": "Female",
    "ethnicity": "Caucasian"
  }
}}

{"meta:project": {
  "name": "Brain Function Study",
  "identifier": "BFS-2023",
  "description": "A study of brain function in healthy adults using multimodal neuroimaging.",
  "principalInvestigator": {
    "@type": "Person",
    "name": "Dr. Jane Doe",
    "identifier": "https://orcid.org/0000-0001-2345-6789"
  }
}}

{"meta:processingSteps": [
  {
    "name": "Motion Correction",
    "description": "6-parameter rigid body motion correction",
    "software": {
      "name": "FSL MCFLIRT",
      "version": "6.0.5"
    }
  },
  {
    "name": "Brain Extraction",
    "description": "Skull stripping using BET",
    "software": {
      "name": "FSL BET",
      "version": "6.0.5"
    }
  }
]}

{"meta:softwareUsed": [
  {
    "@type": "SoftwareApplication",
    "name": "FSL",
    "version": "6.0.5",
    "url": "https://fsl.fmrib.ox.ac.uk/"
  }
]}

[BINARY DATA FOLLOWS]
```

## 11. JSON Schema for Validation

The following JSON Schema can be used to validate metadata extension fields:

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "JNRRD Metadata Extension Schema",
  "description": "Schema for validating JNRRD metadata extension fields based on schema.org",
  "type": "object",
  "properties": {
    "@type": {
      "type": "string",
      "description": "The type of the metadata entity (stored as 'meta:@type' in JNRRD files)"
    },
    "name": {
      "type": "string",
      "description": "Name of the dataset (stored as 'meta:name' in JNRRD files)"
    },
    "description": {
      "type": "string",
      "description": "Description of the dataset (stored as 'meta:description' in JNRRD files)"
    },
    "identifier": {
      "type": "string",
      "description": "Identifier for the dataset, such as a DOI (stored as 'meta:identifier' in JNRRD files)"
    },
    "url": {
      "type": "string",
      "format": "uri",
      "description": "URL for the dataset (stored as 'meta:url' in JNRRD files)"
    },
    "version": {
      "type": "string",
      "description": "Version of the dataset (stored as 'meta:version' in JNRRD files)"
    },
    "keywords": {
      "type": "array",
      "items": {
        "type": "string"
      },
      "description": "Keywords describing the dataset (stored as 'meta:keywords' in JNRRD files)"
    },
    "license": {
      "type": "string",
      "description": "License for the dataset (stored as 'meta:license' in JNRRD files)"
    },
    "dateCreated": {
      "type": "string",
      "format": "date-time",
      "description": "Date the dataset was created (stored as 'meta:dateCreated' in JNRRD files)"
    },
    "datePublished": {
      "type": "string",
      "format": "date-time",
      "description": "Date the dataset was published (stored as 'meta:datePublished' in JNRRD files)"
    },
    "dateModified": {
      "type": "string",
      "format": "date-time",
      "description": "Date the dataset was last modified (stored as 'meta:dateModified' in JNRRD files)"
    },
    "creator": {
      "oneOf": [
        {
          "type": "object",
          "properties": {
            "@type": { "type": "string" },
            "name": { "type": "string" }
          },
          "required": ["name"]
        },
        {
          "type": "array",
          "items": {
            "type": "object",
            "properties": {
              "@type": { "type": "string" },
              "name": { "type": "string" }
            },
            "required": ["name"]
          }
        }
      ],
      "description": "Creator(s) of the dataset (stored as 'meta:creator' in JNRRD files)"
    },
    "publisher": {
      "type": "object",
      "properties": {
        "@type": { "type": "string" },
        "name": { "type": "string" }
      },
      "required": ["name"],
      "description": "Publisher of the dataset (stored as 'meta:publisher' in JNRRD files)"
    },
    "contentUrl": {
      "type": "string",
      "format": "uri",
      "description": "Direct URL to the dataset content (stored as 'meta:contentUrl' in JNRRD files)"
    }
  },
  "required": ["@type", "name"]
}
```

## 12. Implementation Notes

### 12.1 Parser Integration

The metadata extension is fully compatible with the JNRRD header parser described in [jnrrd_header_parser.md](jnrrd_header_parser.md), which handles hierarchical fields and array elements. After parsing, metadata fields are available under the `meta` namespace:

```python
parsed_header = parse_jnrrd_header(header_objects)
metadata = parsed_header["meta"]

# Access metadata fields
dataset_name = metadata.get("name")
creators = metadata.get("creator", [])
```

### 12.2 Schema.org Integration

Tools can convert JNRRD metadata to schema.org JSON-LD format for web publication:

```python
def convert_to_jsonld(metadata):
    """Convert JNRRD metadata to JSON-LD format."""
    jsonld = {
        "@context": "https://schema.org/",
    }
    
    # Copy all metadata fields
    for key, value in metadata.items():
        jsonld[key] = value
    
    return jsonld
```

### 12.3 Minimizing Metadata Size

To reduce header size for large datasets with extensive metadata:

1. Use the flattened hierarchy notation for selective updates
2. Consider using a separate metadata file referenced by the JNRRD file
3. Include only the most essential metadata in the JNRRD file itself

## 13. Compatibility Notes

This extension is designed to be compatible with:

1. **schema.org** - Uses schema.org vocabulary for interoperability
2. **DataCite** - Covers essential fields for citation
3. **Dublin Core** - Supports core metadata elements
4. **DCAT** - Compatible with data catalog vocabulary
5. **Bioschemas** - Supports extensions for life sciences

For specialized domains, consider combining this extension with domain-specific extensions such as the OME extension for microscopy or the NIfTI extension for neuroimaging.