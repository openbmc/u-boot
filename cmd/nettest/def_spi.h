/*
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef DEF_SPI_H
#define DEF_SPI_H

#include "typedef.h"
#include "swfunc.h"

typedef struct _DEVICE_PCI_INFO
{
    uint16_t    usVendorID;
    uint16_t    usDeviceID;
    uint32_t ulPCIConfigurationBaseAddress;
    uint32_t ulPhysicalBaseAddress;
    uint32_t ulMMIOBaseAddress;
    uint16_t    usRelocateIO;
} DEVICE_PCI_INFO;

//VIDEO Engine Info
typedef struct _VIDEO_ENGINE_INFO {
    uint16_t             iEngVersion;
    DEVICE_PCI_INFO    VGAPCIInfo;
} VIDEO_ENGINE_INFO;

BOOLEAN  GetDevicePCIInfo (VIDEO_ENGINE_INFO *VideoEngineInfo);

#endif // DEF_SPI_H