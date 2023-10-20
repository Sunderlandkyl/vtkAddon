//VTK::System::Dec

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

// Fragment shader used to generate the outline for the jump flood algorithm.

in vec2 tcoordVC;
uniform sampler2D scene;  // Image containing the original image.
uniform sampler2D source;  // Image containing closest pixel positions in the object to be outlined.
uniform vec2 windowSize;
int outlineWidthPx;

//VTK::Output::Dec

void main(void)
{
  vec4 sceneColor = texture2D(scene, tcoordVC);
  vec4 closestPosUV = texture2D(source, tcoordVC);

  vec2 closestPosPx = vec2(closestPosUV.x * windowSize.x, closestPosUV.y * windowSize.y);
  vec2 currentPosPx = vec2(tcoordVC.x * windowSize.x, tcoordVC.y * windowSize.y);

  vec2 difference = closestPosPx - currentPosPx;
  float distance2Px = dot(difference, difference);
  float outlineWidth2Px = outlineWidthPx * outlineWidthPx;

  vec4 outlineColor = vec4(1.0);
  float min = outlineWidth2Px;
  float max = min + 25.0; // Fade distance
  if (distance2Px < min)
  {
    outlineColor.a = 1.0;
  }
  else if (distance2Px > min && distance2Px < max)
  {
    float alpha = (max - distance2Px)/(max - min);
    //outlineColor.a = alpha;
    outlineColor = vec4((max - distance2Px)/(max - min));
  }
  else
  {
    outlineColor = vec4(0.0);
  }
  gl_FragData[0] = mix(outlineColor, sceneColor, sceneColor.a);
}
