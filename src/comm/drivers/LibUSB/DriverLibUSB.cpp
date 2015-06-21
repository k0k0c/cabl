/*----------------------------------------------------------------------------------------------------------------------   

                 %%%%%%%%%%%%%%%%%                
                 %%%%%%%%%%%%%%%%%
                 %%%           %%%
                 %%%           %%%
                 %%%           %%%
%%%%%%%%%%%%%%%%%%%%           %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%           %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% www.shaduzlabs.com %%%%

------------------------------------------------------------------------------------------------------------------------

  Copyright (C) 2014 Vincenzo Pacella

  This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
  License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
  version.

  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along with this program.  
  If not, see <http://www.gnu.org/licenses/>.

----------------------------------------------------------------------------------------------------------------------*/

#include "DriverLibUSB.h"
#include "DeviceHandleLibUSB.h"

namespace sl
{
namespace kio
{
  
//----------------------------------------------------------------------------------------------------------------------

DriverLibUSB::DriverLibUSB()
{
  libusb_init(&m_pContext);

#ifdef DEBUG
  libusb_set_debug( m_pContext, 3);
#endif
}
  
//----------------------------------------------------------------------------------------------------------------------
  
DriverLibUSB::~DriverLibUSB()
{
  libusb_exit( m_pContext );
}
  
//----------------------------------------------------------------------------------------------------------------------

Driver::tCollDeviceDescriptor DriverLibUSB::enumerate()
{
  Driver::tCollDeviceDescriptor collDeviceDescriptor;
  
  libusb_device **devices;
  ssize_t nDevices = libusb_get_device_list(m_pContext, &devices);
  
  tDeviceHandle* pHandle = nullptr;
  for( int i=0; i<nDevices; ++i )
  {
    libusb_device *device = devices[i];
    if(libusb_open(device, &pHandle) < 0)
    {
      continue;
    }
    struct libusb_device_descriptor descriptor;
    libusb_get_device_descriptor(device, &descriptor);
    std::string strSerialNumber;
    if(descriptor.iSerialNumber != 0)
    {
      char sNum[256];
      
      if(libusb_get_string_descriptor_ascii(pHandle, descriptor.iSerialNumber,(unsigned char*)sNum, sizeof(sNum)) > 0)
      {
        strSerialNumber = sNum;
      }
    }
    libusb_close(pHandle);
    DeviceDescriptor deviceDescriptor(
      descriptor.idVendor,
      descriptor.idProduct,
      strSerialNumber,
      false
    );

    collDeviceDescriptor.push_back(deviceDescriptor);
  }
  
  libusb_free_device_list(devices, 1);

  return collDeviceDescriptor;
}

//----------------------------------------------------------------------------------------------------------------------
  
tPtr<DeviceHandleImpl> DriverLibUSB::connect( const DeviceDescriptor& device_ )
{
  
  bool bConnected = false;
  libusb_device **devices;
  ssize_t nDevices = libusb_get_device_list(m_pContext, &devices);
  
  tDeviceHandle* pCurrentDevice = nullptr;
  for( int i=0; i<nDevices; ++i )
  {
    libusb_device *device = devices[i];
    struct libusb_device_descriptor descriptor;
    libusb_get_device_descriptor(device, &descriptor);
    if( (descriptor.idVendor != device_.getVendorId()) ||
        (descriptor.idProduct != device_.getProductId())
    )
    {
      continue;  // ADD SERIAL NUMBER!
    }
    
    int e = libusb_open(device, &pCurrentDevice);
    if(e == 0)
    {
      bConnected = true;
      break;
    }
  }
  
  libusb_free_device_list(devices, 1);
  
  libusb_set_configuration(pCurrentDevice, 1 );
  libusb_claim_interface(pCurrentDevice, 0 );

  libusb_set_interface_alt_setting  (pCurrentDevice, 0, 1 );
  
  if(pCurrentDevice == nullptr || !bConnected)
    return nullptr;

  return tPtr<DeviceHandleImpl>(new DeviceHandleLibUSB(pCurrentDevice));
}
  
//----------------------------------------------------------------------------------------------------------------------

} // kio
} // sl