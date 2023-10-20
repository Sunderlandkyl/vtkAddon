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
/**
 * @class   vtkJumpFloodAlgorithmPass
 * @brief   Renders a glowing outline using a image processing pass
 *
 * /// TODO
 *
 * Create a glowing outline of the image rendered by the delegate.
 *
 * This render pass was designed to highlight parts of a scene by applying the render pass to a
 * layered renderer on top of the main scene. For optimal results, actors that form the outline
 * should be brightly colored with lighting disabled. The outline will have the color of the actors.
 * There is only one outline around all objects rendered by the delegate.
 *
 * This pass expects an initialized depth buffer and color buffer.
 * Initialized buffers means they have been cleared with farthest z-value and
 * background color/gradient/transparent color.
 * An opaque pass may have been performed right after the initialization.
 *
 * The delegate is used once.
 *
 * Its delegate is usually set to a vtkCameraPass or to a post-processing pass.
 *
 * This pass requires a OpenGL context that supports texture objects (TO),
 * framebuffer objects (FBO) and GLSL. If not, it will emit an error message
 * and will render its delegate and return.
 *
 * @par Implementation:
 * The image is first rendered to a full size offscreen render target, then blurred twice on a half
 * sized render target using Gaussian blur with an offset. The offset and the smaller render target
 * increase the size of the outline without incurring the cost of a big Gaussian blur kernel. The
 * implementation of the gaussian blur is similar to vtkGaussianBlurPass with the alterations
 * described above.
 *
 * @sa
 * vtkRenderPass vtkGaussianBlurPass
 */

#ifndef vtkJumpFloodAlgorithmPass_h
#define vtkJumpFloodAlgorithmPass_h

// export
#include "vtkAddonExport.h"

#include "vtkImageProcessingPass.h"

#include <vtkSmartPointer.h>
#include <vtkTextureObject.h>
#include <vtkOpenGLHelper.h>
#include <vtkOpenGLFramebufferObject.h>

class VTK_ADDON_EXPORT vtkJumpFloodAlgorithmPass : public vtkImageProcessingPass
{
public:
  static vtkJumpFloodAlgorithmPass* New();
  vtkTypeMacro(vtkJumpFloodAlgorithmPass, vtkImageProcessingPass);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /**
   * Perform rendering according to a render state \p s.
   * \pre s_exists: s!=0
   */
  void Render(const vtkRenderState* s) override;

  /**
   * Release graphics resources and ask components to release their own
   * resources.
   * \pre w_exists: w!=0
   */
  void ReleaseGraphicsResources(vtkWindow* w) override;

  /**
   * Get/Set the size of the outline.
   * TODO
   */
  vtkGetMacro(OutlinePx, int);
  vtkSetMacro(OutlinePx, int);

protected:

  bool InitializeProgram(vtkOpenGLRenderWindow* renWin, vtkOpenGLHelper*& program, const char* vertSource, const char* fragSource, const char* geometrySource);

  /**
   * Graphics resources.
   */
  vtkSmartPointer<vtkOpenGLFramebufferObject> FrameBufferObject;
  vtkSmartPointer<vtkTextureObject> ScenePass;      // render target for the original scene

  vtkSmartPointer<vtkTextureObject> InitPass;       // render target for init pass

  vtkSmartPointer<vtkTextureObject> JumpFloodPass1; // Render targets for iterating jump fill
  vtkSmartPointer<vtkTextureObject> JumpFloodPass2; // passes back and forth.

  vtkSmartPointer<vtkTextureObject> OutlinePass; // passes back and forth.

  // Shader programs
  vtkOpenGLHelper* InitProgram;
  vtkOpenGLHelper* JumpPassProgram;
  vtkOpenGLHelper* OutlineProgram;

  // Outline Px
  int OutlinePx{ 100 };

protected:
  vtkJumpFloodAlgorithmPass();
  ~vtkJumpFloodAlgorithmPass() override;

private:
  vtkJumpFloodAlgorithmPass(const vtkJumpFloodAlgorithmPass&) = delete;
  void operator=(const vtkJumpFloodAlgorithmPass&) = delete;
};

#endif /* vtkJumpFloodAlgorithmPass_h */
