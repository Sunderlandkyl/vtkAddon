/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkOpenGLError.h>
#include <vtkOpenGLFramebufferObject.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkOpenGLShaderCache.h>
#include <vtkOpenGLState.h>
#include <vtkOpenGLVertexArrayObject.h>
#include <vtkRenderState.h>
#include <vtkRenderer.h>
#include <vtkShaderProgram.h>
#include <vtkTextureObject.h>
#include <vtkOpenGLHelper.h>
//#include <vtkTextureObjectVS.h> // a pass through shader

// std includes
#include <cassert>

// Shader includes
#include "vtkJumpFloodAlgorithmPass.h"

#include "vtkJumpFloodAlgorithmFS.h"
#include "vtkJumpFloodInitFS.h"
#include "vtkJumpFloodOutlineFS.h"

vtkStandardNewMacro(vtkJumpFloodAlgorithmPass);

//#define VTK_JUMP_FLOOD_ALGORITHM_DEBUG

#ifdef VTK_JUMP_FLOOD_ALGORITHM_DEBUG
#include <vtkPixelBufferObject.h>
#include <vtkImageImport.h>
#include <vtkImageExtractComponents.h>
#include <vtkPNGWriter.h>
#endif

const char* VERTEX_SHADER =
"//VTK::System::Dec\n"
"\n"
"/*=========================================================================\n"
"\n"
"  Program:   Visualization Toolkit\n"
"  Module:    vtktextureObjectVS.glsl\n"
"\n"
"  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen\n"
"  All rights reserved.\n"
"  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.\n"
"\n"
"     This software is distributed WITHOUT ANY WARRANTY; without even\n"
"     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR\n"
"     PURPOSE.  See the above copyright notice for more information.\n"
"\n"
"=========================================================================*/\n"
"\n"
"in vec4 vertexMC;\n"
"in vec2 tcoordMC;\n"
"out vec2 tcoordVC;\n"
"\n"
"void main()\n"
"{\n"
"  tcoordVC = tcoordMC;\n"
"  gl_Position = vertexMC;\n"
"}\n"
"";
#define DATA_TYPE_VTK VTK_FLOAT
#define DATA_TYPE float
//#define DATA_TYPE_VTK VTK_SIGNED_CHAR
//#define DATA_TYPE signed char

#ifdef VTK_JUMP_FLOOD_ALGORITHM_DEBUG
void DebugWriteImageToFile(vtkTextureObject* pass, int w, int h, int c, const char* path)
{
  // Save first pass in file for debugging.
  vtkPixelBufferObject* pbo = pass->Download();

  DATA_TYPE* openglRawData = new DATA_TYPE[c * w * h];
  unsigned int dims[2];
  dims[0] = w;
  dims[1] = h;
  vtkIdType incs[2];
  incs[0] = 0;
  incs[1] = 0;
  bool status = pbo->Download2D(DATA_TYPE_VTK, openglRawData, dims, 4, incs);
  assert("check" && status);
  pbo->Delete();

  // no pbo
  pass->Bind();
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, openglRawData);
  pass->Deactivate();

  vtkImageImport* importer = vtkImageImport::New();
  importer->CopyImportVoidPointer(
    openglRawData, c * w * h * sizeof(DATA_TYPE));
  importer->SetDataScalarTypeToUnsignedChar();
  importer->SetNumberOfScalarComponents(4);
  importer->SetWholeExtent(0, w - 1, 0, h - 1, 0, 0);
  importer->SetDataExtentToWholeExtent();
  delete[] openglRawData;

  vtkImageExtractComponents* rgbaToRgb = vtkImageExtractComponents::New();
  rgbaToRgb->SetInputConnection(importer->GetOutputPort());
  rgbaToRgb->SetComponents(0, 1, 2);

  vtkPNGWriter* writer = vtkPNGWriter::New();
  writer->SetFileName(path);
  writer->SetInputConnection(rgbaToRgb->GetOutputPort());
  importer->Delete();
  rgbaToRgb->Delete();
  writer->Write();
  writer->Delete();
}
#endif

