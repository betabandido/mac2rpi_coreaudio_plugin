#ifndef Stream_h
#define Stream_h

#include "AudioObject.h"

class Device;

class Stream : public AudioObject {
public:
  Stream(AudioObjectID objectID, Device& device);
  
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
  Device& device_;
};

#endif /* Stream_h */
