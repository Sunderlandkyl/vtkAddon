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

// Fragment shader used to generate the initial pass for the jump flood algorithm.

in vec2 tcoordVC;
uniform sampler2D source;

//VTK::Output::Dec

void main(void)
{
  vec4 color = texture2D(source, tcoordVC);
  if (color.a > 0)
  {
    color.xy = tcoordVC;
    color.z = 0.0;
  }
  else
  {
    color.xy = vec2(-1.0);
    color.z = 1.0/0.0;
  }
  color.a = 1.0;

  gl_FragData[0] = color;
}