// ----------------------------------------------------------------------------
vtkJumpFloodAlgorithmPass::vtkJumpFloodAlgorithmPass()
{
  this->FrameBufferObject = nullptr;

  this->ScenePass = nullptr;
  this->InitPass = nullptr;
  this->JumpFloodPass1 = nullptr;
  this->JumpFloodPass2 = nullptr;

  this->InitProgram = nullptr;
  this->JumpPassProgram = nullptr;
  this->OutlineProgram = nullptr;
}

// ----------------------------------------------------------------------------
vtkJumpFloodAlgorithmPass::~vtkJumpFloodAlgorithmPass()
{
  if (this->FrameBufferObject != nullptr)
  {
    vtkErrorMacro(<< "FrameBufferObject should have been deleted in ReleaseGraphicsResources().");
  }
  if (this->ScenePass != nullptr)
  {
    vtkErrorMacro(<< "ScenePass should have been deleted in ReleaseGraphicsResources().");
  }
  if (this->JumpFloodPass1 != nullptr)
  {
    vtkErrorMacro(<< "JumpFloodPass should have been deleted in ReleaseGraphicsResources().");
  }
  if (this->JumpFloodPass2 != nullptr)
  {
    vtkErrorMacro(<< "JumpFloodPass2 should have been deleted in ReleaseGraphicsResources().");
  }
  if (this->OutlinePass != nullptr)
  {
    vtkErrorMacro(<< "OutlinePass should have been deleted in ReleaseGraphicsResources().");
  }
}

// ----------------------------------------------------------------------------
void vtkJumpFloodAlgorithmPass::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "OutlineGlowPass:";
  /*os << indent << "OutlineIntensity: " << this->OutlineIntensity << endl;*/
  //TODO
}

