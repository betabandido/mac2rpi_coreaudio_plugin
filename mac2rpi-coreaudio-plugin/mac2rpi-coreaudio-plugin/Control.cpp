#include "Control.h"

#include <cmath>

#include "Device.h"
#include "log.h"
#include "OSException.h"

#pragma mark VolumeControl

VolumeControl::VolumeControl(AudioObjectID objectID, Device& device)
  : AudioObject(objectID,
                kAudioVolumeControlClassID,
                kAudioLevelControlClassID,
                device.ObjectID())
  , device_(device)
{}

Boolean VolumeControl::HasProperty(pid_t clientProcessID,
                                   const AudioObjectPropertyAddress& address) const {

  LOG(boost::format("VolumeControlHasProperty: selector=%1%")
      % ControlPropertyToString(address.mSelector));

  switch (address.mSelector) {
    case kAudioControlPropertyScope:
    case kAudioControlPropertyElement:
    case kAudioLevelControlPropertyScalarValue:
    case kAudioLevelControlPropertyDecibelValue:
    case kAudioLevelControlPropertyDecibelRange:
    case kAudioLevelControlPropertyConvertScalarToDecibels:
    case kAudioLevelControlPropertyConvertDecibelsToScalar:
      return true;
  }
  
  return AudioObject::HasProperty(clientProcessID, address);
}

Boolean VolumeControl::IsPropertySettable(pid_t clientProcessID,
                                          const AudioObjectPropertyAddress& address) const {
  switch (address.mSelector) {
    case kAudioControlPropertyScope:
    case kAudioControlPropertyElement:
    case kAudioLevelControlPropertyDecibelRange:
    case kAudioLevelControlPropertyConvertScalarToDecibels:
    case kAudioLevelControlPropertyConvertDecibelsToScalar:
      return false;
      
    case kAudioLevelControlPropertyScalarValue:
    case kAudioLevelControlPropertyDecibelValue:
      return true;
  };
  
  return AudioObject::IsPropertySettable(clientProcessID, address);
}

UInt32 VolumeControl::GetPropertyDataSize(pid_t clientProcessID,
                                          const AudioObjectPropertyAddress& address,
                                          UInt32 qualifierDataSize,
                                          const void* qualifierData) const {
  switch (address.mSelector) {
    case kAudioControlPropertyScope:
      return sizeof(AudioObjectPropertyScope);
      
    case kAudioControlPropertyElement:
      return sizeof(AudioObjectPropertyElement);

    case kAudioLevelControlPropertyScalarValue:
    case kAudioLevelControlPropertyDecibelValue:
      return sizeof(Float32);
      
    case kAudioLevelControlPropertyDecibelRange:
      return sizeof(AudioValueRange);
      
    case kAudioLevelControlPropertyConvertScalarToDecibels:
    case kAudioLevelControlPropertyConvertDecibelsToScalar:
      return sizeof(Float32);
  };
  
  return AudioObject::GetPropertyDataSize(clientProcessID,
                                          address,
                                          qualifierDataSize,
                                          qualifierData);
}

