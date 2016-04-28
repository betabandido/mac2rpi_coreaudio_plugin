#include "PlugIn.h"

#include "Device.h"
#include "log.h"
#include "OSException.h"
#include "types.h"

std::shared_ptr<PlugIn> PlugIn::instance_;

PlugIn& PlugIn::GetInstance() {
  static std::once_flag initOnce;
  std::call_once(initOnce, PlugIn::CreateInstance);
  return *instance_;
}

void PlugIn::CreateInstance() {
  instance_ = std::make_shared<PlugIn>(PreventDirectConstruction{0});
  AudioObjectMap::AddObject(kObjectID_PlugIn, instance_);
    // TODO activate ???
}

PlugIn::PlugIn(const PreventDirectConstruction&)
  : AudioObject(kObjectID_PlugIn,
                kAudioPlugInClassID,
                kAudioObjectClassID,
                0)
  , device_(std::make_shared<Device>())
{
  AudioObjectMap::AddObject(kObjectID_Device, device_);
}

Boolean PlugIn::HasProperty(pid_t clientProcessID,
                            const AudioObjectPropertyAddress& address) const {
  
  LOG(boost::format("PluginHasProperty: selector=%1%")
      % PluginPropertyToString(address.mSelector));
  
  switch (address.mSelector) {
    case kAudioObjectPropertyManufacturer:
    case kAudioPlugInPropertyDeviceList:
    case kAudioPlugInPropertyTranslateUIDToDevice:
    case kAudioPlugInPropertyResourceBundle:
      return true;
  }
  
  return AudioObject::HasProperty(clientProcessID, address);
}

Boolean PlugIn::IsPropertySettable(pid_t clientProcessID,
                                   const AudioObjectPropertyAddress& address) const {
  switch (address.mSelector) {
    case kAudioObjectPropertyManufacturer:
    case kAudioPlugInPropertyDeviceList:
    case kAudioPlugInPropertyTranslateUIDToDevice:
    case kAudioPlugInPropertyResourceBundle:
      return false;
  };
  
  return AudioObject::IsPropertySettable(clientProcessID, address);
}

UInt32 PlugIn::GetPropertyDataSize(pid_t clientProcessID,
                                   const AudioObjectPropertyAddress& address,
                                   UInt32 qualifierDataSize,
                                   const void* qualifierData) const {
  switch (address.mSelector) {
    case kAudioObjectPropertyManufacturer:
      return sizeof(CFStringRef);
      
    case kAudioPlugInPropertyDeviceList:
      return sizeof(AudioObjectID);
      
    case kAudioPlugInPropertyTranslateUIDToDevice:
      return sizeof(AudioObjectID);
      
    case kAudioPlugInPropertyResourceBundle:
      return sizeof(CFStringRef);
  };
  
  return AudioObject::GetPropertyDataSize(clientProcessID,
                                          address,
                                          qualifierDataSize,
                                          qualifierData);
}

UInt32 PlugIn::GetPropertyData(pid_t clientProcessID,
                               const AudioObjectPropertyAddress& address,
                               UInt32 qualifierDataSize,
                               const void* qualifierData,
                               UInt32 dataSize,
                               void* data) const {
  
  LOG(boost::format("PluginGetPropertyData: selector=%1%")
      % PluginPropertyToString(address.mSelector));

  switch (address.mSelector) {
    case kAudioObjectPropertyManufacturer:
      return GetPropertyDataImpl<CFStringRef>
          (dataSize, CFSTR("mac2rpi"), data);
      
    case kAudioPlugInPropertyDeviceList:
      return GetPropertyDataImpl<AudioObjectID>
          (dataSize, device_->ObjectID(), data);
      
    case kAudioPlugInPropertyTranslateUIDToDevice:
      CheckOutDataSize(dataSize, sizeof(AudioObjectID));
      CheckInDataSize(qualifierDataSize, sizeof(CFStringRef));
      if (qualifierData == nullptr)
        throw OSException("no qualifier data",
                          kAudioHardwareBadPropertySizeError);
      *(static_cast<AudioObjectID*>(data)) =
          CFStringCompare(*(static_cast<const CFStringRef*>(qualifierData)),
                          CFSTR("mac2rpi-device"),
                          0)
              ? kObjectID_Device
              : kAudioObjectUnknown;
      return sizeof(AudioObjectID);
      
    case kAudioPlugInPropertyResourceBundle:
      return GetPropertyDataImpl<CFStringRef>
          (dataSize, CFSTR(""), data);
  };
  
  return AudioObject::GetPropertyData(clientProcessID,
                                      address,
                                      qualifierDataSize,
                                      qualifierData,
                                      dataSize,
                                      data);
}

std::pair<UInt32, PlugIn::ChangedPropertyList>
PlugIn::SetPropertyData(pid_t clientProcessID,
                             const AudioObjectPropertyAddress& address,
                             UInt32 qualifierDataSize,
                             const void* qualifierData,
                             UInt32 dataSize,
                             const void* data) {
  
  return AudioObject::SetPropertyData(clientProcessID,
                                      address,
                                      qualifierDataSize,
                                      qualifierData,
                                      dataSize,
                                      data);
}