// ----------------------------------------------------------------------------
// Description:
// Perform rendering according to a render state \p s.
// \pre s_exists: s!=0
void vtkJumpFloodAlgorithmPass::Render(const vtkRenderState* s)
{
  assert("pre: s_exists" && s != nullptr);

  vtkOpenGLClearErrorMacro();

  this->NumberOfRenderedProps = 0;

  vtkRenderer* r = s->GetRenderer();
  vtkOpenGLRenderWindow* renWin = static_cast<vtkOpenGLRenderWindow*>(r->GetRenderWindow());
  vtkOpenGLState* ostate = renWin->GetState();

  if (this->DelegatePass == nullptr)
  {
    vtkWarningMacro(<< " no delegate.");
    return;
  }

  // 1. Create a new render state with an FBO.
  int width;
  int height;
  int size[2];
  s->GetWindowSize(size);
  width = size[0];
  height = size[1];
  int halfWidth = std::ceil((double)width / 1.0);
  int halfHeight = std::ceil((double)height / 1.0);
  //int halfWidth = width;
  //int halfHeight = height;

  // No extra pixels. Tex coord clamping takes care of the edges.
  int widthHalfTarget = halfWidth;
  int heightHalfTarget = halfHeight;
  int widthFullTarget = width;
  int heightFullTarget = height;

  if (this->FrameBufferObject == nullptr)
  {
    this->FrameBufferObject = vtkSmartPointer<vtkOpenGLFramebufferObject>::New();
    this->FrameBufferObject->SetContext(renWin);
  }

  if (this->ScenePass == nullptr)
  {
    this->ScenePass = vtkSmartPointer<vtkTextureObject>::New();
    this->ScenePass->SetContext(renWin);
  }

  // backup GL state
  vtkOpenGLState::ScopedglEnableDisable bsaver(ostate, GL_BLEND);
  vtkOpenGLState::ScopedglEnableDisable dsaver(ostate, GL_DEPTH_TEST);
  ostate->PushFramebufferBindings();

  // 2. Render Scene to an offscreen render target
  this->RenderDelegate(s, width, height, widthFullTarget, heightFullTarget,
    this->FrameBufferObject, this->ScenePass);

  ostate->vtkglDisable(GL_BLEND);
  ostate->vtkglDisable(GL_DEPTH_TEST);

  ////////////////////////
  // Initial render pass.

  if (this->InitPass == nullptr)
  {
    this->InitPass = vtkSmartPointer<vtkTextureObject>::New();
    this->InitPass->SetContext(this->FrameBufferObject->GetContext());
  }
  if (this->InitPass->GetWidth() != static_cast<unsigned int>(widthHalfTarget) ||
    this->InitPass->GetHeight() != static_cast<unsigned int>(heightHalfTarget))
  {
    this->InitPass->Create2D(static_cast<unsigned int>(widthHalfTarget),
      static_cast<unsigned int>(heightHalfTarget), 4, DATA_TYPE_VTK, false);
  }

  if (!this->InitializeProgram(renWin, this->InitProgram, VERTEX_SHADER, vtkJumpFloodInitFS, ""))
  {
    vtkErrorMacro("Couldn't build the shader program. At this point , it can be an error in a "
      "shader or a driver bug.");
    // restore some state.
    this->FrameBufferObject->UnBind();
    this->FrameBufferObject->RestorePreviousBindingsAndBuffers();
    return;
  }

  this->FrameBufferObject->AddColorAttachment(0, this->InitPass);
  this->FrameBufferObject->Start(widthHalfTarget, heightHalfTarget);

  // Setup original scene texture.
  this->ScenePass->Activate();
  this->ScenePass->SetMinificationFilter(vtkTextureObject::Linear);
  this->ScenePass->SetMagnificationFilter(vtkTextureObject::Linear);

  // Clamp the texture coordinates so we don't get an outline at the opposite end of the screen
  this->ScenePass->SetWrapS(vtkTextureObject::ClampToBorder);
  this->ScenePass->SetWrapT(vtkTextureObject::ClampToBorder);
  this->ScenePass->SetBorderColor(-1.0f, -1.0f, -1.0f, -1.0f);

  // Set the scene texture as the input for the shader.
  int sourceId = this->ScenePass->GetTextureUnit();
  this->InitProgram->Program->SetUniformi("source", sourceId);

  this->FrameBufferObject->RenderQuad(0, widthHalfTarget - 1, 0, heightHalfTarget - 1,
    this->InitProgram->Program, this->InitProgram->VAO);

#ifdef VTK_JUMP_FLOOD_ALGORITHM_DEBUG
  DebugWriteImageToFile(this->InitPass, widthHalfTarget, heightHalfTarget, 4, "Init.png");
#endif

  ////////////////////////
  // Jump flood step.
  if (!this->InitializeProgram(renWin, this->JumpPassProgram, VERTEX_SHADER, vtkJumpFloodAlgorithmFS, ""))
  {
    vtkErrorMacro("Couldn't build the shader program. At this point , it can be an error in a "
      "shader or a driver bug.");
    // restore some state.
    this->FrameBufferObject->UnBind();
    this->FrameBufferObject->RestorePreviousBindingsAndBuffers();
    return;
  }

  if (this->JumpFloodPass1 == nullptr)
  {
    this->JumpFloodPass1 = vtkSmartPointer<vtkTextureObject>::New();
    this->JumpFloodPass1->SetContext(this->FrameBufferObject->GetContext());
  }
  if (this->JumpFloodPass1->GetWidth() != static_cast<unsigned int>(widthHalfTarget) ||
    this->JumpFloodPass1->GetHeight() != static_cast<unsigned int>(heightHalfTarget))
  {
    this->JumpFloodPass1->Create2D(static_cast<unsigned int>(widthHalfTarget),
      static_cast<unsigned int>(heightHalfTarget), 4, DATA_TYPE_VTK, false);
  }

  if (this->JumpFloodPass2 == nullptr)
  {
    this->JumpFloodPass2 = vtkSmartPointer<vtkTextureObject>::New();
    this->JumpFloodPass2->SetContext(this->FrameBufferObject->GetContext());
  }
  if (this->JumpFloodPass2->GetWidth() != static_cast<unsigned int>(widthHalfTarget) ||
    this->JumpFloodPass2->GetHeight() != static_cast<unsigned int>(heightHalfTarget))
  {
    this->JumpFloodPass2->Create2D(static_cast<unsigned int>(widthHalfTarget),
      static_cast<unsigned int>(heightHalfTarget), 4, DATA_TYPE_VTK, false);
  }

  vtkTextureObject* source = this->InitPass;
  vtkTextureObject* target = this->JumpFloodPass1;

  int numberOfJumps = ceil(log2(this->OutlinePx + 1));
  for (int i = numberOfJumps; i >= 0; --i)
  {
    this->FrameBufferObject->AddColorAttachment(0, target);
    this->FrameBufferObject->Start(widthHalfTarget, heightHalfTarget);

    float stepSize[2] =
    {
      (pow(2, i) + 0.5) / widthHalfTarget,
      (pow(2, i) + 0.5) / heightHalfTarget,
    };
    this->JumpPassProgram->Program->SetUniform2f("stepSize", stepSize);

    source->Activate();
    sourceId = source->GetTextureUnit();
    source->SetMinificationFilter(vtkTextureObject::Nearest);
    source->SetMagnificationFilter(vtkTextureObject::Nearest);
    // Clamp the texture coordinates so we don't get an outline at the opposite end of the screen
    source->SetWrapS(vtkTextureObject::ClampToBorder);
    source->SetWrapT(vtkTextureObject::ClampToBorder);
    source->SetBorderColor(-1.0f, -1.0f, VTK_FLOAT_MAX, 1.0);
    this->JumpPassProgram->Program->SetUniformi("source", sourceId);

    ostate->vtkglDisable(GL_BLEND);
    ostate->vtkglDisable(GL_DEPTH_TEST);

    this->FrameBufferObject->RenderQuad(0, widthHalfTarget - 1, 0, heightHalfTarget - 1,
      this->JumpPassProgram->Program, this->JumpPassProgram->VAO);

#ifdef VTK_JUMP_FLOOD_ALGORITHM_DEBUG
    {
      std::stringstream debugFileNameSS;
      debugFileNameSS << "Jump_" << i << ".png";
      std::string debugFileName = debugFileNameSS.str();
      DebugWriteImageToFile(target, widthHalfTarget, heightHalfTarget, 4, debugFileName.c_str());
    }
#endif

    source->Deactivate();

    if (source == this->InitPass)
    {
      source = this->JumpFloodPass2;
    }
    vtkTextureObject* tmp = target;
    target = source;
    source = tmp;
  }
  source = target;

  ////////////////////////
  // Outline step
  if (!this->InitializeProgram(renWin, this->OutlineProgram, VERTEX_SHADER, vtkJumpFloodOutlineFS, ""))
  {
    vtkErrorMacro("Couldn't build the shader program. At this point , it can be an error in a "
      "shader or a driver bug.");
    // restore some state.
    this->FrameBufferObject->UnBind();
    this->FrameBufferObject->RestorePreviousBindingsAndBuffers();
    return;
  }

  if (this->OutlinePass == nullptr)
  {
    this->OutlinePass = vtkSmartPointer<vtkTextureObject>::New();
    this->OutlinePass->SetContext(this->FrameBufferObject->GetContext());
  }
  if (this->OutlinePass->GetWidth() != static_cast<unsigned int>(width) ||
    this->OutlinePass->GetHeight() != static_cast<unsigned int>(height))
  {
    this->OutlinePass->Create2D(static_cast<unsigned int>(width),
      static_cast<unsigned int>(height), 4, DATA_TYPE_VTK, false);
  }

  //this->FrameBufferObject->AddColorAttachment(0, this->OutlinePass);
  //this->FrameBufferObject->Start(width, height);

  source->Activate();
  sourceId = source->GetTextureUnit();
  source->SetMinificationFilter(vtkTextureObject::Linear);
  source->SetMagnificationFilter(vtkTextureObject::Linear);
  source->SetWrapS(vtkTextureObject::ClampToEdge);
  source->SetWrapT(vtkTextureObject::ClampToEdge);
  this->OutlineProgram->Program->SetUniformi("source", sourceId);

  // Setup original scene texture.
  this->ScenePass->Activate();
  this->ScenePass->SetMinificationFilter(vtkTextureObject::Linear);
  this->ScenePass->SetMagnificationFilter(vtkTextureObject::Linear);
  this->ScenePass->SetWrapS(vtkTextureObject::ClampToEdge);
  this->ScenePass->SetWrapT(vtkTextureObject::ClampToEdge);
  sourceId = this->ScenePass->GetTextureUnit();
  this->OutlineProgram->Program->SetUniformi("scene", sourceId);

  float windowSize[2] =
  {
    width,
    height,
  };
  this->OutlineProgram->Program->SetUniform2f("windowSize", windowSize);
  this->OutlineProgram->Program->SetUniformi("outlineWidthPx", this->OutlinePx);
  this->FrameBufferObject->RenderQuad(0, width - 1, 0, height - 1,
    this->OutlineProgram->Program, this->OutlineProgram->VAO);

#ifdef VTK_JUMP_FLOOD_ALGORITHM_DEBUG

  DebugWriteImageToFile(this->OutlinePass, width, height, 4, "Outline.png");
#endif

  // If this is a transparent (layered) renderer enable blending
  if (s->GetRenderer()->Transparent())
  {
    ostate->vtkglEnable(GL_BLEND);
    ostate->vtkglBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    ostate->vtkglBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
  }

  this->OutlinePass->CopyToFrameBuffer(0, 0, width - 1, height - 1, 0, 0,
    width - 1, height - 1, width,
    height, // Not used, but only by calling this method
    // directly will the correct texture coordinates be used
    this->OutlineProgram->Program, this->OutlineProgram->VAO);

  source->Deactivate();
  this->ScenePass->Deactivate();

  vtkOpenGLCheckErrorMacro("failed after Render");
}

