# field-hardware-lib

## Purpose
The repository serves as a re-usable interface layer between programs and hardware.  The idea being we can easily write test programs to debug front-ends and utility programs outside of MIDAS.

## Contributing
Create a subdirectory for the system of interest, i.e., trolley-interface or vme-hardware (prefer lowercase and hyphen instead of spaces).  All or your source code goes there.  Each C/C++ project should include a `make install` directive in the Makefile which places the shared object library in `/usr/local/lib/` and the necessary headers in `/usr/local/include`.  These are standard locations, so that programs built using these classes and functions find them on the default search paths.  Please document the library using a Markdown file inside the subdirectory, for example, `vme-hardware/README.md`, which should be linked on the repo's main README.md.

## Libraries
- [vme-hardware](https://github.com/g2-field-team/field-hardware-lib/edit/master/vme-hardware/README.md)
