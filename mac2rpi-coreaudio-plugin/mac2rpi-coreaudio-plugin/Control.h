#ifndef Control_h
#define Control_h

#include "AudioObject.h"

class Device;

class VolumeControl : public AudioObject {
public:
  VolumeControl(AudioObjectID objectID, const Device& device);

  Boolean HasProperty(pid_t clientProcessID,
                      const AudioObjectPropertyAddress& address) const override;
  
  Boolean IsPropertySettable(pid_t clientProcessID,
                             const AudioObjectPropertyAddress& address) const override;
  
  UInt32 GetPropertyDataSize(pid_t clientProcessID,
                             const AudioObjectPropertyAddress& address,
                             UInt32 qualifierDataSize,
                             const void* qualifierData) const override;
  
  UInt32 GetPropertyData(pid_t clientProcessID,
                         const AudioObjectPropertyAddress& address,
                         UInt32 qualifierDataSize,
                         const void* qualifierData,
                         UInt32 dataSize,
                         void* data) const override;
  
  std::pair<UInt32, ChangedPropertyList>
  SetPropertyData(pid_t clientProcessID,
                  const AudioObjectPropertyAddress& address,
                  UInt32 qualifierDataSize,
                  const void* qualifierData,
                  UInt32 dataSize,
                  const void* data) override;
  
private:
  const Device& device_;
};

class MuteControl : public AudioObject {
public:
  MuteControl(AudioObjectID objectID, const Device& device);

  Boolean HasProperty(pid_t clientProcessID,
                      const AudioObjectPropertyAddress& address) const override;
  
  Boolean IsPropertySettable(pid_t clientProcessID,
                             const AudioObjectPropertyAddress& address) const override;
  
  UInt32 GetPropertyDataSize(pid_t clientProcessID,
                             const AudioObjectPropertyAddress& address,
                             UInt32 qualifierDataSize,
                             const void* qualifierData) const override;
  
  UInt32 GetPropertyData(pid_t clientProcessID,
                         const AudioObjectPropertyAddress& address,
                         UInt32 qualifierDataSize,
                         const void* qualifierData,
                         UInt32 dataSize,
                         void* data) const override;
  
  std::pair<UInt32, ChangedPropertyList>
  SetPropertyData(pid_t clientProcessID,
                  const AudioObjectPropertyAddress& address,
                  UInt32 qualifierDataSize,
                  const void* qualifierData,
                  UInt32 dataSize,
                  const void* data) override;
  
private:
  const Device& device_;
};

#endif /* Control_h */
