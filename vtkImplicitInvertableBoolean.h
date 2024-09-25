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
 * @class   vtkImplicitInvertableBoolean
 * @brief   implicit function consisting of boolean combinations of implicit functions, with invert option
 *
 * This class is a subclass of vtkImplicitBoolean that adds an option to invert the result of the boolean operation.
 * 
 */

#ifndef vtkImplicitInvertableBoolean_h
#define vtkImplicitInvertableBoolean_h

// VTK includes
#include "vtkImplicitBoolean.h"

// vtkAddon includes
#include "vtkAddon.h"

class VTK_ADDON_EXPORT vtkImplicitInvertableBoolean : public vtkImplicitBoolean
{
public:
  vtkTypeMacro(vtkImplicitInvertableBoolean, vtkImplicitBoolean);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  static vtkImplicitInvertableBoolean* New();

  ///@{
  /**
   * Evaluate boolean combinations of implicit function using current operator.
   */
  double EvaluateFunction(double x[3]) override;
  ///@}

  /**
   * Evaluate gradient of boolean combination.
   */
  void EvaluateGradient(double x[3], double g[3]) override;

  ///@{
  /**
   * Specify if the resulting function should be inverted
   */
  vtkSetMacro(Invert, bool);
  vtkGetMacro(Invert, bool);
  vtkBooleanMacro(Invert, bool);
  ///@}

protected:
  vtkImplicitInvertableBoolean();
  ~vtkImplicitInvertableBoolean() override;

  bool Invert;

private:
  vtkImplicitInvertableBoolean(const vtkImplicitInvertableBoolean&) = delete;
  void operator=(const vtkImplicitInvertableBoolean&) = delete;
};
#endif
