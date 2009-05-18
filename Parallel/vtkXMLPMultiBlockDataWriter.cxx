/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkXMLPMultiBlockDataWriter.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkXMLPMultiBlockDataWriter.h"

#include "vtkMultiProcessController.h"
#include "vtkObjectFactory.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkXMLPMultiBlockDataWriter);
vtkCxxRevisionMacro(vtkXMLPMultiBlockDataWriter, "$Revision: 1.1 $");

vtkCxxSetObjectMacro(vtkXMLPMultiBlockDataWriter, 
                     Controller,
                     vtkMultiProcessController);


//----------------------------------------------------------------------------
vtkXMLPMultiBlockDataWriter::vtkXMLPMultiBlockDataWriter()
{
  this->Controller = 0;
  this->SetController(vtkMultiProcessController::GetGlobalController());
}

//----------------------------------------------------------------------------
vtkXMLPMultiBlockDataWriter::~vtkXMLPMultiBlockDataWriter()
{
  this->SetController(0);
}

//----------------------------------------------------------------------------
void vtkXMLPMultiBlockDataWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Controller: ";
  if (this->Controller)
    {
    this->Controller->PrintSelf(os, indent.GetNextIndent());
    }
  else
    {
    os << "(none)" << endl;
    }
}

//----------------------------------------------------------------------------
void vtkXMLPMultiBlockDataWriter::FillDataTypes(vtkCompositeDataSet* hdInput)
{
  this->Superclass::FillDataTypes(hdInput);

  if (!this->Controller)
    {
    return;
    }

  int myid = this->Controller->GetLocalProcessId();
  int numProcs = this->Controller->GetNumberOfProcesses();

  unsigned numBlocks = this->GetNumberOfDataTypes();
  int* myDataTypes = this->GetDataTypesPointer();

  if (myid == 0)
    {
    int* dataTypes = new int[numBlocks];
    for (int i=1; i<numProcs; i++)
      {
      this->Controller->Receive(
        dataTypes, 
        numBlocks, 
        i, 
        vtkMultiProcessController::XML_WRITER_DATA_INFO);
      for (unsigned int j=0; j<numBlocks; j++)
        {
        if (dataTypes[j] >= 0)
          {
          myDataTypes[j] = dataTypes[j];
          }
        }
      }
    delete[] dataTypes;
    }
  else
    {
    this->Controller->Send(myDataTypes, 
                           numBlocks, 
                           0, 
                           vtkMultiProcessController::XML_WRITER_DATA_INFO);
    }

}

