// VTK::System::Dec

// ============================================================================
//
//  Program:   Visualization Toolkit
//  Module:    vtkOutlineGlowBlurPassFS.glsl
//
//  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
//  All rights reserved.
//  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.
//
//     This software is distributed WITHOUT ANY WARRANTY; without even
//     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//     PURPOSE.  See the above copyright notice for more information.
//
// ============================================================================

// Fragment shader used by the outline glow blur render pass.

in vec2 tcoordVC;
uniform sampler2D source;

uniform float coef[3];
uniform float offsetx;
uniform float offsety;
uniform float w;
uniform float h;

uniform bool firstPass;

// the output of this shader
// VTK::Output::Dec

void main(void)
{
  vec2 offset = vec2(offsetx, offsety);
  float threshold = 0.0;

  vec4 color = texture2D(source, tcoordVC);
  if (firstPass == false)
  {
    gl_FragData[0] = color;
    return;
  }

  if (color.a > 0.1)
  {
    gl_FragData[0] = vec4(tcoordVC.x, tcoordVC.y, 1.0, 1.0);
  }
  else
  {
    gl_FragData[0] = vec4(tcoordVC.x, tcoordVC.y, 0.0, 1.0);
  }
}
