#ifndef types_h
#define types_h

#include <CoreAudio/AudioServerPlugIn.h>

enum {
  // Changing this enumeration might affect Device::GetPropertyData().
  // For instance, see how kAudioDevicePropertyStreams or
  // kAudioObjectPropertyControlList are handled in that function.
  
  kObjectID_PlugIn = kAudioObjectPlugInObject,
  kObjectID_Device,
  kObjectID_Stream_Output,
  kObjectID_Volume_Output_Master,
  kObjectID_Mute_Output_Master,
};

#endif /* types_h */