UInt32 VolumeControl::GetPropertyData(pid_t clientProcessID,
                                      const AudioObjectPropertyAddress& address,
                                      UInt32 qualifierDataSize,
                                      const void* qualifierData,
                                      UInt32 dataSize,
                                      void* data) const {
  
  LOG(boost::format("VolumeControlGetPropertyData: selector=%1%")
      % ControlPropertyToString(address.mSelector));

  switch (address.mSelector) {
    case kAudioControlPropertyScope:
      return GetPropertyDataImpl<UInt32>
          (dataSize, kAudioObjectPropertyScopeOutput, data);
      
    case kAudioControlPropertyElement:
      return GetPropertyDataImpl<UInt32>
          (dataSize, kAudioObjectPropertyElementMaster, data);

    case kAudioLevelControlPropertyScalarValue:
      return GetPropertyDataImpl<Float32>
          (dataSize, device_.OutputVolume(), data);
      
    case kAudioLevelControlPropertyDecibelValue:
    {
      GetPropertyDataImpl<Float32>
          (dataSize, device_.OutputVolume(), data);
      auto& volume = *(static_cast<Float32*>(data));
      volume = volume * volume;
      volume = Device::volumeMinDB
          + (volume * (Device::volumeMaxDB - Device::volumeMinDB));
      return sizeof(Float32);
    }

    case kAudioLevelControlPropertyDecibelRange:
    {
      CheckOutDataSize(dataSize, sizeof(AudioValueRange));
      auto& range = *(static_cast<AudioValueRange*>(data));
      range.mMinimum = Device::volumeMinDB;
      range.mMaximum = Device::volumeMaxDB;
      return sizeof(AudioValueRange);
    }

    case kAudioLevelControlPropertyConvertScalarToDecibels:
    {
      CheckOutDataSize(dataSize, sizeof(Float32));
      auto& volume = *(static_cast<Float32*>(data));
      if (volume < 0.0) volume = 0;
      if (volume > 1.0) volume = 1;
      volume = volume * volume;
      volume = Device::volumeMinDB
          + (volume * (Device::volumeMaxDB - Device::volumeMinDB));
      return sizeof(Float32);
    }

    case kAudioLevelControlPropertyConvertDecibelsToScalar:
      CheckOutDataSize(dataSize, sizeof(Float32));
      auto& volume = *(static_cast<Float32*>(data));
      if (volume < Device::volumeMinDB) volume = Device::volumeMinDB;
      if (volume > Device::volumeMaxDB) volume = Device::volumeMaxDB;
      volume = volume - Device::volumeMinDB;
      volume = volume / (Device::volumeMaxDB - Device::volumeMinDB);
      volume = std::sqrt(volume);
      return sizeof(Float32);
  };
  
  return AudioObject::GetPropertyData(clientProcessID,
                                      address,
                                      qualifierDataSize,
                                      qualifierData,
                                      dataSize,
                                      data);
}

std::pair<UInt32, VolumeControl::ChangedPropertyList>
VolumeControl::SetPropertyData(pid_t clientProcessID,
                               const AudioObjectPropertyAddress& address,
                               UInt32 qualifierDataSize,
                               const void* qualifierData,
                               UInt32 dataSize,
                               const void* data) {
  switch (address.mSelector) {
    case kAudioLevelControlPropertyScalarValue:
    {
      CheckInDataSize(dataSize, sizeof(Float32));
      auto volume = *(static_cast<const Float32*>(data));
      volume = std::max<Float32>(volume, 0);
      volume = std::min<Float32>(volume, 1);
      LOG(boost::format("###### Volume set scalar value (%1%) !!!") % volume);
      if (volume != device_.OutputVolume()) {
        device_.SetOutputVolume(volume);
        ChangedPropertyList changedProperties;
        changedProperties[0].mSelector = kAudioLevelControlPropertyScalarValue;
        changedProperties[0].mScope = kAudioObjectPropertyScopeGlobal;
        changedProperties[0].mElement = kAudioObjectPropertyElementMaster;
        changedProperties[1].mSelector = kAudioLevelControlPropertyDecibelValue;
        changedProperties[1].mScope = kAudioObjectPropertyScopeGlobal;
        changedProperties[1].mElement = kAudioObjectPropertyElementMaster;
        return std::make_pair(2, changedProperties);
      }
      
      return std::make_pair(0, ChangedPropertyList{});
    }
      
    case kAudioLevelControlPropertyDecibelValue:
    {
      CheckInDataSize(dataSize, sizeof(Float32));
      auto volume = *(static_cast<const Float32*>(data));
      volume = std::max<Float32>(volume, Device::volumeMinDB);
      volume = std::min<Float32>(volume, Device::volumeMaxDB);
      LOG(boost::format("###### Volume set decibel value (%1%) !!!") % volume);
      
      volume = volume - Device::volumeMinDB;
      volume = volume / (Device::volumeMaxDB - Device::volumeMinDB);
      volume = std::sqrt(volume);
      
      if (volume != device_.OutputVolume()) {
        device_.SetOutputVolume(volume);
        ChangedPropertyList changedProperties;
        changedProperties[0].mSelector = kAudioLevelControlPropertyScalarValue;
        changedProperties[0].mScope = kAudioObjectPropertyScopeGlobal;
        changedProperties[0].mElement = kAudioObjectPropertyElementMaster;
        changedProperties[1].mSelector = kAudioLevelControlPropertyDecibelValue;
        changedProperties[1].mScope = kAudioObjectPropertyScopeGlobal;
        changedProperties[1].mElement = kAudioObjectPropertyElementMaster;
        return std::make_pair(2, changedProperties);
      }
      
      return std::make_pair(0, ChangedPropertyList{});
    }
  };
  
  return AudioObject::SetPropertyData(clientProcessID,
                                      address,
                                      qualifierDataSize,
                                      qualifierData,
                                      dataSize,
                                      data);
}