// ----------------------------------------------------------------------------
bool vtkJumpFloodAlgorithmPass::InitializeProgram(vtkOpenGLRenderWindow* renWin, vtkOpenGLHelper*& program, const char* vsSource, const char* fsSource, const char* gsSource)
{
  if (program)
  {
    renWin->GetShaderCache()->ReadyShaderProgram(program->Program);
  }
  else
  {
    program = new vtkOpenGLHelper;

    // compile and bind it if needed
    vtkShaderProgram* newShader = renWin->GetShaderCache()->ReadyShaderProgram(
      vsSource, fsSource, gsSource);

    // if the shader changed reinitialize the VAO
    if (newShader != program->Program)
    {
      program->Program = newShader;
      program->VAO->ShaderProgramChanged(); // reset the VAO as the shader has changed
    }

    program->ShaderSourceTime.Modified();
  }

  if (!program->Program || !program->Program->GetCompiled())
  {
    return false;
  }
  return true;
}

// ----------------------------------------------------------------------------
// Description:
// Release graphics resources and ask components to release their own
// resources.
// \pre w_exists: w!=0
void vtkJumpFloodAlgorithmPass::ReleaseGraphicsResources(vtkWindow* w)
{
  assert("pre: w_exists" && w != nullptr);

  this->Superclass::ReleaseGraphicsResources(w);

  if (this->InitProgram != nullptr)
  {
    this->InitProgram->ReleaseGraphicsResources(w);
    delete this->InitProgram;
    this->InitProgram = nullptr;
  }
  if (this->JumpPassProgram != nullptr)
  {
    this->JumpPassProgram->ReleaseGraphicsResources(w);
    delete this->JumpPassProgram;
    this->JumpPassProgram = nullptr;
  }
  if (this->OutlineProgram != nullptr)
  {
    this->OutlineProgram->ReleaseGraphicsResources(w);
    delete this->OutlineProgram;
    this->OutlineProgram = nullptr;
  }

  this->FrameBufferObject = nullptr;
  this->ScenePass = nullptr;
  this->InitPass = nullptr;
  this->JumpFloodPass1 = nullptr;
  this->JumpFloodPass2 = nullptr;
  this->OutlinePass = nullptr;
}
