/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImplicitBoolean.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkCapPolyData
 * @brief   implicit function consisting of boolean combinations of implicit functions, with invert option
 *
 * This class is a subclass of vtkPolyDataAlgorithm that will generate an end cap for a polydata cut with the specified function
 * 
 */

#ifndef vtkCapPolyData_h
#define vtkCapPolyData_h

// VTK includes
#include <vtkAppendPolyData.h>
#include <vtkImplicitFunction.h>
#include <vtkPolyDataAlgorithm.h>

// vtkAddon includes
#include "vtkAddon.h"

class VTK_ADDON_EXPORT vtkCapPolyData : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkCapPolyData, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  static vtkCapPolyData* New();

  ///@{
  /**
   * Specify the implicit function with which to perform the
   * clipping. If you do not define an implicit function, then the input
   * scalar data will be used for clipping.
   */
  vtkSetObjectMacro(ClipFunction, vtkImplicitFunction);
  vtkGetObjectMacro(ClipFunction, vtkImplicitFunction);
  ///@}

  /**
  * Return the mtime also considering the locator and clip function.
  */
  vtkMTimeType GetMTime() override;

protected:
  vtkCapPolyData();
  ~vtkCapPolyData() override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;
  vtkImplicitFunction* ClipFunction{ nullptr };

  vtkNew<vtkAppendPolyData> Append;

private:
  vtkCapPolyData(const vtkCapPolyData&) = delete;
  void operator=(const vtkCapPolyData&) = delete;
};
#endif
