/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImplicitInvertableBoolean.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkImplicitInvertableBoolean.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkImplicitInvertableBoolean);

//----------------------------------------------------------------------------
vtkImplicitInvertableBoolean::vtkImplicitInvertableBoolean()
{
  this->Invert = false;
}

//----------------------------------------------------------------------------
vtkImplicitInvertableBoolean::~vtkImplicitInvertableBoolean()
{
}

//----------------------------------------------------------------------------
double vtkImplicitInvertableBoolean::EvaluateFunction(double x[3])
{
  double value = Superclass::EvaluateFunction(x);
  if (this->Invert)
  {
    value = -value;
  }
  return value;
}

//----------------------------------------------------------------------------
void vtkImplicitInvertableBoolean::EvaluateGradient(double x[3], double g[3])
{
  Superclass::EvaluateGradient(x, g);
  if (this->Invert)
  {
    g[0] = -g[0];
    g[1] = -g[1];
    g[2] = -g[2];
  }
}

//----------------------------------------------------------------------------
void vtkImplicitInvertableBoolean::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Invert:\n";
  os << indent << (this->Invert ? "True" : "False") << "\n";
}
