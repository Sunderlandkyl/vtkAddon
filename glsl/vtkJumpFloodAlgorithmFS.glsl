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

// Fragment shader used as the iterative step in the jump flood algorithm.

in vec2 tcoordVC;
uniform sampler2D source;
uniform vec2 stepSize;

//VTK::Output::Dec

void main(void)
{
  float maxDistance = 1.0/0.0;
  float closestDistance2 = maxDistance;
  vec4 closestPos = vec4(-1.0, -1.0, 1.0/0.0, 1.0);
  for(int i = -1; i <= 1; ++i)
  {
    for(int j = -1; j <= 1; ++j)
    {
      vec2 offset = vec2(i * stepSize.x, j * stepSize.y);
      vec2 texPos = clamp(tcoordVC + offset, 0.0, 1.0);
      vec4 currentPos = texture2D(source, texPos);
      if (currentPos.z == 1.0/0.0)
      {
        continue;
      }

      vec2 difference = currentPos.xy - tcoordVC;
      float currentDistance2 = dot(difference, difference);
      if (currentDistance2 < closestDistance2)
      {
        closestDistance2 = currentDistance2;
        closestPos.xy = currentPos.xy;
        closestPos.z = closestDistance2;
      }
    }
  }

  gl_FragData[0] = closestPos;
}