/////////////////
// MuteControl //
/////////////////

#pragma mark MuteControl

MuteControl::MuteControl(AudioObjectID objectID, Device& device)
  : AudioObject(objectID,
                kAudioMuteControlClassID,
                kAudioBooleanControlClassID,
                device.ObjectID())
  , device_(device)
{}

Boolean MuteControl::HasProperty(pid_t clientProcessID,
                                 const AudioObjectPropertyAddress& address) const {
  
  LOG(boost::format("MuteControlHasProperty: selector=%1%")
      % ControlPropertyToString(address.mSelector));

  switch (address.mSelector) {
    case kAudioControlPropertyScope:
    case kAudioControlPropertyElement:
    case kAudioBooleanControlPropertyValue:
      return true;
  }
  
  return AudioObject::HasProperty(clientProcessID, address);
}

Boolean MuteControl::IsPropertySettable(pid_t clientProcessID,
                                        const AudioObjectPropertyAddress& address) const {
  switch (address.mSelector) {
    case kAudioControlPropertyScope:
    case kAudioControlPropertyElement:
      return false;
      
    case kAudioBooleanControlPropertyValue:
      return true;
  };
  
  return AudioObject::IsPropertySettable(clientProcessID, address);
}

UInt32 MuteControl::GetPropertyDataSize(pid_t clientProcessID,
                                        const AudioObjectPropertyAddress& address,
                                        UInt32 qualifierDataSize,
                                        const void* qualifierData) const {
  switch (address.mSelector) {
    case kAudioControlPropertyScope:
      return sizeof(AudioObjectPropertyScope);
      
    case kAudioControlPropertyElement:
      return sizeof(AudioObjectPropertyElement);
      
    case kAudioBooleanControlPropertyValue:
      return sizeof(UInt32);
  };
  
  return AudioObject::GetPropertyDataSize(clientProcessID,
                                          address,
                                          qualifierDataSize,
                                          qualifierData);
}

UInt32 MuteControl::GetPropertyData(pid_t clientProcessID,
                                    const AudioObjectPropertyAddress& address,
                                    UInt32 qualifierDataSize,
                                    const void* qualifierData,
                                    UInt32 dataSize,
                                    void* data) const {
  
  LOG(boost::format("MuteControlGetPropertyData: selector=%1%")
      % ControlPropertyToString(address.mSelector));

  switch (address.mSelector) {
    case kAudioControlPropertyScope:
      return GetPropertyDataImpl<UInt32>
      (dataSize, kAudioObjectPropertyScopeOutput, data);
      
    case kAudioControlPropertyElement:
      return GetPropertyDataImpl<UInt32>
      (dataSize, kAudioObjectPropertyElementMaster, data);
      
    case kAudioBooleanControlPropertyValue:
      return GetPropertyDataImpl<UInt32>
          (dataSize, device_.OutputMute(), data);
  };
  
  return AudioObject::GetPropertyData(clientProcessID,
                                      address,
                                      qualifierDataSize,
                                      qualifierData,
                                      dataSize,
                                      data);
}

std::pair<UInt32, VolumeControl::ChangedPropertyList>
MuteControl::SetPropertyData(pid_t clientProcessID,
                             const AudioObjectPropertyAddress& address,
                             UInt32 qualifierDataSize,
                             const void* qualifierData,
                             UInt32 dataSize,
                             const void* data) {
  switch (address.mSelector) {
    case kAudioBooleanControlPropertyValue:
    {
      CheckInDataSize(dataSize, sizeof(UInt32));
      bool mute = *(static_cast<const UInt32*>(data)) != 0;
      LOG(boost::format("###### Mute set value (%1%) !!!") % mute);
      
      if (mute != device_.OutputMute()) {
        device_.SetOutputMute(mute);
        ChangedPropertyList changedProperties;
        changedProperties[0].mSelector = kAudioBooleanControlPropertyValue;
        changedProperties[0].mScope = kAudioObjectPropertyScopeGlobal;
        changedProperties[0].mElement = kAudioObjectPropertyElementMaster;
        return std::make_pair(1, changedProperties);
      }
      
      return std::make_pair(0, ChangedPropertyList{});
    }
  };
  
  return AudioObject::SetPropertyData(clientProcessID,
                                      address,
                                      qualifierDataSize,
                                      qualifierData,
                                      dataSize,
                                      data);
}
