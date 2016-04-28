#include "AudioObject.h"

#include "OSException.h"

AudioObject::AudioObject(AudioObjectID objectID,
                         AudioClassID classID,
                         AudioClassID baseClassID,
                         AudioObjectID ownerID)
  : objectID_(objectID)
  , classID_(classID)
  , baseClassID_(baseClassID)
  , ownerID_(ownerID)
{}

Boolean AudioObject::HasProperty(pid_t clientProcessID,
                                 const AudioObjectPropertyAddress& address) const {
#pragma unused(clientProcessID)
  
  switch (address.mSelector) {
    case kAudioObjectPropertyBaseClass:
    case kAudioObjectPropertyClass:
    case kAudioObjectPropertyOwner:
    case kAudioObjectPropertyOwnedObjects:
      return true;
  }
  
  return false;
}

Boolean AudioObject::IsPropertySettable(pid_t clientProcessID,
                                        const AudioObjectPropertyAddress& address) const {
#pragma unused(clientProcessID)

  switch (address.mSelector) {
    case kAudioObjectPropertyBaseClass:
    case kAudioObjectPropertyClass:
    case kAudioObjectPropertyOwner:
    case kAudioObjectPropertyOwnedObjects:
      return false;
  };
  
  throw OSException("unknown property", kAudioHardwareUnknownPropertyError);
}

UInt32 AudioObject::GetPropertyDataSize(pid_t clientProcessID,
                                        const AudioObjectPropertyAddress& address,
                                        UInt32 qualifierDataSize,
                                        const void* qualifierData) const {
  
#pragma unused(clientProcessID, qualifierDataSize, qualifierData)

  switch (address.mSelector) {
    case kAudioObjectPropertyBaseClass:
    case kAudioObjectPropertyClass:
      return sizeof(AudioClassID);
      
    case kAudioObjectPropertyOwner:
      return sizeof(AudioObjectID);
      
    case kAudioObjectPropertyOwnedObjects:
      return 0;
  };
  
  throw OSException("unknown property", kAudioHardwareUnknownPropertyError);
}

UInt32 AudioObject::GetPropertyData(pid_t clientProcessID,
                                    const AudioObjectPropertyAddress& address,
                                    UInt32 qualifierDataSize,
                                    const void* qualifierData,
                                    UInt32 dataSize,
                                    void* data) const {

#pragma unused(clientProcessID, qualifierDataSize, qualifierData)

  switch (address.mSelector) {
    case kAudioObjectPropertyBaseClass:
      return GetPropertyDataImpl<AudioClassID>
          (dataSize, baseClassID_, data);
      
    case kAudioObjectPropertyClass:
      return GetPropertyDataImpl<AudioClassID>
          (dataSize, classID_, data);
      
    case kAudioObjectPropertyOwner:
      return GetPropertyDataImpl<AudioObjectID>
          (dataSize, ownerID_, data);
      
    case kAudioObjectPropertyOwnedObjects:
      return 0;
  };
  
  throw OSException("unknown property", kAudioHardwareUnknownPropertyError);
}

std::pair<UInt32, AudioObject::ChangedPropertyList>
AudioObject::SetPropertyData(pid_t clientProcessID,
                             const AudioObjectPropertyAddress& address,
                             UInt32 qualifierDataSize,
                             const void* qualifierData,
                             UInt32 dataSize,
                             const void* data) {
  
#pragma unused( \
    clientProcessID, address, qualifierDataSize, qualifierData, dataSize, data)
  
  throw OSException("unknown property", kAudioHardwareUnknownPropertyError);
}

void CheckOutDataSize(UInt32 provided, UInt32 required) {
  if (provided < required)
    throw OSException("not enough space for return value",
                      kAudioHardwareBadPropertySizeError);
}

void CheckInDataSize(UInt32 provided, UInt32 required) {
  if (provided != required)
    throw OSException("wrong size for property data",
                      kAudioHardwareBadPropertySizeError);
}

std::map<AudioObjectID, AudioObjectMap::AudioObjectPtr>
    AudioObjectMap::audioObjects_;

void AudioObjectMap::AddObject(AudioObjectID objectID, AudioObjectPtr object) {
  auto result = audioObjects_.emplace(objectID, object);
  if (!result.second)
    throw OSException("object already exists");
}

AudioObject& AudioObjectMap::FindObject(AudioObjectID objectID) {
  auto it = audioObjects_.find(objectID);
  if (it == end(audioObjects_))
    throw OSException("invalid object ID", kAudioHardwareBadObjectError);
  return *it->second;
}
