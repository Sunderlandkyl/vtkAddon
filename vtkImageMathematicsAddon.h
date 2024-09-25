/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageMathematics.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkImageMathematicsAddon
 * @brief   implicit function consisting of boolean combinations of implicit functions, with invert option
 *
 * This class is a subclass of vtkImageMathematics that adds an option to invert the result of the boolean operation.
 * 
 */

#ifndef vtkImageMathematicsAddon_h
#define vtkImageMathematicsAddon_h

#define VTK_NORMALIZE_MULTIPLY 100

// VTK includes
#include "vtkImageMathematics.h"

// vtkAddon includes
#include "vtkAddon.h"

class VTK_ADDON_EXPORT vtkImageMathematicsAddon : public vtkImageMathematics
{
public:
  vtkTypeMacro(vtkImageMathematicsAddon, vtkImageMathematics);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  static vtkImageMathematicsAddon* New();

  void SetOperationToNormalizeMultiply() { this->SetOperation(VTK_NORMALIZE_MULTIPLY); }

  vtkSetVector2Macro(NormalizeRange, double);
  vtkGetVector2Macro(NormalizeRange, double);

protected:
  vtkImageMathematicsAddon();
  ~vtkImageMathematicsAddon() override;

  int RequestInformation(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  void ThreadedRequestData(vtkInformation* request, vtkInformationVector** inputVector,
    vtkInformationVector* outputVector, vtkImageData*** inData, vtkImageData** outData,
    int outExt[6], int threadId) override;

  double NormalizeRange[2] = { 0.0, 1.0 };

private:
  vtkImageMathematicsAddon(const vtkImageMathematicsAddon&) = delete;
  void operator=(const vtkImageMathematicsAddon&) = delete;
};
#endif
