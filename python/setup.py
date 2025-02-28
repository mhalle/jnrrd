from setuptools import setup, find_packages

# Read the content of README.md for the long description
with open("../README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()

setup(
    name="jnrrd",
    version="0.2.0",
    author="JNRRD Contributors",
    description="JSON-based Nearly Raw Raster Data - A modern evolution of the NRRD file format",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/username/jnrrd",
    packages=find_packages(),
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Science/Research",
        "Intended Audience :: Healthcare Industry",
        "License :: OSI Approved :: MIT License",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Topic :: Scientific/Engineering :: Medical Science Apps.",
        "Topic :: Scientific/Engineering :: Image Processing",
    ],
    python_requires=">=3.8",
    install_requires=[
        "numpy>=1.20.0",
    ],
    extras_require={
        "all": [
            "bz2file>=0.98",
            "zstandard>=0.18.0",
            "lz4>=4.0.0",
        ],
        "dev": [
            "pytest>=7.0.0",
            "black>=23.0.0",
            "mypy>=1.0.0",
        ],
    },
)