/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkEncodeString.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

// Encode a string in a C++ file from a text file.
// For example, it can be used to encode a GLSL source file (in Rendering or
// VolumeRendering) or an event log (in Widgets/Testing).

#include "vtkSystemIncludes.h" // for cout,endl
#include <vtkstd/string> 
#include <vtksys/ios/sstream>
//#include <vtksys/SystemTools.hxx>

// Functions from kwsys SystemTools, as we cannot link vtkEncodeString
// against vtksys because of installation isssues.

/**
 * Return file name of a full filename (i.e. file name without path).
 */
vtkstd::string GetFilenameName(const vtkstd::string& filename)
{
#if defined(_WIN32)
  vtkstd::string::size_type slash_pos = filename.find_last_of("/\\");
#else
  vtkstd::string::size_type slash_pos = filename.find_last_of("/");
#endif
  if(slash_pos != vtkstd::string::npos)
    {
    return filename.substr(slash_pos + 1);
    }
  else
    {
    return filename;
    }
}

/**
 * Return file name without extension of a full filename (i.e. without path).
 * Warning: it considers the longest extension (for example: .tar.gz)
 */
vtkstd::string GetFilenameWithoutExtension(const vtkstd::string& filename)
{
  vtkstd::string name = GetFilenameName(filename);
  vtkstd::string::size_type dot_pos = name.find(".");
  if(dot_pos != vtkstd::string::npos)
    {
    return name.substr(0, dot_pos);
    }
  else
    {
    return name;
    }
}


/**
 * Return file name without extension of a full filename (i.e. without path).
 * Warning: it considers the last extension (for example: removes .gz
 * from .tar.gz)
 */
vtkstd::string GetFilenameWithoutLastExtension(const vtkstd::string& filename)
{
  vtkstd::string name = GetFilenameName(filename);
  vtkstd::string::size_type dot_pos = name.rfind(".");
  if(dot_pos != vtkstd::string::npos)
    {
    return name.substr(0, dot_pos);
    }
  else
    {
    return name;
    }
}


class Output
{
public:
  Output()
    {
    }
  Output(const Output&);
  void operator=(const Output&);
  ~Output()
    {
    }
  vtksys_ios::ostringstream Stream;

  bool ProcessFile(const char *inputFile,
                   const char *stringName,
                   bool buildHeader,
                   const vtkstd::string &fileName)
    {
      FILE *fp=fopen(inputFile, "r");
      if(!fp)
        {
          cout << "Cannot open file: " << inputFile
               << " (check path and permissions)" << endl;
          return false;
        }
      int ch;
      this->Stream << " * Define the " << stringName << " string." << endl
                   << " *" << endl
                   << " * Generated from file: " << inputFile << endl
                   << " */" << endl;
      
      if(buildHeader)
        {
          this->Stream << "#include \""<<fileName<<".h\"" << endl;
        }
      this->Stream << "const char *" << stringName << " =" 
                   << endl << "\"";
      while ( ( ch = fgetc(fp) ) != EOF )
        {
          if ( ch == '\n' )
            {
              this->Stream << "\\n\"" << endl << "\"";
            }
          else if ( ch == '\\' )
            {
              this->Stream << "\\\\";
            }
          else if ( ch == '\"' )
            {
              this->Stream << "\\\"";
            }
          else if ( ch != '\r' )
            {
              this->Stream << static_cast<unsigned char>(ch);
            }
        }
      this->Stream << "\\n\";" << endl;
      fclose(fp);
      return true;
    }
};

int main(int argc,
         char *argv[])
{
  vtkstd::string option;
  
  if(argc==7)
    {
      option=argv[4];
    }
  
  if(argc<4 || argc>7 || (argc==7 && option.compare("--build-header")!=0))
    {
      cout << "Encode a string in a C or C++ file from a text file." << endl;
      cout << "Usage: " << argv[0] << " <output-file> <input-path> <stringname>"
           << "[--build-header <export-macro> <extra-header>]" << endl
           << "Example: " << argv[0] << " MyString.cxx MyString.txt MyGeneratedString --build-header MYSTRING_EXPORT MyHeaderDefiningExport.h" << endl;
      return 1;
    }
  Output ot;
  ot.Stream << "/* DO NOT EDIT." << endl
            << " * Generated by " << argv[0] << endl
            << " * " << endl;
  
  vtkstd::string output = argv[1];
  vtkstd::string input = argv[2];
  
  bool outputIsC=output.find(".c",output.size()-2)!=vtkstd::string::npos;
  bool buildHeader=argc==7;
  
  vtkstd::string fileName=GetFilenameWithoutLastExtension(output);
  
  if(!ot.ProcessFile(input.c_str(), argv[3],buildHeader,fileName))
    {
      cout<<"Problem generating c";
      if(!outputIsC)
        {
          cout<<"++";
        }
      cout<<"file from source text file: " <<
        input.c_str() << endl;
      return 1;
    }
  
  ot.Stream << endl;
  
  if(buildHeader)
    {
      Output hs;
      hs.Stream << "/* DO NOT EDIT." << endl
                << " * Generated by " << argv[0] << endl
                << " * " << endl
                << " * Declare the " << argv[3] << " string." << endl
                << " *" << endl
                << " */" << endl
                << "#ifndef __"<<fileName<<"_h"<<endl
                << "#define __"<<fileName<<"_h"<<endl
                << endl
                << "#include \"" << argv[6] << "\"" <<endl // extra header file
                << endl;
      
      if(outputIsC)
        {
          hs.Stream << "#ifdef __cplusplus" <<endl
                    << "extern \"C\" {" <<endl
                    << "#endif /* #ifdef __cplusplus */" <<endl
                    << endl;
        }
      hs.Stream << argv[5] <<" extern const char *" << argv[3] << ";"<< endl
                << endl;
      
      if(outputIsC)
        {
          hs.Stream << "#ifdef __cplusplus" <<endl
                    << "}" <<endl
                    << "#endif /* #ifdef __cplusplus */" <<endl
                    << endl;
        }
      
      hs.Stream << "#endif /* #ifndef __" <<fileName<< "_h */" << endl;
      
      vtkstd::string headerOutput=GetFilenameWithoutExtension(output)+".h";
      
      FILE *hfp=fopen(headerOutput.c_str(),"w");
      if(!hfp)
        {
          cout << "Cannot open output file: " << headerOutput.c_str() << endl;
          return 1;
        }
      fprintf(hfp, "%s", hs.Stream.str().c_str());
      fclose(hfp);
    }
  
  FILE *fp=fopen(output.c_str(),"w");
  if(!fp)
    {
      cout << "Cannot open output file: " << output.c_str() << endl;
      return 1;
    }
  fprintf(fp, "%s", ot.Stream.str().c_str());
  fclose(fp);
  return 0;
}
